#ifndef _INVERTIBLE_RLBWT_ROW_HPP
#define _INVERTIBLE_RLBWT_ROW_HPP

#include "orbit/internal/rlbwt/specializations/rlbwt_row.hpp"

namespace orbit::rlbwt {

template <typename columns_t, size_t CHARACTER_BITS, size_t PRIMARY_BITS, size_t POINTER_BITS>
struct invertible_rlbwt_row_bits {};

template <typename columns_t, size_t CHARACTER_BITS, size_t PRIMARY_BITS, size_t POINTER_BITS, size_t OFFSET_BITS>
struct invertible_rlbwt_offset_row_bits {};

} // namespace orbit::rlbwt

namespace orbit {

template <typename columns_t, size_t C, size_t P, size_t PTR>
struct move_row_traits<rlbwt::invertible_rlbwt_row_bits<columns_t, C, P, PTR>> {
    static constexpr size_t CHARACTER_BITS = C;
    static constexpr size_t PRIMARY_BITS = P;
    static constexpr size_t POINTER_FWD_BITS = PTR;
    static constexpr size_t POINTER_INV_BITS = PTR;
    static constexpr size_t FWD_INTERVAL_BITS = 1;
    static constexpr size_t INV_INTERVAL_BITS = 1;
};

template <typename columns_t, size_t C, size_t P, size_t PTR, size_t OFF>
struct move_row_traits<rlbwt::invertible_rlbwt_offset_row_bits<columns_t, C, P, PTR, OFF>> {
    static constexpr size_t CHARACTER_BITS = C;
    static constexpr size_t PRIMARY_BITS = P;
    static constexpr size_t POINTER_FWD_BITS = PTR;
    static constexpr size_t POINTER_INV_BITS = PTR;
    static constexpr size_t OFFSET_BITS = OFF;
    static constexpr size_t FWD_INTERVAL_BITS = 1;
    static constexpr size_t INV_INTERVAL_BITS = 1;
};

} // namespace orbit

namespace orbit::rlbwt {

template <typename columns_t,
         bool StoreOffsets = move_cols_traits<columns_t>::STORE_OFFSETS,
         bool UseSpillover = move_cols_traits<columns_t>::USE_SPILLOVER>
struct invertible_rlbwt_row;

template <typename columns_t>
struct invertible_rlbwt_row<columns_t, true, false> {
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer_fwd : row_traits::POINTER_FWD_BITS;
    ulint pointer_inv : row_traits::POINTER_INV_BITS;
    ulint offset : row_traits::OFFSET_BITS;
    ulint fwd_interval : row_traits::FWD_INTERVAL_BITS;
    ulint inv_interval : row_traits::INV_INTERVAL_BITS;
    ulint character : row_traits::CHARACTER_BITS;

    invertible_rlbwt_row() = default;
    invertible_rlbwt_row(const std::array<ulint, num_cols>& values) { set(values); }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER_FWD) pointer_fwd = val;
        else if constexpr (col == cols_traits::POINTER_INV) pointer_inv = val;
        else if constexpr (col == cols_traits::OFFSET) offset = val;
        else if constexpr (col == cols_traits::FWD_INTERVAL) fwd_interval = val;
        else if constexpr (col == cols_traits::INV_INTERVAL) inv_interval = val;
        else if constexpr (col == cols_traits::CHARACTER) character = val;
    }
    template <size_t... indices>
    void set(const std::array<ulint, num_cols>& values, std::index_sequence<indices...>) {
        (set<static_cast<columns>(indices)>(values[indices]), ...);
    }
    void set(const std::array<ulint, num_cols>& values) {
        set(values, std::make_index_sequence<num_cols>{});
    }

    template <columns col>
    ulint get() const {
        if constexpr (col == cols_traits::PRIMARY) return primary;
        else if constexpr (col == cols_traits::POINTER_FWD) return pointer_fwd;
        else if constexpr (col == cols_traits::POINTER_INV) return pointer_inv;
        else if constexpr (col == cols_traits::OFFSET) return offset;
        else if constexpr (col == cols_traits::FWD_INTERVAL) return fwd_interval;
        else if constexpr (col == cols_traits::INV_INTERVAL) return inv_interval;
        else if constexpr (col == cols_traits::CHARACTER) return character;
        else throw std::invalid_argument("Invalid column");
    }
    template <size_t... indices>
    std::array<ulint, num_cols> get(std::index_sequence<indices...>) const {
        return {get<static_cast<columns>(indices)>()...};
    }
    std::array<ulint, num_cols> get() const {
        return get(std::make_index_sequence<num_cols>{});
    }

    static const std::array<uchar, num_cols>& get_widths() {
        static const std::array<uchar, num_cols> widths{
            static_cast<uchar>(row_traits::PRIMARY_BITS),
            static_cast<uchar>(row_traits::POINTER_FWD_BITS),
            static_cast<uchar>(row_traits::POINTER_INV_BITS),
            static_cast<uchar>(row_traits::OFFSET_BITS),
            static_cast<uchar>(row_traits::FWD_INTERVAL_BITS),
            static_cast<uchar>(row_traits::INV_INTERVAL_BITS),
            static_cast<uchar>(row_traits::CHARACTER_BITS),
        };
        return widths;
    }

    static void assert_widths(const std::array<uchar, num_cols>& widths) {
        assert(widths[static_cast<size_t>(cols_traits::PRIMARY)] <= row_traits::PRIMARY_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_FWD)] <= row_traits::POINTER_FWD_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_INV)] <= row_traits::POINTER_INV_BITS);
        assert(widths[static_cast<size_t>(cols_traits::OFFSET)] <= row_traits::OFFSET_BITS);
        assert(widths[static_cast<size_t>(cols_traits::FWD_INTERVAL)] <= row_traits::FWD_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::INV_INTERVAL)] <= row_traits::INV_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::CHARACTER)] <= row_traits::CHARACTER_BITS);
    }

} __attribute__((packed));

template <typename columns_t>
struct invertible_rlbwt_row<columns_t, false, false> {
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer_fwd : row_traits::POINTER_FWD_BITS;
    ulint pointer_inv : row_traits::POINTER_INV_BITS;
    ulint fwd_interval : row_traits::FWD_INTERVAL_BITS;
    ulint inv_interval : row_traits::INV_INTERVAL_BITS;
    ulint character : row_traits::CHARACTER_BITS;

    invertible_rlbwt_row() = default;
    invertible_rlbwt_row(const std::array<ulint, num_cols>& values) { set(values); }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER_FWD) pointer_fwd = val;
        else if constexpr (col == cols_traits::POINTER_INV) pointer_inv = val;
        else if constexpr (col == cols_traits::FWD_INTERVAL) fwd_interval = val;
        else if constexpr (col == cols_traits::INV_INTERVAL) inv_interval = val;
        else if constexpr (col == cols_traits::CHARACTER) character = val;
    }
    template <size_t... indices>
    void set(const std::array<ulint, num_cols>& values, std::index_sequence<indices...>) {
        (set<static_cast<columns>(indices)>(values[indices]), ...);
    }
    void set(const std::array<ulint, num_cols>& values) {
        set(values, std::make_index_sequence<num_cols>{});
    }

    template <columns col>
    ulint get() const {
        if constexpr (col == cols_traits::PRIMARY) return primary;
        else if constexpr (col == cols_traits::POINTER_FWD) return pointer_fwd;
        else if constexpr (col == cols_traits::POINTER_INV) return pointer_inv;
        else if constexpr (col == cols_traits::FWD_INTERVAL) return fwd_interval;
        else if constexpr (col == cols_traits::INV_INTERVAL) return inv_interval;
        else if constexpr (col == cols_traits::CHARACTER) return character;
        else throw std::invalid_argument("Invalid column");
    }
    template <size_t... indices>
    std::array<ulint, num_cols> get(std::index_sequence<indices...>) const {
        return {get<static_cast<columns>(indices)>()...};
    }
    std::array<ulint, num_cols> get() const {
        return get(std::make_index_sequence<num_cols>{});
    }

    static const std::array<uchar, num_cols>& get_widths() {
        static const std::array<uchar, num_cols> widths{
            static_cast<uchar>(row_traits::PRIMARY_BITS),
            static_cast<uchar>(row_traits::POINTER_FWD_BITS),
            static_cast<uchar>(row_traits::POINTER_INV_BITS),
            static_cast<uchar>(row_traits::FWD_INTERVAL_BITS),
            static_cast<uchar>(row_traits::INV_INTERVAL_BITS),
            static_cast<uchar>(row_traits::CHARACTER_BITS),
        };
        return widths;
    }

    static void assert_widths(const std::array<uchar, num_cols>& widths) {
        assert(widths[static_cast<size_t>(cols_traits::PRIMARY)] <= row_traits::PRIMARY_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_FWD)] <= row_traits::POINTER_FWD_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_INV)] <= row_traits::POINTER_INV_BITS);
        assert(widths[static_cast<size_t>(cols_traits::FWD_INTERVAL)] <= row_traits::FWD_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::INV_INTERVAL)] <= row_traits::INV_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::CHARACTER)] <= row_traits::CHARACTER_BITS);
    }

} __attribute__((packed));

template <typename columns_t>
struct invertible_rlbwt_row<columns_t, false, true> {
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer_fwd : row_traits::POINTER_FWD_BITS;
    ulint fwd_interval : row_traits::FWD_INTERVAL_BITS;
    ulint inv_interval : row_traits::INV_INTERVAL_BITS;
    ulint character : row_traits::CHARACTER_BITS;

    invertible_rlbwt_row() = default;
    invertible_rlbwt_row(const std::array<ulint, num_cols>& values) { set(values); }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER_FWD) pointer_fwd = val;
        else if constexpr (col == cols_traits::FWD_INTERVAL) fwd_interval = val;
        else if constexpr (col == cols_traits::INV_INTERVAL) inv_interval = val;
        else if constexpr (col == cols_traits::CHARACTER) character = val;
    }
    template <size_t... indices>
    void set(const std::array<ulint, num_cols>& values, std::index_sequence<indices...>) {
        (set<static_cast<columns>(indices)>(values[indices]), ...);
    }
    void set(const std::array<ulint, num_cols>& values) {
        set(values, std::make_index_sequence<num_cols>{});
    }

    template <columns col>
    ulint get() const {
        if constexpr (col == cols_traits::PRIMARY) return primary;
        else if constexpr (col == cols_traits::POINTER_FWD) return pointer_fwd;
        else if constexpr (col == cols_traits::FWD_INTERVAL) return fwd_interval;
        else if constexpr (col == cols_traits::INV_INTERVAL) return inv_interval;
        else if constexpr (col == cols_traits::CHARACTER) return character;
        else throw std::invalid_argument("Invalid column");
    }
    template <size_t... indices>
    std::array<ulint, num_cols> get(std::index_sequence<indices...>) const {
        return {get<static_cast<columns>(indices)>()...};
    }
    std::array<ulint, num_cols> get() const {
        return get(std::make_index_sequence<num_cols>{});
    }

    static const std::array<uchar, num_cols>& get_widths() {
        static const std::array<uchar, num_cols> widths{
            static_cast<uchar>(row_traits::PRIMARY_BITS),
            static_cast<uchar>(row_traits::POINTER_FWD_BITS),
            static_cast<uchar>(row_traits::FWD_INTERVAL_BITS),
            static_cast<uchar>(row_traits::INV_INTERVAL_BITS),
            static_cast<uchar>(row_traits::CHARACTER_BITS),
        };
        return widths;
    }

    static void assert_widths(const std::array<uchar, num_cols>& widths) {
        assert(widths[static_cast<size_t>(cols_traits::PRIMARY)] <= row_traits::PRIMARY_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_FWD)] <= row_traits::POINTER_FWD_BITS);
        assert(widths[static_cast<size_t>(cols_traits::FWD_INTERVAL)] <= row_traits::FWD_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::INV_INTERVAL)] <= row_traits::INV_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::CHARACTER)] <= row_traits::CHARACTER_BITS);
    }

} __attribute__((packed));

using invertible_dna_seq_columns_15 = invertible_rlbwt_offset_row_bits<invertible_rlbwt_columns, 3, 19, 40, 16>;
using invertible_dna_seq_cols_idx_18 = invertible_rlbwt_offset_row_bits<invertible_rlbwt_columns_idx, 3, 45, 39, 16>;
using invertible_rlbwt_cols_15 = invertible_rlbwt_offset_row_bits<invertible_rlbwt_columns, 8, 16, 39, 16>;
using invertible_rlbwt_cols_idx_18 = invertible_rlbwt_offset_row_bits<invertible_rlbwt_columns_idx, 8, 42, 38, 16>;
using invertible_dna_seq_columns_scan_13 = invertible_rlbwt_row_bits<invertible_rlbwt_columns_scan, 3, 19, 40>;
using invertible_dna_seq_cols_idx_scan_16 = invertible_rlbwt_row_bits<invertible_rlbwt_columns_idx_scan, 3, 45, 39>;
using invertible_dna_seq_columns_spill_11 = invertible_rlbwt_row_bits<invertible_rlbwt_columns_spill, 3, 19, 40>;
using invertible_dna_seq_cols_idx_spill_14 = invertible_rlbwt_row_bits<invertible_rlbwt_columns_idx_spill, 3, 45, 39>;

using invertible_rlbwt_columns_default = invertible_dna_seq_columns_15;
using invertible_rlbwt_columns_idx_default = invertible_dna_seq_cols_idx_18;
using invertible_rlbwt_columns_scan_default = invertible_dna_seq_columns_scan_13;
using invertible_rlbwt_columns_idx_scan_default = invertible_dna_seq_cols_idx_scan_16;
using invertible_rlbwt_columns_spill_default = invertible_dna_seq_columns_spill_11;
using invertible_rlbwt_columns_idx_spill_default = invertible_dna_seq_cols_idx_spill_14;

} // namespace orbit::rlbwt

namespace orbit {
template <>
struct move_row_traits<rlbwt::invertible_rlbwt_columns> : move_row_traits<rlbwt::invertible_rlbwt_columns_default> {};
template <>
struct move_row_traits<rlbwt::invertible_rlbwt_columns_idx> : move_row_traits<rlbwt::invertible_rlbwt_columns_idx_default> {};
template <>
struct move_row_traits<rlbwt::invertible_rlbwt_columns_scan> : move_row_traits<rlbwt::invertible_rlbwt_columns_scan_default> {};
template <>
struct move_row_traits<rlbwt::invertible_rlbwt_columns_idx_scan> : move_row_traits<rlbwt::invertible_rlbwt_columns_idx_scan_default> {};
template <>
struct move_row_traits<rlbwt::invertible_rlbwt_columns_spill> : move_row_traits<rlbwt::invertible_rlbwt_columns_spill_default> {};
template <>
struct move_row_traits<rlbwt::invertible_rlbwt_columns_idx_spill> : move_row_traits<rlbwt::invertible_rlbwt_columns_idx_spill_default> {};

template<>
struct table_row_for<rlbwt::invertible_rlbwt_columns> {
    using type = rlbwt::invertible_rlbwt_row<rlbwt::invertible_rlbwt_columns>;
};
template<>
struct table_row_for<rlbwt::invertible_rlbwt_columns_idx> {
    using type = rlbwt::invertible_rlbwt_row<rlbwt::invertible_rlbwt_columns_idx>;
};
template<>
struct table_row_for<rlbwt::invertible_rlbwt_columns_scan> {
    using type = rlbwt::invertible_rlbwt_row<rlbwt::invertible_rlbwt_columns_scan>;
};
template<>
struct table_row_for<rlbwt::invertible_rlbwt_columns_idx_scan> {
    using type = rlbwt::invertible_rlbwt_row<rlbwt::invertible_rlbwt_columns_idx_scan>;
};
template<>
struct table_row_for<rlbwt::invertible_rlbwt_columns_spill> {
    using type = rlbwt::invertible_rlbwt_row<rlbwt::invertible_rlbwt_columns_spill>;
};
template<>
struct table_row_for<rlbwt::invertible_rlbwt_columns_idx_spill> {
    using type = rlbwt::invertible_rlbwt_row<rlbwt::invertible_rlbwt_columns_idx_spill>;
};
} // namespace orbit

#endif /* _INVERTIBLE_RLBWT_ROW_HPP */
