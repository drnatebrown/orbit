#ifndef _RLBWT_INTERVAL_ENCODING_HPP
#define _RLBWT_INTERVAL_ENCODING_HPP

#include "orbit/common.hpp"
#include "orbit/internal/move/interval_encoding_impl.hpp"
#include "orbit/internal/ds/alphabet.hpp"
#include "orbit/internal/rlbwt/rlbwt_helpers.hpp"

namespace orbit::rlbwt {

template<bool invertible=false,typename int_vector_t = int_vector_aligned, typename alphabet_t = nucleotide>
class rlbwt_interval_encoding_impl : public interval_encoding_impl<invertible, int_vector_t> {
    using base = interval_encoding_impl<invertible, int_vector_t>;
public:
    using alphabet_tag = alphabet_t;

    rlbwt_interval_encoding_impl() = default;

    // When heads / lengths / img_rank_inv / alphabet were computed independently
    // (e.g. LF or FL preprocess already done). Lengths and img_rank_inv are
    // pre-split; splitting is applied here.
    template<typename container1_t, typename container2_t, typename container3_t>
    static rlbwt_interval_encoding_impl<invertible, int_vector_t, alphabet_t>
    from_heads_lengths_and_img_rank_inv(const container1_t& heads,
                                        const container2_t& lengths,
                                        const container3_t& img_rank_inv,
                                        alphabet_t alphabet,
                                        const size_t domain,
                                        const ulint max_length,
                                        const split_params& sp = split_params()) {
        assert(heads.size() == lengths.size());
        assert(lengths.size() == img_rank_inv.size());

        rlbwt_interval_encoding_impl<invertible, int_vector_t, alphabet_t> enc;
        enc.set_initial_values(domain, lengths.size(), max_length, sp);
        enc.init_img_rank_inv(lengths, img_rank_inv);
        enc.alphabet_ = std::move(alphabet);
        enc.init_heads(heads, lengths);
        return enc;
    }

    template<typename container1_t, typename container2_t>
    static rlbwt_interval_encoding_impl<invertible, int_vector_t, alphabet_t> lf_interval_encoding(const container1_t& rlbwt_heads, const container2_t& rlbwt_run_lengths, const split_params& sp = split_params()) {
        assert(rlbwt_heads.size() == rlbwt_run_lengths.size());

        auto [head_counts, n, max_length] = get_LF_head_counts(rlbwt_heads, rlbwt_run_lengths);
        int_vector_t img_rank_inv = get_LF_img_rank_inv(rlbwt_heads, head_counts);
        return from_heads_lengths_and_img_rank_inv(rlbwt_heads, rlbwt_run_lengths, img_rank_inv,
                                                   alphabet_t(head_counts), n, max_length, sp);
    }

    template<typename container1_t, typename container2_t>
    static rlbwt_interval_encoding_impl<invertible, int_vector_t, alphabet_t> fl_interval_encoding(const container1_t& rlbwt_heads, const container2_t& rlbwt_run_lengths, const split_params& sp = split_params()) {
        assert(rlbwt_heads.size() == rlbwt_run_lengths.size());

        auto [head_counts, n, max_length] = get_FL_head_counts(rlbwt_heads, rlbwt_run_lengths);
        auto [F_heads, F_lens, F_img_rank_inv] = get_FL_runs_and_img_rank_inv<container1_t, container2_t, int_vector_t>(rlbwt_heads, rlbwt_run_lengths, head_counts, max_length);
        return from_heads_lengths_and_img_rank_inv(F_heads, F_lens, F_img_rank_inv,
                                                   alphabet_t(head_counts), n, max_length, sp);
    }

    const int_vector_t& get_heads() const {
        return heads_;
    }

    const alphabet_t& get_alphabet() const {
        return alphabet_;
    }

    const ulint sigma() const {
        return static_cast<ulint>(alphabet_.size());
    }

private:
    alphabet_t alphabet_;
    int_vector_t heads_;

    template<typename collection1_t, typename collection2_t>
    void init_heads(const collection1_t& heads, const collection2_t& original_lengths) {
        heads_ = int_vector_t(base::intervals(), bit_width(alphabet_.size() - 1));
        size_t new_interval_idx = 0;
        for (size_t i = 0; i < original_lengths.size(); ++i) {
            ulint remaining = original_lengths[i];
            while (remaining > 0) {
                ulint chunk = base::get_length(new_interval_idx);
                heads_.set(new_interval_idx, alphabet_.map_char(heads[i]));
                remaining -= chunk;
                ++new_interval_idx;
            }
        }
        assert(new_interval_idx == base::intervals());
    }
};

} // namespace orbit::rlbwt

#endif /* end of include guard: _RLBWT_INTERVAL_ENCODING_HPP */