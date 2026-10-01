#ifndef _INVERTIBLE_ROW_HPP
#define _INVERTIBLE_ROW_HPP

#include "orbit/internal/move/move_row.hpp"

namespace orbit {

// ============================================= INVERTIBLE =============================================

// Scan layout (no OFFSET). Offset layout adds OFFSET_BITS.
template <typename columns_t, size_t PRIMARY_BITS, size_t POINTER_BITS>
struct invertible_row_bits {};

template <typename columns_t, size_t PRIMARY_BITS, size_t POINTER_BITS, size_t OFFSET_BITS>
struct invertible_offset_row_bits {};

template <typename columns_t, size_t P, size_t PTR>
struct move_row_traits<invertible_row_bits<columns_t, P, PTR>> {
    static constexpr size_t PRIMARY_BITS = P;
    static constexpr size_t POINTER_FWD_BITS = PTR;
    static constexpr size_t POINTER_INV_BITS = PTR;

    static constexpr size_t FWD_INTERVAL_BITS = 1;
    static constexpr size_t INV_INTERVAL_BITS = 1;
};

template <typename columns_t, size_t P, size_t PTR, size_t OFF>
struct move_row_traits<invertible_offset_row_bits<columns_t, P, PTR, OFF>> {
    static constexpr size_t PRIMARY_BITS = P;
    static constexpr size_t POINTER_FWD_BITS = PTR;
    static constexpr size_t POINTER_INV_BITS = PTR;
    static constexpr size_t OFFSET_BITS = OFF;

    static constexpr size_t FWD_INTERVAL_BITS = 1;
    static constexpr size_t INV_INTERVAL_BITS = 1;
};

template <typename columns_t,
         bool StoreOffsets = move_cols_traits<columns_t>::STORE_OFFSETS,
         bool UseSpillover = move_cols_traits<columns_t>::USE_SPILLOVER>
struct invertible_row;

template <typename columns_t>
struct invertible_row<columns_t, true, false> {
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer_fwd : row_traits::POINTER_FWD_BITS;
    ulint pointer_inv : row_traits::POINTER_INV_BITS;
    ulint offset : row_traits::OFFSET_BITS;
    ulint fwd_interval : row_traits::FWD_INTERVAL_BITS;
    ulint inv_interval : row_traits::INV_INTERVAL_BITS;

    invertible_row() = default;
    invertible_row(const std::array<ulint, num_cols>& values) {
        set(values);
    }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER_FWD) pointer_fwd = val;
        else if constexpr (col == cols_traits::POINTER_INV) pointer_inv = val;
        else if constexpr (col == cols_traits::OFFSET) offset = val;
        else if constexpr (col == cols_traits::FWD_INTERVAL) fwd_interval = val;
        else if constexpr (col == cols_traits::INV_INTERVAL) inv_interval = val;
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
    }

} __attribute__((packed));

template <typename columns_t>
struct invertible_row<columns_t, false, false> {
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer_fwd : row_traits::POINTER_FWD_BITS;
    ulint pointer_inv : row_traits::POINTER_INV_BITS;
    ulint fwd_interval : row_traits::FWD_INTERVAL_BITS;
    ulint inv_interval : row_traits::INV_INTERVAL_BITS;

    invertible_row() = default;
    invertible_row(const std::array<ulint, num_cols>& values) {
        set(values);
    }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER_FWD) pointer_fwd = val;
        else if constexpr (col == cols_traits::POINTER_INV) pointer_inv = val;
        else if constexpr (col == cols_traits::FWD_INTERVAL) fwd_interval = val;
        else if constexpr (col == cols_traits::INV_INTERVAL) inv_interval = val;
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
        };
        return widths;
    }

    static void assert_widths(const std::array<uchar, num_cols>& widths) {
        assert(widths[static_cast<size_t>(cols_traits::PRIMARY)] <= row_traits::PRIMARY_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_FWD)] <= row_traits::POINTER_FWD_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_INV)] <= row_traits::POINTER_INV_BITS);
        assert(widths[static_cast<size_t>(cols_traits::FWD_INTERVAL)] <= row_traits::FWD_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::INV_INTERVAL)] <= row_traits::INV_INTERVAL_BITS);
    }

} __attribute__((packed));

// Spill columns: one live pointer cell; dual heads index into the spill table.
template <typename columns_t>
struct invertible_row<columns_t, false, true> {
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer_fwd : row_traits::POINTER_FWD_BITS;
    ulint fwd_interval : row_traits::FWD_INTERVAL_BITS;
    ulint inv_interval : row_traits::INV_INTERVAL_BITS;

    invertible_row() = default;
    invertible_row(const std::array<ulint, num_cols>& values) {
        set(values);
    }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER_FWD) pointer_fwd = val;
        else if constexpr (col == cols_traits::FWD_INTERVAL) fwd_interval = val;
        else if constexpr (col == cols_traits::INV_INTERVAL) inv_interval = val;
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
        };
        return widths;
    }

    static void assert_widths(const std::array<uchar, num_cols>& widths) {
        assert(widths[static_cast<size_t>(cols_traits::PRIMARY)] <= row_traits::PRIMARY_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER_FWD)] <= row_traits::POINTER_FWD_BITS);
        assert(widths[static_cast<size_t>(cols_traits::FWD_INTERVAL)] <= row_traits::FWD_INTERVAL_BITS);
        assert(widths[static_cast<size_t>(cols_traits::INV_INTERVAL)] <= row_traits::INV_INTERVAL_BITS);
    }

} __attribute__((packed));

// Mode A (offset): 16+39+39+16+1+1 = 112 bits (14 bytes)
using invertible_cols_14 = invertible_offset_row_bits<invertible_columns, 16, 39, 16>;
using invertible_cols_default = invertible_cols_14;

// Scan: 16+39+39+1+1 = 96 bits (12 bytes)
using invertible_cols_scan_12 = invertible_row_bits<invertible_columns_scan, 16, 39>;
using invertible_cols_scan_default = invertible_cols_scan_12;

// Spill: 16+39+1+1 = 57 bits (8 bytes packed)
using invertible_cols_spill_8 = invertible_row_bits<invertible_columns_spill, 16, 39>;
using invertible_cols_spill_default = invertible_cols_spill_8;

// Mode A absolute: 42+38+38+16+1+1 = 136 bits (17 bytes)
using invertible_cols_idx_17 = invertible_offset_row_bits<invertible_columns_idx, 42, 38, 16>;
using invertible_cols_idx_default = invertible_cols_idx_17;

// Scan absolute: 42+38+38+1+1 = 120 bits (15 bytes)
using invertible_cols_idx_scan_15 = invertible_row_bits<invertible_columns_idx_scan, 42, 38>;
using invertible_cols_idx_scan_default = invertible_cols_idx_scan_15;

// Spill absolute: 42+38+1+1 = 82 bits (11 bytes packed)
using invertible_cols_idx_spill_11 = invertible_row_bits<invertible_columns_idx_spill, 42, 38>;
using invertible_cols_idx_spill_default = invertible_cols_idx_spill_11;

template <>
struct move_row_traits<invertible_columns> : move_row_traits<invertible_cols_default> {};

template <>
struct move_row_traits<invertible_columns_idx> : move_row_traits<invertible_cols_idx_default> {};

template <>
struct move_row_traits<invertible_columns_scan> : move_row_traits<invertible_cols_scan_default> {};

template <>
struct move_row_traits<invertible_columns_idx_scan> : move_row_traits<invertible_cols_idx_scan_default> {};

template <>
struct move_row_traits<invertible_columns_spill> : move_row_traits<invertible_cols_spill_default> {};

template <>
struct move_row_traits<invertible_columns_idx_spill> : move_row_traits<invertible_cols_idx_spill_default> {};

template<>
struct table_row_for<invertible_columns> {
    using type = invertible_row<invertible_columns>;
};

template<>
struct table_row_for<invertible_columns_idx> {
    using type = invertible_row<invertible_columns_idx>;
};

template<>
struct table_row_for<invertible_columns_scan> {
    using type = invertible_row<invertible_columns_scan>;
};

template<>
struct table_row_for<invertible_columns_idx_scan> {
    using type = invertible_row<invertible_columns_idx_scan>;
};

template<>
struct table_row_for<invertible_columns_spill> {
    using type = invertible_row<invertible_columns_spill>;
};

template<>
struct table_row_for<invertible_columns_idx_spill> {
    using type = invertible_row<invertible_columns_idx_spill>;
};

} // namespace orbit

#endif /* _INVERTIBLE_ROW_HPP */
