#ifndef _INVERTIBLE_RLBWT_COLUMNS_HPP
#define _INVERTIBLE_RLBWT_COLUMNS_HPP

#include "orbit/internal/move/move_columns.hpp"

// ================================ RLBWT INVERTIBLE MOVE STRUCTURES ================================

namespace orbit::rlbwt {

enum class invertible_rlbwt_columns {
    LENGTH,
    POINTER_FWD,
    POINTER_INV,
    OFFSET,
    FWD_INTERVAL,
    INV_INTERVAL,
    CHARACTER,
    COUNT
};

enum class invertible_rlbwt_columns_idx {
    START,
    POINTER_FWD,
    POINTER_INV,
    OFFSET,
    FWD_INTERVAL,
    INV_INTERVAL,
    CHARACTER,
    COUNT
};

enum class invertible_rlbwt_columns_scan {
    LENGTH,
    POINTER_FWD,
    POINTER_INV,
    FWD_INTERVAL,
    INV_INTERVAL,
    CHARACTER,
    COUNT
};

enum class invertible_rlbwt_columns_idx_scan {
    START,
    POINTER_FWD,
    POINTER_INV,
    FWD_INTERVAL,
    INV_INTERVAL,
    CHARACTER,
    COUNT
};

enum class invertible_rlbwt_columns_spill {
    LENGTH,
    POINTER_FWD,
    FWD_INTERVAL,
    INV_INTERVAL,
    CHARACTER,
    COUNT
};

enum class invertible_rlbwt_columns_idx_spill {
    START,
    POINTER_FWD,
    FWD_INTERVAL,
    INV_INTERVAL,
    CHARACTER,
    COUNT
};

} // namespace orbit::rlbwt

namespace orbit {

template<>
struct move_cols_traits<rlbwt::invertible_rlbwt_columns> {
    static constexpr bool RELATIVE = true;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = true;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr rlbwt::invertible_rlbwt_columns PRIMARY = rlbwt::invertible_rlbwt_columns::LENGTH;

    static constexpr rlbwt::invertible_rlbwt_columns LENGTH = rlbwt::invertible_rlbwt_columns::LENGTH;
    static constexpr rlbwt::invertible_rlbwt_columns POINTER_FWD = rlbwt::invertible_rlbwt_columns::POINTER_FWD;
    static constexpr rlbwt::invertible_rlbwt_columns POINTER_INV = rlbwt::invertible_rlbwt_columns::POINTER_INV;
    static constexpr rlbwt::invertible_rlbwt_columns OFFSET = rlbwt::invertible_rlbwt_columns::OFFSET;
    static constexpr rlbwt::invertible_rlbwt_columns FWD_INTERVAL = rlbwt::invertible_rlbwt_columns::FWD_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns INV_INTERVAL = rlbwt::invertible_rlbwt_columns::INV_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns CHARACTER = rlbwt::invertible_rlbwt_columns::CHARACTER;
    static constexpr size_t NUM_COLS = static_cast<size_t>(rlbwt::invertible_rlbwt_columns::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<rlbwt::invertible_rlbwt_columns_idx> {
    static constexpr bool RELATIVE = false;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = true;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr rlbwt::invertible_rlbwt_columns_idx PRIMARY = rlbwt::invertible_rlbwt_columns_idx::START;

    static constexpr rlbwt::invertible_rlbwt_columns_idx START = rlbwt::invertible_rlbwt_columns_idx::START;
    static constexpr rlbwt::invertible_rlbwt_columns_idx POINTER_FWD = rlbwt::invertible_rlbwt_columns_idx::POINTER_FWD;
    static constexpr rlbwt::invertible_rlbwt_columns_idx POINTER_INV = rlbwt::invertible_rlbwt_columns_idx::POINTER_INV;
    static constexpr rlbwt::invertible_rlbwt_columns_idx OFFSET = rlbwt::invertible_rlbwt_columns_idx::OFFSET;
    static constexpr rlbwt::invertible_rlbwt_columns_idx FWD_INTERVAL = rlbwt::invertible_rlbwt_columns_idx::FWD_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_idx INV_INTERVAL = rlbwt::invertible_rlbwt_columns_idx::INV_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_idx CHARACTER = rlbwt::invertible_rlbwt_columns_idx::CHARACTER;
    static constexpr size_t NUM_COLS = static_cast<size_t>(rlbwt::invertible_rlbwt_columns_idx::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<rlbwt::invertible_rlbwt_columns_scan> {
    static constexpr bool RELATIVE = true;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr rlbwt::invertible_rlbwt_columns_scan PRIMARY = rlbwt::invertible_rlbwt_columns_scan::LENGTH;

    static constexpr rlbwt::invertible_rlbwt_columns_scan LENGTH = rlbwt::invertible_rlbwt_columns_scan::LENGTH;
    static constexpr rlbwt::invertible_rlbwt_columns_scan POINTER_FWD = rlbwt::invertible_rlbwt_columns_scan::POINTER_FWD;
    static constexpr rlbwt::invertible_rlbwt_columns_scan POINTER_INV = rlbwt::invertible_rlbwt_columns_scan::POINTER_INV;
    static constexpr rlbwt::invertible_rlbwt_columns_scan FWD_INTERVAL = rlbwt::invertible_rlbwt_columns_scan::FWD_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_scan INV_INTERVAL = rlbwt::invertible_rlbwt_columns_scan::INV_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_scan CHARACTER = rlbwt::invertible_rlbwt_columns_scan::CHARACTER;
    static constexpr size_t NUM_COLS = static_cast<size_t>(rlbwt::invertible_rlbwt_columns_scan::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<rlbwt::invertible_rlbwt_columns_idx_scan> {
    static constexpr bool RELATIVE = false;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan PRIMARY = rlbwt::invertible_rlbwt_columns_idx_scan::START;

    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan START = rlbwt::invertible_rlbwt_columns_idx_scan::START;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan POINTER_FWD = rlbwt::invertible_rlbwt_columns_idx_scan::POINTER_FWD;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan POINTER_INV = rlbwt::invertible_rlbwt_columns_idx_scan::POINTER_INV;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan FWD_INTERVAL = rlbwt::invertible_rlbwt_columns_idx_scan::FWD_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan INV_INTERVAL = rlbwt::invertible_rlbwt_columns_idx_scan::INV_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_scan CHARACTER = rlbwt::invertible_rlbwt_columns_idx_scan::CHARACTER;
    static constexpr size_t NUM_COLS = static_cast<size_t>(rlbwt::invertible_rlbwt_columns_idx_scan::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<rlbwt::invertible_rlbwt_columns_spill> {
    static constexpr bool RELATIVE = true;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = true;
    static constexpr rlbwt::invertible_rlbwt_columns_spill PRIMARY = rlbwt::invertible_rlbwt_columns_spill::LENGTH;

    static constexpr rlbwt::invertible_rlbwt_columns_spill LENGTH = rlbwt::invertible_rlbwt_columns_spill::LENGTH;
    static constexpr rlbwt::invertible_rlbwt_columns_spill POINTER_FWD = rlbwt::invertible_rlbwt_columns_spill::POINTER_FWD;
    static constexpr rlbwt::invertible_rlbwt_columns_spill FWD_INTERVAL = rlbwt::invertible_rlbwt_columns_spill::FWD_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_spill INV_INTERVAL = rlbwt::invertible_rlbwt_columns_spill::INV_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_spill CHARACTER = rlbwt::invertible_rlbwt_columns_spill::CHARACTER;
    static constexpr size_t NUM_COLS = static_cast<size_t>(rlbwt::invertible_rlbwt_columns_spill::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<rlbwt::invertible_rlbwt_columns_idx_spill> {
    static constexpr bool RELATIVE = false;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = true;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_spill PRIMARY = rlbwt::invertible_rlbwt_columns_idx_spill::START;

    static constexpr rlbwt::invertible_rlbwt_columns_idx_spill START = rlbwt::invertible_rlbwt_columns_idx_spill::START;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_spill POINTER_FWD = rlbwt::invertible_rlbwt_columns_idx_spill::POINTER_FWD;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_spill FWD_INTERVAL = rlbwt::invertible_rlbwt_columns_idx_spill::FWD_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_spill INV_INTERVAL = rlbwt::invertible_rlbwt_columns_idx_spill::INV_INTERVAL;
    static constexpr rlbwt::invertible_rlbwt_columns_idx_spill CHARACTER = rlbwt::invertible_rlbwt_columns_idx_spill::CHARACTER;
    static constexpr size_t NUM_COLS = static_cast<size_t>(rlbwt::invertible_rlbwt_columns_idx_spill::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct column_switcher<rlbwt::invertible_rlbwt_columns> {
    using relative = rlbwt::invertible_rlbwt_columns;
    using absolute = rlbwt::invertible_rlbwt_columns_idx;
    using with_offsets = rlbwt::invertible_rlbwt_columns;
    using without_offsets = rlbwt::invertible_rlbwt_columns_scan;
    using spill = rlbwt::invertible_rlbwt_columns_spill;
};

template<>
struct column_switcher<rlbwt::invertible_rlbwt_columns_idx> {
    using relative = rlbwt::invertible_rlbwt_columns;
    using absolute = rlbwt::invertible_rlbwt_columns_idx;
    using with_offsets = rlbwt::invertible_rlbwt_columns_idx;
    using without_offsets = rlbwt::invertible_rlbwt_columns_idx_scan;
    using spill = rlbwt::invertible_rlbwt_columns_idx_spill;
};

template<>
struct column_switcher<rlbwt::invertible_rlbwt_columns_scan> {
    using relative = rlbwt::invertible_rlbwt_columns_scan;
    using absolute = rlbwt::invertible_rlbwt_columns_idx_scan;
    using with_offsets = rlbwt::invertible_rlbwt_columns;
    using without_offsets = rlbwt::invertible_rlbwt_columns_scan;
    using spill = rlbwt::invertible_rlbwt_columns_spill;
};

template<>
struct column_switcher<rlbwt::invertible_rlbwt_columns_idx_scan> {
    using relative = rlbwt::invertible_rlbwt_columns_scan;
    using absolute = rlbwt::invertible_rlbwt_columns_idx_scan;
    using with_offsets = rlbwt::invertible_rlbwt_columns_idx;
    using without_offsets = rlbwt::invertible_rlbwt_columns_idx_scan;
    using spill = rlbwt::invertible_rlbwt_columns_idx_spill;
};

template<>
struct column_switcher<rlbwt::invertible_rlbwt_columns_spill> {
    using relative = rlbwt::invertible_rlbwt_columns_spill;
    using absolute = rlbwt::invertible_rlbwt_columns_idx_spill;
    using with_offsets = rlbwt::invertible_rlbwt_columns;
    using without_offsets = rlbwt::invertible_rlbwt_columns_scan;
    using spill = rlbwt::invertible_rlbwt_columns_spill;
};

template<>
struct column_switcher<rlbwt::invertible_rlbwt_columns_idx_spill> {
    using relative = rlbwt::invertible_rlbwt_columns_spill;
    using absolute = rlbwt::invertible_rlbwt_columns_idx_spill;
    using with_offsets = rlbwt::invertible_rlbwt_columns_idx;
    using without_offsets = rlbwt::invertible_rlbwt_columns_idx_scan;
    using spill = rlbwt::invertible_rlbwt_columns_idx_spill;
};

} // namespace orbit

#endif /* _INVERTIBLE_RLBWT_COLUMNS_HPP */
