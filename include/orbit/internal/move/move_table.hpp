#ifndef _MOVE_TABLE_HH
#define _MOVE_TABLE_HH

#include "orbit/common.hpp"
#include "orbit/internal/move/move_row.hpp"
#include "orbit/internal/move/move_columns.hpp"
#include "orbit/internal/rlbwt/specializations/rlbwt_row.hpp"
#include "orbit/internal/ds/packed_vector.hpp"

#include <cassert>
#include <numeric>

namespace orbit {

template<typename derived, typename columns_t>
struct move_table_interface {
    // Sets NumCols, Columns, and ColsTraits
    MOVE_CLASS_TRAITS(columns_t)

    template <typename C = columns>
    void set_primary(size_t i, ulint start, ulint length) {
        if constexpr (cols_traits_for<C>::RELATIVE) {
            static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::PRIMARY)>(i, length);
        } else {
            static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::PRIMARY)>(i, start);
        }
    }

    template <typename C = columns>
    std::enable_if_t<cols_traits_for<C>::RELATIVE, void>
    set_length(size_t i, ulint l) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::LENGTH)>(i, l);
    }
    
    template <typename C = columns>
    std::enable_if_t<!cols_traits_for<C>::RELATIVE, void>
    set_start(size_t i, ulint s) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::START)>(i, s);
    }
    
    void set_pointer(size_t i, ulint p) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits::POINTER)>(i, p);
    }
    
    void set_offset(size_t i, ulint o) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits::OFFSET)>(i, o);
    }
    
    ulint get_primary(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits::PRIMARY)>(i);
    }

    template <typename C = columns>
    std::enable_if_t<cols_traits_for<C>::RELATIVE, ulint>
    get_length(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits_for<C>::LENGTH)>(i);
    }
    
    template <typename C = columns>
    std::enable_if_t<!cols_traits_for<C>::RELATIVE, ulint>
    get_start(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits_for<C>::START)>(i);
    }   
    
    ulint get_pointer(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits::POINTER)>(i);
    }
    
    ulint get_offset(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits::OFFSET)>(i);
    }
};

// ============================================= INVERTIBLE =============================================

template<typename derived, typename columns_t>
struct invertible_table_interface {
    // Sets NumCols, Columns, and ColsTraits
    MOVE_CLASS_TRAITS(columns_t)

    template <typename C = columns>
    void set_primary(size_t i, ulint start, ulint length) {
        if constexpr (cols_traits_for<C>::RELATIVE) {
            static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::PRIMARY)>(i, length);
        } else {
            static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::PRIMARY)>(i, start);
        }
    }

    template <typename C = columns>
    std::enable_if_t<cols_traits_for<C>::RELATIVE, void>
    set_length(size_t i, ulint l) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::LENGTH)>(i, l);
    }
    
    template <typename C = columns>
    std::enable_if_t<!cols_traits_for<C>::RELATIVE, void>
    set_start(size_t i, ulint s) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::START)>(i, s);
    }
    
    void set_pointer_fwd(size_t i, ulint p) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits::POINTER_FWD)>(i, p);
    }

    template <typename C = columns>
    std::enable_if_t<!cols_traits_for<C>::USE_SPILLOVER, void>
    set_pointer_inv(size_t i, ulint p) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::POINTER_INV)>(i, p);
    }

    void set_fwd_interval(size_t i, bool is_fwd) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits::FWD_INTERVAL)>(i, static_cast<ulint>(is_fwd));
    }

    void set_inv_interval(size_t i, bool is_inv) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits::INV_INTERVAL)>(i, static_cast<ulint>(is_inv));
    }

    template <typename C = columns>
    std::enable_if_t<cols_traits_for<C>::STORE_OFFSETS, void>
    set_offset(size_t i, ulint o) {
        static_cast<derived*>(this)->template set<to_cols(cols_traits_for<C>::OFFSET)>(i, o);
    }
    
    ulint get_primary(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits::PRIMARY)>(i);
    }

    template <typename C = columns>
    std::enable_if_t<cols_traits_for<C>::RELATIVE, ulint>
    get_length(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits_for<C>::LENGTH)>(i);
    }
    
    template <typename C = columns>
    std::enable_if_t<!cols_traits_for<C>::RELATIVE, ulint>
    get_start(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits_for<C>::START)>(i);
    }   
    
    ulint get_pointer_fwd(size_t i) const {
        const auto* self = static_cast<const derived*>(this);
        if constexpr (cols_traits::USE_SPILLOVER) {
            if (get_fwd_interval(i) && get_inv_interval(i)) {
                return self->spillover().template get<spill_pointer_columns::FWD>(
                    self->template get<to_cols(cols_traits::POINTER_FWD)>(i));
            }
            return self->template get<to_cols(cols_traits::POINTER_FWD)>(i);
        } else {
            return self->template get<to_cols(cols_traits::POINTER_FWD)>(i);
        }
    }
    
    ulint get_pointer_inv(size_t i) const {
        const auto* self = static_cast<const derived*>(this);
        if constexpr (cols_traits::USE_SPILLOVER) {
            if (get_fwd_interval(i) && get_inv_interval(i)) {
                return self->spillover().template get<spill_pointer_columns::INV>(
                    self->template get<to_cols(cols_traits::POINTER_FWD)>(i));
            }
            // Sole inverse id lives in the forward pointer cell.
            return self->template get<to_cols(cols_traits::POINTER_FWD)>(i);
        } else {
            return self->template get<to_cols(cols_traits::POINTER_INV)>(i);
        }
    }

    bool get_fwd_interval(size_t i) const {
        return static_cast<bool>(static_cast<const derived*>(this)->template get<to_cols(cols_traits::FWD_INTERVAL)>(i));
    }

    bool get_inv_interval(size_t i) const {
        return static_cast<bool>(static_cast<const derived*>(this)->template get<to_cols(cols_traits::INV_INTERVAL)>(i));
    }

    template <typename C = columns>
    std::enable_if_t<cols_traits_for<C>::STORE_OFFSETS, ulint>
    get_offset(size_t i) const {
        return static_cast<const derived*>(this)->template get<to_cols(cols_traits_for<C>::OFFSET)>(i);
    }
};

struct no_spillover {};

// Dual-head side table: packed {FWD, INV} rows, same bit width as interval ids.
using pointer_spillover = packed_vector<spill_pointer_columns>;

inline pointer_spillover empty_pointer_spillover() {
    return pointer_spillover(0, {uchar(1), uchar(1)});
}

inline pointer_spillover make_pointer_spillover(size_t rows, uchar width) {
    if (width == 0) width = 1;
    return pointer_spillover(rows, {width, width});
}

template<typename columns_t>
struct move_payload {
    packed_vector<columns_t> rows;
    pointer_spillover spill = empty_pointer_spillover();
};

// ============================================= MOVE TABLE =============================================

template<typename columns_t = move_columns,
         template<typename, typename> class interface = move_table_interface,
         invertible_space_mode mode = default_invertible_space<columns_t>::value,
         typename row_t = typename table_row_for<columns_t>::type>
struct move_table_impl : public interface<move_table_impl<columns_t, interface, mode, row_t>, columns_t> {
    using row = row_t;
    using row_traits = typename row::row_traits;
    static constexpr invertible_space_mode space_mode = mode;
    // Sets num_cols, columns, and cols_traits
    MOVE_CLASS_TRAITS(columns_t)
    
    std::vector<row> table;
    [[no_unique_address]] std::conditional_t<cols_traits::USE_SPILLOVER, pointer_spillover, no_spillover> spill_;

    move_table_impl() {
        if constexpr (cols_traits::USE_SPILLOVER) spill_ = empty_pointer_spillover();
    }
    move_table_impl(packed_vector<columns> &&vec, pointer_spillover spill = empty_pointer_spillover()) {
        row::assert_widths(vec.get_widths());
        
        table = std::vector<row>(vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            set_row(i, vec.get_row(i));
        }
        if constexpr (cols_traits::USE_SPILLOVER) {
            spill_ = std::move(spill);
        }
    }

    template <bool spill = cols_traits::USE_SPILLOVER, typename = std::enable_if_t<spill>>
    const pointer_spillover& spillover() const {
        return spill_;
    }

    template <bool spill = cols_traits::USE_SPILLOVER, typename = std::enable_if_t<spill>>
    size_t spillover_rows() const {
        return spill_.size();
    }

    const std::array<uchar, num_cols>& get_widths() const {
        return row::get_widths();
    }

    size_t size() const { return table.size(); }

    template <columns col>
    void set(size_t i, ulint val) {
        table[i].template set<col>(val);
    }

    template <columns col>
    ulint get(size_t i) const {
        return table[i].template get<col>();
    }

    void set_row(size_t i, const std::array<ulint, num_cols>& values) {
        table[i].set(values);
    }

    std::array<ulint, num_cols> get_row(size_t i) const {
        return table[i].get();
    }

    size_t serialize(std::ostream &out) {
        size_t written_bytes = 0;

        size_t tbl_size = table.size();
        out.write((char *)&tbl_size, sizeof(tbl_size));
        written_bytes += sizeof(tbl_size);

        char* data = reinterpret_cast<char*>(table.data());
        size_t size = tbl_size * sizeof(row);
        out.write(data, size);
        written_bytes += size;

        if constexpr (cols_traits::USE_SPILLOVER) {
            written_bytes += spill_.serialize(out);
        }

        return written_bytes;
    }

    void load(std::istream &in)
    {
        size_t size;
        in.read((char *)&size, sizeof(size));

        table = std::vector<row>(size);
        char* data = reinterpret_cast<char*>(table.data());
        size_t bytes = size * sizeof(row);
        in.read(data, bytes);

        if constexpr (cols_traits::USE_SPILLOVER) {
            spill_.load(in);
        }
    }

    // Widths don't help, the struct is already defined. Spill rows add two pointer columns.
    static size_t bits_needed(size_t num_rows, std::array<uchar, num_cols> widths, size_t spill_rows = 0) {
        size_t bits = bytes_to_bits(sizeof(row)) * num_rows;
        if constexpr (cols_traits::USE_SPILLOVER) {
            const size_t pw = widths[static_cast<size_t>(cols_traits::POINTER_FWD)];
            bits += pw * 2 * spill_rows;
        }
        return bits;
    }
};

template <typename columns_t = move_columns,
          template<typename, typename> class interface = move_table_interface,
          invertible_space_mode mode = default_invertible_space<columns_t>::value>
struct move_vector_impl : public interface<move_vector_impl<columns_t, interface, mode>, columns_t> {
    static constexpr invertible_space_mode space_mode = mode;
    // Sets num_cols, columns, and cols_traits
    MOVE_CLASS_TRAITS(columns_t)
    
    packed_vector<columns> vec;
    [[no_unique_address]] std::conditional_t<cols_traits::USE_SPILLOVER, pointer_spillover, no_spillover> spill_;

    move_vector_impl() {
        if constexpr (cols_traits::USE_SPILLOVER) spill_ = empty_pointer_spillover();
    }
    move_vector_impl(packed_vector<columns> &&vec, pointer_spillover spill = empty_pointer_spillover()) : vec(std::move(vec)) {
        if constexpr (cols_traits::USE_SPILLOVER) {
            spill_ = std::move(spill);
        }
    }

    template <bool spill = cols_traits::USE_SPILLOVER, typename = std::enable_if_t<spill>>
    const pointer_spillover& spillover() const {
        return spill_;
    }

    template <bool spill = cols_traits::USE_SPILLOVER, typename = std::enable_if_t<spill>>
    size_t spillover_rows() const {
        return spill_.size();
    }

    size_t size() const { return vec.size(); }

    template <columns col>
    void set(size_t i, ulint val) {
        vec.template set<col>(i, val);
    }

    template <columns col>
    ulint get(size_t i) const {
        return vec.template get<col>(i);
    }

    void set_row(size_t i, std::array<ulint, num_cols> values) {
        vec.set_row(i, values);
    }

    std::array<ulint, num_cols> get_row(size_t i) const {
        return vec.get_row(i);
    }

    const std::array<uchar, num_cols>& get_widths() const {
        return vec.get_widths();
    }

    size_t serialize(std::ostream &out) {
        size_t written = vec.serialize(out);
        if constexpr (cols_traits::USE_SPILLOVER) {
            written += spill_.serialize(out);
        }
        return written;
    }

    void load(std::istream &in)
    {
        vec.load(in);
        if constexpr (cols_traits::USE_SPILLOVER) {
            spill_.load(in);
        }
    }

    // Easy, just sum the widths. Spill rows add two pointer columns.
    static size_t bits_needed(size_t num_rows, std::array<uchar, num_cols> widths, size_t spill_rows = 0) {
        size_t total_width = std::accumulate(widths.begin(), widths.end(), 0, [](size_t sum, uchar width) {
            return sum + width;
        });
        size_t bits = total_width * num_rows;
        if constexpr (cols_traits::USE_SPILLOVER) {
            const size_t pw = widths[static_cast<size_t>(cols_traits::POINTER_FWD)];
            bits += pw * 2 * spill_rows;
        }
        return bits;
    }
};

template<typename columns_t, invertible_space_mode mode>
using move_vector_for = std::conditional_t<resolve_cols_traits<columns_t>::type::INVERTIBLE,
    move_vector_impl<columns_t, invertible_table_interface, mode>,
    move_vector_impl<columns_t, move_table_interface, mode>>;

template<typename columns_t, invertible_space_mode mode>
using move_table_for = std::conditional_t<resolve_cols_traits<columns_t>::type::INVERTIBLE,
    move_table_impl<columns_t, invertible_table_interface, mode>,
    move_table_impl<columns_t, move_table_interface, mode>>;

template<typename columns_t>
using move_vector = move_vector_for<columns_t, default_invertible_space<columns_t>::value>;

template<typename columns_t>
using move_table = move_table_for<columns_t, default_invertible_space<columns_t>::value>;

} // namespace orbit

#endif // _MOVE_TABLE_HH