#ifndef _PHI_HELPERS_HPP
#define _PHI_HELPERS_HPP

#include "orbit/common.hpp"
#include "orbit/internal/ds/packed_vector_aligned.hpp"
#include "orbit/internal/rlbwt/lf_permutation.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>

namespace orbit::rlbwt {

using int_vec = int_vector_aligned;

template<typename lf_t>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_images(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, lf_t& lf, size_t* domain = nullptr, ulint* max_length = nullptr) {
    int_vec phi_lengths(lf.runs(), bit_width(lf.domain() - 1));

    size_t UNUSED_INTERVAL = max_val(bit_width(lf.intervals()));
    size_t UNUSED_SA = max_val(bit_width(lf.domain()));
    int_vec move_run_to_phi(lf.intervals(), bit_width(lf.intervals())); // Map move run to its Phi interval (only set those corresponding to RLBWT runs)
    int_vec run_tail_sa_samples(lf.intervals(), bit_width(lf.domain())); // The SA samples at the tail of each move run (only set those corresponding to RLBWT runs)

    ulint max_length_seen = 0;
    auto pos = lf.first();
    size_t last_sample = lf.domain();
    size_t sa = lf.domain() - 1;
    // Phi intervals correspond to the original (unsplit) permutation runs, not move runs.
    size_t curr_phi_interval = lf.runs() - 1;
    // Step through entire BWT to recover Phi structure and SA samples at tails
    for (size_t i = 0; i < lf.domain(); ++i) {
        size_t interval = pos.interval;
        size_t offset = pos.offset;
        // If at BWT runhead
        if (offset == 0) {
            if (interval == 0 || lf.get_character(interval - 1) != lf.get_character(interval)) {
                phi_lengths[curr_phi_interval] = last_sample - sa;
                max_length_seen = std::max(max_length_seen, static_cast<ulint>(phi_lengths[curr_phi_interval]));
                move_run_to_phi.set(interval, curr_phi_interval);
                last_sample = sa;
                --curr_phi_interval;
            }
            else {
                move_run_to_phi.set(interval, UNUSED_INTERVAL);
            }
        }
        // If at BWT run tail
        if (offset == lf.get_length(interval) - 1) {
            if (interval == lf.intervals() - 1 || lf.get_character(interval + 1) != lf.get_character(interval)) {
                run_tail_sa_samples.set(interval, sa);
            }
            else {
                run_tail_sa_samples.set(interval, UNUSED_SA);
            }
        }
        --sa;
        pos = lf.LF(pos);
    }

    int_vec phi_images(lf.runs(), bit_width(lf.domain() - 1));
    // Step through BWT tail samples to fill in Phi interval permutations
    for (size_t i = 0; i < run_tail_sa_samples.size(); ++i) {
        if (run_tail_sa_samples.get(i) == UNUSED_SA) continue;
        phi_images[move_run_to_phi.get((i == lf.intervals() - 1) ? 0 : i + 1)] = run_tail_sa_samples.get(i);
    }

    if (domain != nullptr) {
        *domain = lf.domain();
    }
    if (max_length != nullptr) {
        *max_length = max_length_seen;
    }

    return {phi_lengths, phi_images};
}

template<typename alphabet_t=nucleotide>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_images(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, size_t* domain = nullptr, ulint* max_length = nullptr) {
    // Need a move structure with LF to find SA samples
    lf_move_impl_default<alphabet_t> move_lf(bwt_heads, bwt_run_lengths);
    return rlbwt_to_phi_images(bwt_heads, bwt_run_lengths, move_lf, domain, max_length);
}   

template<typename lf_t>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_img_rank_inv(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, lf_t& lf, size_t* domain = nullptr, ulint* max_length = nullptr) {
    int_vec phi_lengths(lf.runs(), bit_width(lf.domain() - 1));

    size_t UNUSED_INTERVAL = max_val(bit_width(lf.intervals()));
    size_t UNUSED_VISIT_RANK = max_val(bit_width(lf.runs()));
    int_vec move_run_to_phi(lf.intervals(), bit_width(lf.intervals())); // Map move run to its Phi interval (only set those corresponding to RLBWT runs)
    int_vec run_tail_visit_rank(lf.intervals(), bit_width(lf.runs())); // The rank of the visit to the tail of each move run (only set those corresponding to RLBWT runs)

    ulint max_length_seen = 0;
    auto pos = lf.first();
    size_t last_sample = lf.domain();
    size_t sa = lf.domain() - 1;
    // Phi intervals correspond to the original (unsplit) permutation runs, not move runs.
    size_t curr_phi_interval = lf.runs() - 1;
    size_t curr_visit_rank = lf.runs() - 1;
    // Step through entire BWT to recover Phi structure and SA samples at tails
    for (size_t i = 0; i < lf.domain(); ++i) {
        size_t interval = pos.interval;
        size_t offset = pos.offset;
        // If at BWT runhead
        if (offset == 0) {
            if (interval == 0 || lf.get_character(interval - 1) != lf.get_character(interval)) {
                phi_lengths[curr_phi_interval] = last_sample - sa;
                max_length_seen = std::max(max_length_seen, static_cast<ulint>(phi_lengths[curr_phi_interval]));
                move_run_to_phi.set(interval, curr_phi_interval);
                last_sample = sa;
                --curr_phi_interval;
            }
            else {
                move_run_to_phi.set(interval, UNUSED_INTERVAL);
            }
        }
        // If at BWT run tail
        if (offset == lf.get_length(interval) - 1) {
            if (interval == lf.intervals() - 1 || lf.get_character(interval + 1) != lf.get_character(interval)) {
                run_tail_visit_rank.set(interval, curr_visit_rank);
                --curr_visit_rank;
            }
            else {
                run_tail_visit_rank.set(interval, UNUSED_VISIT_RANK);
            }
        }
        --sa;
        pos = lf.LF(pos);
    }

    int_vec phi_img_rank_inv(lf.runs(), bit_width(lf.runs() - 1));
    // Step through BWT tail samples to fill in Phi interval permutations
    for (size_t i = 0; i < run_tail_visit_rank.size(); ++i) {
        if (run_tail_visit_rank.get(i) == UNUSED_VISIT_RANK) continue;
        phi_img_rank_inv[run_tail_visit_rank.get(i)] = move_run_to_phi.get((i == lf.intervals() - 1) ? 0 : i + 1);
    }

    if (domain != nullptr) {
        *domain = lf.domain();
    }
    if (max_length != nullptr) {
        *max_length = max_length_seen;
    }

    return {phi_lengths, phi_img_rank_inv};
}

template<typename alphabet_t=nucleotide>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_img_rank_inv(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, size_t* domain = nullptr, ulint* max_length = nullptr) {
    // Need a move structure with LF to find SA samples
    lf_move_impl_default<alphabet_t> move_lf(bwt_heads, bwt_run_lengths);
    return rlbwt_to_phi_img_rank_inv(bwt_heads, bwt_run_lengths, move_lf, domain, max_length);
}

template<typename lf_t>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_inv_images(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, lf_t& lf, size_t* domain = nullptr, ulint* max_length = nullptr) {
    int_vec phi_inv_lengths(lf.runs(), bit_width(lf.domain() - 1));
    
    size_t UNUSED_INTERVAL = max_val(bit_width(lf.intervals()));
    size_t UNUSED_SA = max_val(bit_width(lf.domain()));
    int_vec move_run_to_phi_inv(lf.intervals(), bit_width(lf.intervals())); // Map move run to its phi_inv interval (only set those corresponding to RLBWT runs)
    int_vec run_head_sa_samples(lf.intervals(), bit_width(lf.domain())); // The SA samples at the head of each move run (only set those corresponding to RLBWT runs)

    ulint max_length_seen = 0;
    auto pos = lf.first();
    size_t last_sample = lf.domain();
    size_t sa = lf.domain() - 1;
    // phi_inv intervals correspond to the original (unsplit) permutation runs, not move runs.
    size_t curr_phi_inv_interval = lf.runs() - 1;
    // Step through entire BWT to recover phi_inv structure and SA samples at heads
    for (size_t i = 0; i < lf.domain(); ++i) {
        size_t interval = pos.interval;
        size_t offset = pos.offset;
        // If at BWT tail
        if (offset == lf.get_length(interval) - 1) {
            if (interval == lf.intervals() - 1 || lf.get_character(interval + 1) != lf.get_character(interval)) {
                phi_inv_lengths[curr_phi_inv_interval] = last_sample - sa;
                max_length_seen = std::max(max_length_seen, static_cast<ulint>(phi_inv_lengths[curr_phi_inv_interval]));
                move_run_to_phi_inv.set(interval, curr_phi_inv_interval);
                last_sample = sa;
                --curr_phi_inv_interval;
            }
            else {
                move_run_to_phi_inv.set(interval, UNUSED_INTERVAL);
            }
        }
        // If at BWT run head
        if (offset == 0) {
            if (interval == 0 || lf.get_character(interval - 1) != lf.get_character(interval)) {
                run_head_sa_samples.set(interval, sa);
            }
            else {
                run_head_sa_samples.set(interval, UNUSED_SA);
            }
        }
        --sa;
        pos = lf.LF(pos);
    }

    int_vec phi_inv_images(lf.runs(), bit_width(lf.domain() - 1));
    // Step through BWT head samples to fill in Phi interval permutations
    for (size_t i = 0; i < run_head_sa_samples.size(); ++i) {
        if (run_head_sa_samples.get(i) == UNUSED_SA) continue;
        phi_inv_images[move_run_to_phi_inv.get((i == 0) ? lf.intervals() - 1 : i - 1)] = run_head_sa_samples.get(i);
    }

    if (domain != nullptr) {
        *domain = lf.domain();
    }
    if (max_length != nullptr) {
        *max_length = max_length_seen;
    }

    return {phi_inv_lengths, phi_inv_images};
}

template<typename alphabet_t=nucleotide>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_inv_images(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, size_t* domain = nullptr, ulint* max_length = nullptr) {
    // Need a move structure with LF to find SA samples
    lf_move_impl_default<alphabet_t> move_lf(bwt_heads, bwt_run_lengths);
    return rlbwt_to_phi_inv_images(bwt_heads, bwt_run_lengths, move_lf, domain, max_length);
}

template<typename lf_t>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_inv_img_rank_inv(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, lf_t& lf, size_t* domain = nullptr, ulint* max_length = nullptr) {
    int_vec phi_inv_lengths(lf.runs(), bit_width(lf.domain() - 1));
    
    size_t UNUSED_INTERVAL = max_val(bit_width(lf.intervals()));
    size_t UNUSED_VISIT_RANK = max_val(bit_width(lf.runs()));
    int_vec move_run_to_phi_inv(lf.intervals(), bit_width(lf.intervals())); // Map move run to its phi_inv interval (only set those corresponding to RLBWT runs)
    int_vec run_head_visit_rank(lf.intervals(), bit_width(lf.runs())); // The rank of the visit to the head of each move run (only set those corresponding to RLBWT runs)

    ulint max_length_seen = 0;
    auto pos = lf.first();
    size_t last_sample = lf.domain();
    size_t sa = lf.domain() - 1;
    // phi_inv intervals correspond to the original (unsplit) permutation runs, not move runs.
    size_t curr_phi_inv_interval = lf.runs() - 1;
    size_t curr_visit_rank = lf.runs() - 1;
    // Step through entire BWT to recover phi_inv structure and SA samples at heads
    for (size_t i = 0; i < lf.domain(); ++i) {
        size_t interval = pos.interval;
        size_t offset = pos.offset;
        // If at BWT tail
        if (offset == lf.get_length(interval) - 1) {
            if (interval == lf.intervals() - 1 || lf.get_character(interval + 1) != lf.get_character(interval)) {
                phi_inv_lengths[curr_phi_inv_interval] = last_sample - sa;
                max_length_seen = std::max(max_length_seen, static_cast<ulint>(phi_inv_lengths[curr_phi_inv_interval]));
                move_run_to_phi_inv.set(interval, curr_phi_inv_interval);
                last_sample = sa;
                --curr_phi_inv_interval;
            }
            else {
                move_run_to_phi_inv.set(interval, UNUSED_INTERVAL);
            }
        }
        // If at BWT run head
        if (offset == 0) {
            if (interval == 0 || lf.get_character(interval - 1) != lf.get_character(interval)) {
                run_head_visit_rank.set(interval, curr_visit_rank);
                --curr_visit_rank;
            }
            else {
                run_head_visit_rank.set(interval, UNUSED_VISIT_RANK);
            }
        }
        --sa;
        pos = lf.LF(pos);
    }

    int_vec phi_inv_img_rank_inv(lf.runs(), bit_width(lf.runs() - 1));
    // Step through BWT head samples to fill in Phi interval permutations
    for (size_t i = 0; i < run_head_visit_rank.size(); ++i) {
        if (run_head_visit_rank.get(i) == UNUSED_VISIT_RANK) continue;
        phi_inv_img_rank_inv[run_head_visit_rank.get(i)] = move_run_to_phi_inv.get((i == 0) ? lf.intervals() - 1 : i - 1);
    }

    if (domain != nullptr) {
        *domain = lf.domain();
    }
    if (max_length != nullptr) {
        *max_length = max_length_seen;
    }

    return {phi_inv_lengths, phi_inv_img_rank_inv};
}

template<typename alphabet_t=nucleotide>
inline std::tuple<int_vec, int_vec> rlbwt_to_phi_inv_img_rank_inv(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, size_t* domain = nullptr, ulint* max_length = nullptr) {
    // Need a move structure with LF to find SA samples
    lf_move_impl_default<alphabet_t> move_lf(bwt_heads, bwt_run_lengths);
    return rlbwt_to_phi_inv_img_rank_inv(bwt_heads, bwt_run_lengths, move_lf, domain, max_length);
}

/**
 * Suffix Array Sample Methods
 */

// From starting suffix array samples (BWT run head samples) and ending suffix array samples (BWT run tail samples) to Phi interval images
template<typename container1_t, typename container2_t>
inline std::tuple<int_vec, int_vec> sa_samples_to_phi_starts_images(const container1_t& sa_heads, const container2_t& sa_tails, size_t domain) {
    assert(sa_heads.size() == sa_tails.size());
    assert(!sa_heads.empty());
    assert(domain > 0);

    int_vec phi_interval_starts(sa_heads.size(), bit_width(domain - 1));
    int_vec phi_interval_images(sa_heads.size(), bit_width(domain - 1));

    int_vec head_order(sa_heads.size(), bit_width(sa_heads.size() - 1));
    std::iota(head_order.begin(), head_order.end(), 0);
    std::sort(head_order.begin(), head_order.end(), [&](size_t a, size_t b) {
        return sa_heads[a] < sa_heads[b];
    });

    for (size_t sorted_idx = 0; sorted_idx < head_order.size(); ++sorted_idx) {
        const size_t original_idx = head_order[sorted_idx];
        assert(static_cast<ulint>(sa_heads[original_idx]) < domain);
        assert(static_cast<ulint>(sa_tails[original_idx]) < domain);
        phi_interval_starts[sorted_idx] = sa_heads[original_idx];
        phi_interval_images[sorted_idx] = (original_idx == 0) ? sa_tails[sa_tails.size() - 1] : sa_tails[original_idx - 1];
    }

    return {phi_interval_starts, phi_interval_images};
}


template<typename container1_t, typename container2_t>
inline std::tuple<int_vec, int_vec, size_t> sa_samples_to_phi_starts_images(const container1_t& sa_heads, const container2_t& sa_tails) {
    assert(sa_heads.size() == sa_tails.size());
    assert(!sa_heads.empty());

    // Assume that terminator is of least order, else BWT is not valid
    size_t domain = sa_heads[0] + 1;
    auto [phi_starts, phi_images] = sa_samples_to_phi_starts_images(sa_heads, sa_tails, domain);
    return {phi_starts, phi_images, domain};
}

// Sort BWT run tail samples into Phi-inverse interval starts and carry along
// the corresponding BWT run head samples as interval images.
template<typename container1_t, typename container2_t>
inline std::tuple<int_vec, int_vec> sa_samples_to_phi_inv_starts_images(const container1_t& sa_heads, const container2_t& sa_tails, size_t domain) {
    assert(sa_heads.size() == sa_tails.size());
    assert(!sa_heads.empty());
    assert(domain > 0);

    int_vec phi_inv_interval_starts(sa_tails.size(), bit_width(domain - 1));
    int_vec phi_inv_interval_images(sa_tails.size(), bit_width(domain - 1));

    int_vec tail_order(sa_tails.size(), bit_width(sa_tails.size() - 1));
    std::iota(tail_order.begin(), tail_order.end(), 0);
    std::sort(tail_order.begin(), tail_order.end(), [&](size_t a, size_t b) {
        return sa_tails[a] < sa_tails[b];
    });

    for (size_t sorted_idx = 0; sorted_idx < tail_order.size(); ++sorted_idx) {
        const size_t original_idx = tail_order[sorted_idx];
        assert(static_cast<ulint>(sa_heads[original_idx]) < domain);
        assert(static_cast<ulint>(sa_tails[original_idx]) < domain);
        phi_inv_interval_starts[sorted_idx] = sa_tails[original_idx];
        phi_inv_interval_images[sorted_idx] = (original_idx == sa_heads.size() - 1) ? sa_heads[0] : sa_heads[original_idx + 1];
    }

    return {phi_inv_interval_starts, phi_inv_interval_images};
}

template<typename container1_t, typename container2_t>
inline std::tuple<int_vec, int_vec, size_t> sa_samples_to_phi_inv_starts_images(const container1_t& sa_heads, const container2_t& sa_tails) {
    assert(sa_heads.size() == sa_tails.size());
    assert(!sa_heads.empty());

    // Assume that terminator is of least order, else BWT is not valid
    size_t domain = sa_heads[0] + 1;
    auto [phi_inv_starts, phi_inv_images] = sa_samples_to_phi_inv_starts_images(sa_heads, sa_tails, domain);
    return {phi_inv_starts, phi_inv_images, domain};
}

} // namespace orbit::rlbwt

#endif /* end of include guard: _PHI_HELPERS_HPP */