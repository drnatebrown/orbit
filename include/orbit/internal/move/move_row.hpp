#ifndef _MOVE_ROW_HPP
#define _MOVE_ROW_HPP

#include "orbit/common.hpp"
#include "orbit/internal/move/move_columns.hpp"
#include <stdexcept>
#include <array>
#include <cassert>

namespace orbit {

template <typename bits_t>
struct move_row_traits;

// ============================================= DEFAULT =============================================

template <typename columns_t, size_t PRIMARY_BITS, size_t POINTER_BITS, size_t OFFSET_BITS>
struct move_row_bits {};

template<typename columns_t>
struct table_row_for {};

template <typename columns_t, size_t P, size_t PTR, size_t OFF>
struct move_row_traits<move_row_bits<columns_t, P, PTR, OFF>> {
    static constexpr size_t PRIMARY_BITS = P;
    static constexpr size_t POINTER_BITS = PTR;
    static constexpr size_t OFFSET_BITS = OFF;
};

template <typename columns_t = move_columns>
struct move_row {
    // Sets NumCols, Columns, and cols_traits
    MOVE_CLASS_TRAITS(columns_t)
    using row_traits = move_row_traits<columns>;

    ulint primary : row_traits::PRIMARY_BITS;
    ulint pointer : row_traits::POINTER_BITS;
    ulint offset : row_traits::OFFSET_BITS;

    move_row() = default;
    move_row(const std::array<ulint, num_cols>& values) {
        set(values);
    }

    template <columns col>
    void set(ulint val) {
        if constexpr (col == cols_traits::PRIMARY) primary = val;
        else if constexpr (col == cols_traits::POINTER) pointer = val;
        else if constexpr (col == cols_traits::OFFSET) offset = val;
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
        else if constexpr (col == cols_traits::POINTER) return pointer;
        else if constexpr (col == cols_traits::OFFSET)   return offset;
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
            static_cast<uchar>(row_traits::POINTER_BITS),
            static_cast<uchar>(row_traits::OFFSET_BITS),
        };
        return widths;
    }

    static void assert_widths(const std::array<uchar, num_cols>& widths) {
        assert(widths[static_cast<size_t>(cols_traits::PRIMARY)] <= row_traits::PRIMARY_BITS);
        assert(widths[static_cast<size_t>(cols_traits::POINTER)] <= row_traits::POINTER_BITS);
        assert(widths[static_cast<size_t>(cols_traits::OFFSET)] <= row_traits::OFFSET_BITS);
    }

} __attribute__((packed));

// Column Sizes for move_columns
using move_cols_4 = move_row_bits<move_columns, 8, 16, 8>;     // (4 bytes)
using move_cols_5 = move_row_bits<move_columns, 8, 24, 8>;     // (5 bytes)
using move_cols_6 = move_row_bits<move_columns, 12, 24, 12>; // (6 bytes)
using move_cols_7 = move_row_bits<move_columns, 12, 32, 12>; // (7 bytes)
using move_cols_8 = move_row_bits<move_columns, 16, 32, 16>; // (8 bytes)
using move_cols_9 = move_row_bits<move_columns, 16, 40, 16>; // (9 bytes)
using move_cols_10 = move_row_bits<move_columns, 20, 40, 20>; // (10 bytes)
using move_cols_11 = move_row_bits<move_columns, 20, 48, 20>; // (11 bytes)
using move_cols_12 = move_row_bits<move_columns, 24, 48, 24>; // (12 bytes)
using move_cols_default = move_cols_9;

// Column Sizes for move_columns_idx
using move_cols_idx_5 = move_row_bits<move_columns_idx, 17, 15, 8>; // (5 bytes)
using move_cols_idx_6 = move_row_bits<move_columns_idx, 21, 19, 8>; // (6 bytes)
using move_cols_idx_7 = move_row_bits<move_columns_idx, 27, 25, 8>; // (7 bytes)
using move_cols_idx_8 = move_row_bits<move_columns_idx, 29, 27, 12>; // (8 bytes)
using move_cols_idx_9 = move_row_bits<move_columns_idx, 33, 31, 12>; // (9 bytes)
using move_cols_idx_10 = move_row_bits<move_columns_idx, 35, 33, 12>; // (10 bytes)
using move_cols_idx_11 = move_row_bits<move_columns_idx, 37, 35, 16>; // (11 bytes)
using move_cols_idx_12 = move_row_bits<move_columns_idx, 41, 39, 16>; // (12 bytes)
using move_cols_idx_13 = move_row_bits<move_columns_idx, 45, 43, 16>; // (13 bytes)
using move_cols_idx_14 = move_row_bits<move_columns_idx, 47, 45, 20>; // (14 bytes)
using move_cols_idx_15 = move_row_bits<move_columns_idx, 51, 49, 20>; // (15 bytes)
using move_cols_idx_default = move_cols_idx_12;

template <>
struct move_row_traits<move_columns> : move_row_traits<move_cols_default> {};

template <>
struct move_row_traits<move_columns_idx> : move_row_traits<move_cols_idx_default> {};

template<>
struct table_row_for<move_columns> {
    using type = move_row<move_columns>;
};

template<>
struct table_row_for<move_columns_idx> {
    using type = move_row<move_columns_idx>;
};

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

template <typename columns_t, bool StoreOffsets = move_cols_traits<columns_t>::STORE_OFFSETS>
struct invertible_row;

template <typename columns_t>
struct invertible_row<columns_t, true> {
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
struct invertible_row<columns_t, false> {
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

// Mode A (offset): 16+39+39+16+1+1 = 112 bits (14 bytes)
using invertible_cols_14 = invertible_offset_row_bits<invertible_columns, 16, 39, 16>;
using invertible_cols_default = invertible_cols_14;

// Scan: 16+39+39+1+1 = 96 bits (12 bytes)
using invertible_cols_scan_12 = invertible_row_bits<invertible_columns_scan, 16, 39>;
using invertible_cols_scan_default = invertible_cols_scan_12;

// Mode A absolute: 42+38+38+16+1+1 = 136 bits (17 bytes)
using invertible_cols_idx_17 = invertible_offset_row_bits<invertible_columns_idx, 42, 38, 16>;
using invertible_cols_idx_default = invertible_cols_idx_17;

// Scan absolute: 42+38+38+1+1 = 120 bits (15 bytes)
using invertible_cols_idx_scan_15 = invertible_row_bits<invertible_columns_idx_scan, 42, 38>;
using invertible_cols_idx_scan_default = invertible_cols_idx_scan_15;

template <>
struct move_row_traits<invertible_columns> : move_row_traits<invertible_cols_default> {};

template <>
struct move_row_traits<invertible_columns_idx> : move_row_traits<invertible_cols_idx_default> {};

template <>
struct move_row_traits<invertible_columns_scan> : move_row_traits<invertible_cols_scan_default> {};

template <>
struct move_row_traits<invertible_columns_idx_scan> : move_row_traits<invertible_cols_idx_scan_default> {};

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

} // namespace orbit

#endif /* end of include guard: _MOVE_ROW_HPP */