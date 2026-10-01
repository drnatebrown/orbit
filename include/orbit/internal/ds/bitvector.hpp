#ifndef _BITVECTOR_HPP
#define _BITVECTOR_HPP

#include "orbit/common.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

namespace orbit {

// Rank is intentionally not supported. Elias-Fano get_idx needs select1:
// the position of the k-th 1 (0-based).
class bit_vector {
public:
    bit_vector() = default;

    explicit bit_vector(size_t bit_len) : size_(bit_len) {
        if (bit_len > 0) {
            words_.assign((bit_len + 63) / 64, 0);
        }
    }

    size_t size() const { return size_; }
    size_t num_ones() const { return num_ones_; }
    bool select_ready() const { return select_ready_; }

    void set(size_t pos) {
        assert(pos < size_);
        const size_t word = pos / 64;
        const uint64_t mask = uint64_t{1} << (pos % 64);
        assert((words_[word] & mask) == 0);
        words_[word] |= mask;
        ++num_ones_;
        select_ready_ = false;
    }

    bool get(size_t pos) const {
        assert(pos < size_);
        return (words_[pos / 64] >> (pos % 64)) & 1ULL;
    }

    // Simple-select inventory: absolute samples every 256 ones, uint16
    // sub-samples every 8 ones when the block spans at most 16 bits,
    // otherwise an explicit spill of positions (long block).
    void build_select() {
        inv_pos_.clear();
        inv_long_.clear();
        sub_off_.clear();
        sub_u16_.clear();
        spill_off_.clear();
        spill_.clear();

        if (num_ones_ == 0) {
            select_ready_ = true;
            return;
        }

        std::vector<size_t> ones;
        ones.reserve(num_ones_);
        for (size_t w = 0; w < words_.size(); ++w) {
            uint64_t word = words_[w];
            while (word) {
                const unsigned bit = static_cast<unsigned>(__builtin_ctzll(word));
                const size_t pos = w * 64 + bit;
                if (pos >= size_) break;
                ones.push_back(pos);
                word &= word - 1;
            }
        }
        assert(ones.size() == num_ones_);

        constexpr size_t kOnesPerInventory = 256;
        constexpr size_t kOnesPerSample = 8;
        const size_t ninv = (num_ones_ + kOnesPerInventory - 1) / kOnesPerInventory;
        inv_pos_.resize(ninv);
        inv_long_.resize(ninv);
        sub_off_.resize(ninv);
        spill_off_.resize(ninv);

        for (size_t inv = 0; inv < ninv; ++inv) {
            const size_t begin = inv * kOnesPerInventory;
            const size_t end = std::min(num_ones_, begin + kOnesPerInventory);
            inv_pos_[inv] = ones[begin];
            const bool is_long = (ones[end - 1] - ones[begin]) > 65535;
            inv_long_[inv] = static_cast<uint8_t>(is_long);
            sub_off_[inv] = sub_u16_.size();
            spill_off_[inv] = spill_.size();
            if (is_long) {
                for (size_t j = begin + 1; j < end; ++j) {
                    spill_.push_back(ones[j]);
                }
            } else {
                for (size_t j = begin + kOnesPerSample; j < end; j += kOnesPerSample) {
                    sub_u16_.push_back(static_cast<uint16_t>(ones[j] - ones[begin]));
                }
            }
        }
        select_ready_ = true;
    }

    // Position of the k-th 1. k is 0-based.
    size_t select1(size_t k) const {
        assert(select_ready_);
        assert(k < num_ones_);
        constexpr size_t kOnesPerInventory = 256;
        constexpr size_t kOnesPerSample = 8;
        const size_t inv = k >> 8; // / 256
        const size_t rem = k & (kOnesPerInventory - 1);
        size_t bit = inv_pos_[inv];
        if (rem == 0) return bit;

        if (inv_long_[inv]) {
            return spill_[spill_off_[inv] + rem - 1];
        }

        const size_t sample = rem / kOnesPerSample;
        const size_t within = rem % kOnesPerSample;
        if (sample > 0) {
            bit += sub_u16_[sub_off_[inv] + sample - 1];
        }
        if (within == 0) return bit;
        return select_ones_after(bit, within);
    }

    size_t serialize(std::ostream& out) const {
        size_t written = 0;
        const size_t size = size_;
        const size_t ones = num_ones_;
        const size_t nwords = words_.size();
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        out.write(reinterpret_cast<const char*>(&ones), sizeof(ones));
        out.write(reinterpret_cast<const char*>(&nwords), sizeof(nwords));
        written += 3 * sizeof(size_t);
        if (nwords > 0) {
            out.write(reinterpret_cast<const char*>(words_.data()), nwords * sizeof(uint64_t));
            written += nwords * sizeof(uint64_t);
        }
        return written;
    }

    void load(std::istream& in) {
        size_t size = 0;
        size_t ones = 0;
        size_t nwords = 0;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        in.read(reinterpret_cast<char*>(&ones), sizeof(ones));
        in.read(reinterpret_cast<char*>(&nwords), sizeof(nwords));
        size_ = size;
        num_ones_ = ones;
        words_.assign(nwords, 0);
        if (nwords > 0) {
            in.read(reinterpret_cast<char*>(words_.data()), nwords * sizeof(uint64_t));
        }
        build_select();
    }

private:
    static size_t select_in_word(uint64_t word, size_t rank) {
        for (size_t i = 0; i < rank; ++i) {
            word &= word - 1;
        }
        return static_cast<size_t>(__builtin_ctzll(word));
    }

    // Position of the `count`-th 1 strictly after `bit` (`count` >= 1).
    size_t select_ones_after(size_t bit, size_t count) const {
        assert(count > 0);
        size_t word_i = bit / 64;
        const unsigned shift = static_cast<unsigned>(bit % 64);
        uint64_t word = 0;
        if (shift + 1 < 64) {
            word = words_[word_i] & (~uint64_t{0} << (shift + 1));
        }
        while (true) {
            const unsigned pc = static_cast<unsigned>(__builtin_popcountll(word));
            if (count <= pc) {
                return word_i * 64 + select_in_word(word, count - 1);
            }
            count -= pc;
            ++word_i;
            assert(word_i < words_.size());
            word = words_[word_i];
        }
    }

    std::vector<uint64_t> words_;
    size_t size_ = 0;
    size_t num_ones_ = 0;

    bool select_ready_ = false;
    std::vector<size_t> inv_pos_;
    std::vector<uint8_t> inv_long_;
    std::vector<size_t> sub_off_;
    std::vector<uint16_t> sub_u16_;
    std::vector<size_t> spill_off_;
    std::vector<size_t> spill_;
};

// High part of an Elias-Fano sequence of interval starts.
// select1(i) - i is the high bits of start i; lows live in the move table.
struct ef_index {
    bool enabled = false;
    uchar bits = 0;
    ulint sentinel_low = 0;
    bit_vector high;

    ulint start(size_t interval, ulint low) const {
        assert(enabled);
        assert(bits < 64);
        const size_t selected = high.select1(interval);
        assert(selected >= interval);
        const ulint high_bits = static_cast<ulint>(selected - interval);
        return (high_bits << bits) | low;
    }

    size_t serialize(std::ostream& out) const {
        size_t written = 0;
        const bool on = enabled;
        out.write(reinterpret_cast<const char*>(&on), sizeof(on));
        written += sizeof(on);
        if (!on) return written;

        out.write(reinterpret_cast<const char*>(&bits), sizeof(bits));
        written += sizeof(bits);
        out.write(reinterpret_cast<const char*>(&sentinel_low), sizeof(sentinel_low));
        written += sizeof(sentinel_low);
        written += high.serialize(out);
        return written;
    }

    void load(std::istream& in) {
        bool on = false;
        in.read(reinterpret_cast<char*>(&on), sizeof(on));
        enabled = on;
        if (!on) {
            bits = 0;
            sentinel_low = 0;
            high = bit_vector();
            return;
        }
        in.read(reinterpret_cast<char*>(&bits), sizeof(bits));
        in.read(reinterpret_cast<char*>(&sentinel_low), sizeof(sentinel_low));
        high.load(in);
    }
};

} // namespace orbit

#endif /* _BITVECTOR_HPP */
