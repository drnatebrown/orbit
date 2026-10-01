#ifndef _INVERTIBLE_COLUMNS_HPP
#define _INVERTIBLE_COLUMNS_HPP

#include "orbit/internal/move/move_columns.hpp"

namespace orbit {

// ================================ INVERTIBLE MOVE STRUCTURES ================================
// Mode A (store offsets) takes the primary names. Scan is the no-OFFSET layout.
// Spill omits POINTER_INV; dual-head pointers live in a side table.

enum class invertible_columns {
    LENGTH, // Length of move interval
    POINTER_FWD, // Pointer to forward move interval
    POINTER_INV, // Pointer to inverse move interval
    OFFSET, // Offset for the direction that is not a head (unused when both flags set)
    FWD_INTERVAL, // Interval was originally a forward move interval
    INV_INTERVAL, // Interval was originally an inverse move interval
    COUNT // Helper to get the number of columns
};

enum class invertible_columns_idx {
    START, // i is the start of the interval
    POINTER_FWD,
    POINTER_INV,
    OFFSET,
    FWD_INTERVAL,
    INV_INTERVAL,
    COUNT
};

enum class invertible_columns_scan {
    LENGTH,
    POINTER_FWD,
    POINTER_INV,
    FWD_INTERVAL,
    INV_INTERVAL,
    COUNT
};

enum class invertible_columns_idx_scan {
    START,
    POINTER_FWD,
    POINTER_INV,
    FWD_INTERVAL,
    INV_INTERVAL,
    COUNT
};

enum class invertible_columns_spill {
    LENGTH,
    POINTER_FWD, // Sole id, or spill-row index when both heads
    FWD_INTERVAL,
    INV_INTERVAL,
    COUNT
};

enum class invertible_columns_idx_spill {
    START,
    POINTER_FWD,
    FWD_INTERVAL,
    INV_INTERVAL,
    COUNT
};

template<>
struct move_cols_traits<invertible_columns> {
    static constexpr bool RELATIVE = true;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = true;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr invertible_columns PRIMARY = invertible_columns::LENGTH;

    static constexpr invertible_columns LENGTH = invertible_columns::LENGTH;
    static constexpr invertible_columns POINTER_FWD = invertible_columns::POINTER_FWD;
    static constexpr invertible_columns POINTER_INV = invertible_columns::POINTER_INV;
    static constexpr invertible_columns OFFSET = invertible_columns::OFFSET;
    static constexpr invertible_columns FWD_INTERVAL = invertible_columns::FWD_INTERVAL;
    static constexpr invertible_columns INV_INTERVAL = invertible_columns::INV_INTERVAL;
    static constexpr size_t NUM_COLS = static_cast<size_t>(invertible_columns::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<invertible_columns_idx> {
    static constexpr bool RELATIVE = false;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = true;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr invertible_columns_idx PRIMARY = invertible_columns_idx::START;

    static constexpr invertible_columns_idx START = invertible_columns_idx::START;
    static constexpr invertible_columns_idx POINTER_FWD = invertible_columns_idx::POINTER_FWD;
    static constexpr invertible_columns_idx POINTER_INV = invertible_columns_idx::POINTER_INV;
    static constexpr invertible_columns_idx OFFSET = invertible_columns_idx::OFFSET;
    static constexpr invertible_columns_idx FWD_INTERVAL = invertible_columns_idx::FWD_INTERVAL;
    static constexpr invertible_columns_idx INV_INTERVAL = invertible_columns_idx::INV_INTERVAL;
    static constexpr size_t NUM_COLS = static_cast<size_t>(invertible_columns_idx::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<invertible_columns_scan> {
    static constexpr bool RELATIVE = true;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr invertible_columns_scan PRIMARY = invertible_columns_scan::LENGTH;

    static constexpr invertible_columns_scan LENGTH = invertible_columns_scan::LENGTH;
    static constexpr invertible_columns_scan POINTER_FWD = invertible_columns_scan::POINTER_FWD;
    static constexpr invertible_columns_scan POINTER_INV = invertible_columns_scan::POINTER_INV;
    static constexpr invertible_columns_scan FWD_INTERVAL = invertible_columns_scan::FWD_INTERVAL;
    static constexpr invertible_columns_scan INV_INTERVAL = invertible_columns_scan::INV_INTERVAL;
    static constexpr size_t NUM_COLS = static_cast<size_t>(invertible_columns_scan::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<invertible_columns_idx_scan> {
    static constexpr bool RELATIVE = false;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = false;
    static constexpr invertible_columns_idx_scan PRIMARY = invertible_columns_idx_scan::START;

    static constexpr invertible_columns_idx_scan START = invertible_columns_idx_scan::START;
    static constexpr invertible_columns_idx_scan POINTER_FWD = invertible_columns_idx_scan::POINTER_FWD;
    static constexpr invertible_columns_idx_scan POINTER_INV = invertible_columns_idx_scan::POINTER_INV;
    static constexpr invertible_columns_idx_scan FWD_INTERVAL = invertible_columns_idx_scan::FWD_INTERVAL;
    static constexpr invertible_columns_idx_scan INV_INTERVAL = invertible_columns_idx_scan::INV_INTERVAL;
    static constexpr size_t NUM_COLS = static_cast<size_t>(invertible_columns_idx_scan::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<invertible_columns_spill> {
    static constexpr bool RELATIVE = true;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = true;
    static constexpr invertible_columns_spill PRIMARY = invertible_columns_spill::LENGTH;

    static constexpr invertible_columns_spill LENGTH = invertible_columns_spill::LENGTH;
    static constexpr invertible_columns_spill POINTER_FWD = invertible_columns_spill::POINTER_FWD;
    static constexpr invertible_columns_spill FWD_INTERVAL = invertible_columns_spill::FWD_INTERVAL;
    static constexpr invertible_columns_spill INV_INTERVAL = invertible_columns_spill::INV_INTERVAL;
    static constexpr size_t NUM_COLS = static_cast<size_t>(invertible_columns_spill::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct move_cols_traits<invertible_columns_idx_spill> {
    static constexpr bool RELATIVE = false;
    static constexpr bool INVERTIBLE = true;
    static constexpr bool STORE_OFFSETS = false;
    static constexpr bool USE_SPILLOVER = true;
    static constexpr invertible_columns_idx_spill PRIMARY = invertible_columns_idx_spill::START;

    static constexpr invertible_columns_idx_spill START = invertible_columns_idx_spill::START;
    static constexpr invertible_columns_idx_spill POINTER_FWD = invertible_columns_idx_spill::POINTER_FWD;
    static constexpr invertible_columns_idx_spill FWD_INTERVAL = invertible_columns_idx_spill::FWD_INTERVAL;
    static constexpr invertible_columns_idx_spill INV_INTERVAL = invertible_columns_idx_spill::INV_INTERVAL;
    static constexpr size_t NUM_COLS = static_cast<size_t>(invertible_columns_idx_spill::COUNT);

    using position = move_position<RELATIVE>::type;
};

template<>
struct column_switcher<invertible_columns> {
    using relative = invertible_columns;
    using absolute = invertible_columns_idx;
    using with_offsets = invertible_columns;
    using without_offsets = invertible_columns_scan;
    using spill = invertible_columns_spill;
};

template<>
struct column_switcher<invertible_columns_idx> {
    using relative = invertible_columns;
    using absolute = invertible_columns_idx;
    using with_offsets = invertible_columns_idx;
    using without_offsets = invertible_columns_idx_scan;
    using spill = invertible_columns_idx_spill;
};

template<>
struct column_switcher<invertible_columns_scan> {
    using relative = invertible_columns_scan;
    using absolute = invertible_columns_idx_scan;
    using with_offsets = invertible_columns;
    using without_offsets = invertible_columns_scan;
    using spill = invertible_columns_spill;
};

template<>
struct column_switcher<invertible_columns_idx_scan> {
    using relative = invertible_columns_scan;
    using absolute = invertible_columns_idx_scan;
    using with_offsets = invertible_columns_idx;
    using without_offsets = invertible_columns_idx_scan;
    using spill = invertible_columns_idx_spill;
};

template<>
struct column_switcher<invertible_columns_spill> {
    using relative = invertible_columns_spill;
    using absolute = invertible_columns_idx_spill;
    using with_offsets = invertible_columns;
    using without_offsets = invertible_columns_scan;
    using spill = invertible_columns_spill;
};

template<>
struct column_switcher<invertible_columns_idx_spill> {
    using relative = invertible_columns_spill;
    using absolute = invertible_columns_idx_spill;
    using with_offsets = invertible_columns_idx;
    using without_offsets = invertible_columns_idx_scan;
    using spill = invertible_columns_idx_spill;
};

// Separated dual-head pointers: row i is {fwd, inv} for a dual head.
enum class spill_pointer_columns { FWD, INV, COUNT };

// Mutually exclusive invertible layouts.
enum class invertible_space_mode { offsets, scan, spill };

constexpr invertible_space_mode DEFAULT_INVERTIBLE_SPACE = invertible_space_mode::offsets;

template<typename columns_t, typename = void>
struct has_store_offsets : std::false_type {};

template<typename columns_t>
struct has_store_offsets<columns_t, std::void_t<decltype(move_cols_traits<columns_t>::STORE_OFFSETS)>>
    : std::true_type {};

template<typename columns_t, typename = void>
struct has_use_spillover : std::false_type {};

template<typename columns_t>
struct has_use_spillover<columns_t, std::void_t<decltype(move_cols_traits<columns_t>::USE_SPILLOVER)>>
    : std::true_type {};

// offsets when the column set stores OFFSET (or is not invertible); spill when USE_SPILLOVER;
// scan otherwise. Spill is never the default for offset/scan column sets.
template<typename columns_t, typename = void>
struct default_invertible_space {
    static constexpr invertible_space_mode value = invertible_space_mode::offsets;
};

template<typename columns_t>
struct default_invertible_space<columns_t,
    std::enable_if_t<has_use_spillover<columns_t>::value && move_cols_traits<columns_t>::USE_SPILLOVER>> {
    static constexpr invertible_space_mode value = invertible_space_mode::spill;
};

template<typename columns_t>
struct default_invertible_space<columns_t,
    std::enable_if_t<has_store_offsets<columns_t>::value && !move_cols_traits<columns_t>::STORE_OFFSETS &&
                     !(has_use_spillover<columns_t>::value && move_cols_traits<columns_t>::USE_SPILLOVER)>> {
    static constexpr invertible_space_mode value = invertible_space_mode::scan;
};

template<typename base_columns, invertible_space_mode mode>
using switch_space_columns = std::conditional_t<
    mode == invertible_space_mode::offsets,
    typename column_switcher<base_columns>::with_offsets,
    std::conditional_t<
        mode == invertible_space_mode::spill,
        typename column_switcher<base_columns>::spill,
        typename column_switcher<base_columns>::without_offsets>>;

} // namespace orbit

#endif /* _INVERTIBLE_COLUMNS_HPP */
