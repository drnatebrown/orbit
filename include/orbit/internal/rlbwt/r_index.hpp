#ifndef ORBIT_INTERNAL_RLBWT_R_INDEX_HPP
#define ORBIT_INTERNAL_RLBWT_R_INDEX_HPP

#include "orbit/internal/rlbwt/lf_permutation.hpp"
#include "orbit/internal/rlbwt/phi_permutation.hpp"

#include "orbit/common.hpp"
#include "orbit/internal/ds/int_vector_aligned.hpp"

namespace orbit::rlbwt {

using int_vec = int_vector_aligned;

class r_index {
public:
    r_index(const std::vector<uchar>& rlbwt_heads, const std::vector<ulint>& rlbwt_run_lengths, const split_params& sp_lf = split_params(), const split_params& sp_phi = split_params()) {
        lf = lf_perm(rlbwt_heads, rlbwt_run_lengths, sp_lf);
        
        size_t domain;
        ulint max_length;
        int_vec interval_to_phi_run(lf.intervals(), bit_width(lf.runs()));
        auto [phi_lengths, phi_img_rank_inv] = rlbwt_to_phi_img_rank_inv(rlbwt_heads, rlbwt_run_lengths, lf, &domain, &max_length, &interval_to_phi_run);
        
        auto phi_enc = interval_encoding_impl<>::from_lengths_and_img_rank_inv(phi_lengths, phi_img_rank_inv, domain, max_length, sp_phi);
        phi = phi_perm(phi_enc);

        update_toeholds(phi_lengths, interval_to_phi_run);
    }

    ulint count(std::string &pattern) {
        sa_range range = {lf.first(), lf.last()};

        size_t m = pattern.size();
        for (size_t i = 0; i < pattern.size(); ++i) {
            auto new_top = lf_top(range.first, pattern[m - i - 1]);
            auto new_bottom = lf_bottom(range.second, pattern[m - i - 1]);

            if (!new_top.has_value() || !new_bottom.has_value()) {
                assert(!new_top.has_value() && !new_bottom.has_value());
                return 0;
            }
            range.first = new_top.value();
            range.second = new_bottom.value();
        }
        
        return count(range);
    }

    std::vector<ulint> locate(std::string &pattern) {
        sa_range range = {lf.first(), lf.last()};
        phi_position toehold = lf.get<toehold_cols::HEAD_PHI_INTERVAL>(range.second);

        size_t m = pattern.size();
        for (size_t i = 0; i < pattern.size(); ++i) {
            auto new_top = lf_top(range.first, pattern[m - i - 1]);
            auto new_bottom = lf_bottom_toehold(range.second, pattern[m - i - 1], toehold);
            if (!new_top.has_value() || !new_bottom.has_value()) {
                assert(!new_top.has_value() && !new_bottom.has_value());
                return {};
            }
            range.first = new_top.value();
            range.second = new_bottom.value();
        }

        std::vector<ulint> sa_locations;
        while (range.second >= range.first) {
            sa_locations.push_back(phi.SA(toehold));
            toehold = phi.next(toehold);
        }
        return sa_locations;
    }

private:
    DEFINE_ORBIT_COLUMNS(toehold_cols, HEAD_PHI_INTERVAL);
    using toehold_data = orbit::columns_tuple<toehold_cols>;

    using lf_perm = orbit::rlbwt::lf_permutation_impl<toehold_cols>;
    using phi_perm = orbit::rlbwt::phi_permutation_impl<empty_data_columns, false, true>;
    using lf_position = typename lf_perm::position;
    using phi_position = typename phi_perm::position;

    using sa_range = std::pair<lf_position, lf_position>;

    lf_perm lf;
    phi_perm phi;

    std::optional<lf_position> lf_top(lf_position pos, uchar c) {
        auto succ_pos = lf.succ_char(pos, c);
        if (succ_pos.has_value()) {
            return lf.LF(succ_pos.value());
        }
        else {
            return std::nullopt;
        }  
    }

    std::optional<lf_position> lf_bottom(lf_position pos, uchar c) {
        auto pred_pos = lf.pred_char(pos, c);
        if (pred_pos.has_value()) {
            return lf.LF(pred_pos.value());
        }
        else {
            return std::nullopt;
        }
    }

    std::optional<lf_position> lf_bottom_toehold(lf_position pos, uchar c, phi_position &toehold) {
        auto pred_pos = lf.pred_char(pos, c);
        if (pred_pos.has_value()) {
            if (pred_pos.value() != toehold) {
                auto next_head = lf.down(pred_pos);
                toehold = {lf.get<toehold_cols::HEAD_PHI_INTERVAL>(next_head), 0};
                toehold = phi.next(toehold);
            }
            else {
                toehold = phi.down(toehold);
            }
            return lf.LF(pred_pos.value());
        }
        else {
            return std::nullopt;
        }
    }

    ulint count(sa_range range) {
        ulint count = (lf.get_length(range.first) - range.first.offset) + (range.second.offset + 1);
        if (range.first.interval == range.second.interval) {
            return count;
        }

        for (size_t i = range.first.interval + 1; i <= range.second.interval - 1; ++i) {
            count += lf.get_length(i);
        }
        return count;
    }

    void update_toeholds(int_vec &phi_lengths, int_vec &lf_interval_to_phi_run) {
        int_vec orig_interval_to_new_interval(phi_lengths.size(), bit_width(phi.intervals() - 1));

        size_t curr_split_interval = 0;
        for (size_t i = 0; i < phi_lengths.size(); ++i) {
            orig_interval_to_new_interval.set(i, curr_split_interval);
            
            ulint curr_length = phi_lengths[i];
            while (curr_length > 0) {
                curr_length -= phi.get_length(curr_split_interval);
                ++curr_split_interval;
            }
        }
        assert(curr_split_interval == phi.intervals());

        size_t UNUSED_PHI_RUN = phi.runs();
        size_t UNUSED_PHI_INTERVAL = phi.intervals();
        lf.init_data({bit_width(phi.intervals())});
        for (size_t i = 0; i < lf.intervals(); ++i) {
            if (lf_interval_to_phi_run.get(i) != UNUSED_PHI_RUN) {
                lf.set<toehold_cols::HEAD_PHI_INTERVAL>(i, orig_interval_to_new_interval.get(lf_interval_to_phi_run.get(i)));
            }
            else {
                lf.set<toehold_cols::HEAD_PHI_INTERVAL>(i, UNUSED_PHI_INTERVAL);
            }
        }
    }
};

} // namespace orbit::rlbwt
#endif // ORBIT_INTERNAL_RLBWT_R_INDEX_HPP