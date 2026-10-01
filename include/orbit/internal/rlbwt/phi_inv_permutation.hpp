#ifndef _PERMUTATION_PHI_INV_HPP
#define _PERMUTATION_PHI_INV_HPP

#include "orbit/common.hpp"
#include "orbit/internal/perm/permutation_impl.hpp"
#include "orbit/internal/rlbwt/phi_helpers.hpp"

namespace orbit::rlbwt {

// Absolute positions by default. Relative positions are supported when
// store_ef_high_bits is set (length capping required); SA() uses get_idx.
template<typename data_columns_t = empty_data_columns,
         bool integrated_move_structure = DEFAULT_INTEGRATED_MOVE_STRUCTURE,
         bool exponential_search = DEFAULT_EXPONENTIAL_SEARCH,
         typename base_columns_t = move_columns,
         template<typename, template<typename> class> class move_structure_t = move_structure,
         template<typename> class table_t = move_vector,
         invertible_space_mode space_mode = DEFAULT_INVERTIBLE_SPACE,
         bool store_absolute_positions = true>
class phi_inv_permutation_impl : public permutation_impl<data_columns_t, integrated_move_structure, store_absolute_positions, exponential_search, base_columns_t, move_structure_t, table_t, space_mode> {
    using base = permutation_impl<data_columns_t, integrated_move_structure, store_absolute_positions, exponential_search, base_columns_t, move_structure_t, table_t, space_mode>;
public:
    using base::base;
    using base::operator=;
    using position = typename base::position;
    using interval_encoding_t = typename base::interval_encoding_t;

    template<class dc = data_columns_t,
             std::enable_if_t<std::is_same_v<dc, empty_data_columns>, int> = 0>
    phi_inv_permutation_impl(const std::vector<uchar>& rlbwt_heads,
        const std::vector<ulint>& rlbwt_run_lengths,
        const split_params& sp = split_params(),
        bool store_ef_high_bits = false)
    : base([&] {
        size_t domain = 0;
        ulint max_length = 0;
        auto [phi_inv_lengths, phi_inv_img_rank_inv] =
            rlbwt_to_phi_inv_img_rank_inv<>(rlbwt_heads, rlbwt_run_lengths,
                                            &domain, &max_length);
        return interval_encoding_t::from_lengths_and_img_rank_inv(
            phi_inv_lengths, phi_inv_img_rank_inv, domain, max_length, sp);
    }(), store_ef_high_bits) {}

    position phi_inv(position pos) {
        return base::next(pos);
    }

    position phi_inv(position pos, ulint steps) {
        return base::next(pos, steps);
    }

    ulint SA(position pos) {
        return base::get_idx(pos);
    }
};

template<bool exponential_search = DEFAULT_EXPONENTIAL_SEARCH,
         typename base_columns_t = move_columns,
         template<typename, template<typename> class> class move_structure_t = move_structure,
         template<typename> class table_t = move_vector,
         invertible_space_mode space_mode = DEFAULT_INVERTIBLE_SPACE,
         bool store_absolute_positions = true>
using phi_inv_move_impl = phi_inv_permutation_impl<empty_data_columns, false, exponential_search, base_columns_t, move_structure_t, table_t, space_mode, store_absolute_positions>;

using phi_inv_move_relative = phi_inv_move_impl<DEFAULT_EXPONENTIAL_SEARCH, move_columns, move_structure, move_vector, DEFAULT_INVERTIBLE_SPACE, false>;

} // namespace orbit::rlbwt

#endif /* end of include guard: _PERMUTATION_PHI_INV_HPP */
