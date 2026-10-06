#ifndef _RLBWT_STRUCTURE_HPP
#define _RLBWT_STRUCTURE_HPP

#include "orbit/common.hpp"
#include "orbit/internal/move/move_structure_impl.hpp"
#include "orbit/internal/rlbwt/specializations/rlbwt_columns.hpp"
#include "orbit/internal/rlbwt/specializations/rlbwt_interval_encoding.hpp"

namespace orbit::rlbwt {

// Shared implementation for both RLBWT column types
template<typename rlbwt_columns_t = rlbwt_columns, template<typename> class table_t = move_vector>
class rlbwt_move_structure : public move_structure<rlbwt_columns_t, table_t> {
    using base = move_structure<rlbwt_columns_t, table_t>;  
public:
    // Sets Columns, ColsTraits, and NumCols
    MOVE_CLASS_TRAITS(typename base::columns)
    using position = typename cols_traits::position;
    using interval_encoding_t = typename base::interval_encoding_t;

    rlbwt_move_structure() = default;

    rlbwt_move_structure(const std::vector<uchar>& head_chars, const std::vector<ulint>& lengths, const std::vector<ulint>& images, const ulint domain, const uchar sigma, const split_params& sp = split_params(), bool store_ef_high_bits = false)
    : rlbwt_move_structure(prepare_rows(head_chars, lengths, images, domain, sigma, sp, store_ef_high_bits)) {}

    template<typename rlbwt_interval_encoding_t>
    rlbwt_move_structure(const rlbwt_interval_encoding_t& enc)
    : base(find_structure(enc), enc.domain(), enc.runs()) {}

    rlbwt_move_structure(packed_vector<columns> &&structure, const size_t domain, ulint runs)
        : base(std::move(structure), domain, runs) {}

    rlbwt_move_structure(packed_vector<columns> &&structure, const size_t domain, ulint runs, pointer_spillover spill)
        : base(std::move(structure), domain, runs, std::move(spill)) {}

    rlbwt_move_structure(packed_vector<columns> &&structure, const size_t domain, ulint runs, pointer_spillover spill, ef_index ef)
        : base(std::move(structure), domain, runs, std::move(spill), std::move(ef)) {}

    rlbwt_move_structure(move_payload<columns> built, const size_t domain, ulint runs)
        : base(std::move(built), domain, runs) {}

    static move_payload<columns> find_structure(const std::vector<ulint>& lengths, const std::vector<ulint>& images, const ulint domain, const uchar char_width, const split_params& sp = split_params()) = delete;

    // When the permutation is already computed but with no encoding
    static move_payload<columns> find_structure(const std::vector<uchar>& head_chars, const std::vector<ulint>& lengths, const std::vector<ulint>& images, const uchar sigma, const split_params& sp = split_params(), ef_index* ef = nullptr) {
        assert(head_chars.size() == lengths.size());

        interval_encoding_t enc(lengths, images, sp);
        assert(enc.runs() == lengths.size());
        require_ef_supported(enc, ef);

        move_payload<columns> built;
        built.rows = packed_vector<columns>(enc.intervals(), get_move_widths(enc.domain(), enc.intervals(), enc.max_length(), sigma));
        base::populate_structure(built.rows, enc, &built.spill, ef);
        set_characters(built.rows, head_chars, lengths, enc);
        return built;
    }

    // When the permutation is already computed with a regular encoding
    template<typename rlbwt_interval_encoding_t>
    static move_payload<columns> find_structure(const std::vector<uchar>& rlbwt_chars, const rlbwt_interval_encoding_t& enc, const uchar sigma, ef_index* ef = nullptr) {
        static_assert(!cols_traits::INVERTIBLE ||
                          rlbwt_interval_encoding_t::invertible_tag,
                      "Invertible RLBWT structure requires an invertible "
                      "interval encoding");
        assert(rlbwt_chars.size() == enc.intervals());
        require_ef_supported(enc, ef);

        move_payload<columns> built;
        built.rows = packed_vector<columns>(enc.intervals(), get_move_widths(enc.domain(), enc.intervals(), enc.max_length(), sigma));
        base::populate_structure(built.rows, enc, &built.spill, ef);
        set_characters(built.rows, rlbwt_chars);
        return built;
    }

    // When the permutation is already computed with a rlbwt encoding
    template<typename rlbwt_interval_encoding_t>
    static move_payload<columns> find_structure(const rlbwt_interval_encoding_t& enc, ef_index* ef = nullptr) {
        static_assert(!cols_traits::INVERTIBLE ||
                          rlbwt_interval_encoding_t::invertible_tag,
                      "Invertible RLBWT structure requires an invertible "
                      "interval encoding");
        assert(enc.get_heads().size() == enc.intervals());
        require_ef_supported(enc, ef);

        move_payload<columns> built;
        built.rows = packed_vector<columns>(enc.intervals(), get_move_widths(enc.domain(), enc.intervals(), enc.max_length(), enc.sigma()));
        base::populate_structure(built.rows, enc, &built.spill, ef);
        set_characters(built.rows, enc.get_heads());
        return built;
    }

    uchar get_character(size_t i) const {
        return base::table.template get<cols_traits::CHARACTER>(i);
    }
    uchar get_character(position pos) const {
        return get_character(pos.interval);
    }

private:
    struct rows_and_ef {
        move_payload<columns> payload;
        ef_index ef;
        size_t domain = 0;
        ulint runs = 0;
    };

    rlbwt_move_structure(rows_and_ef built)
        : base(std::move(built.payload.rows), built.domain, built.runs, std::move(built.payload.spill), std::move(built.ef)) {}

    static rows_and_ef prepare_rows(const std::vector<uchar>& head_chars, const std::vector<ulint>& lengths, const std::vector<ulint>& images, const ulint domain, const uchar sigma, const split_params& sp, bool store_ef_high_bits) {
        rows_and_ef built;
        built.domain = domain;
        built.runs = lengths.size();
        built.payload = find_structure(head_chars, lengths, images, sigma, sp, store_ef_high_bits ? &built.ef : nullptr);
        return built;
    }

    template<typename encoding_t>
    static void require_ef_supported(const encoding_t& enc, ef_index* ef) {
        if (ef == nullptr) return;
        assert(cols_traits::RELATIVE && "store_ef_high_bits requires relative positions");
        assert(enc.get_split_params().length_capping.has_value() &&
               "store_ef_high_bits requires length capping");
    }

    static std::array<uchar, num_cols> get_move_widths(const ulint domain, const ulint runs, const ulint max_length, const uchar sigma) {
        auto widths = base::get_move_widths(domain, runs, max_length);
        widths[static_cast<size_t>(cols_traits::CHARACTER)] = bit_width(sigma - 1);
        return widths;
    }

    // Copy characters assuming no splitting
    template<typename collection_t>
    static void set_characters(packed_vector<columns>& structure, const collection_t& rlbwt_chars) {
        assert(rlbwt_chars.size() == structure.size());

        for (size_t i = 0; i < rlbwt_chars.size(); ++i) {
            structure.template set<to_cols(cols_traits::CHARACTER)>(i, static_cast<ulint>(rlbwt_chars[i]));
        }
    }

    // Copy characters over while accounting for splitting.
    // Relative PRIMARY may hold Elias-Fano lows, so interval lengths come from the encoding.
    template<typename encoding_t>
    static void set_characters(packed_vector<columns>& structure, const std::vector<uchar>& rlbwt_chars, const std::vector<ulint>& lengths, const encoding_t& enc) {
        assert(rlbwt_chars.size() == lengths.size());
        
        auto get_structure_length = [&structure, &enc](size_t row) {
            if constexpr (!cols_traits::RELATIVE) {
                const ulint domain = enc.domain();
                return (row == structure.size() - 1) 
                    ? domain - structure.template get<cols_traits::PRIMARY>(row)
                    : structure.template get<cols_traits::PRIMARY>(row + 1) - structure.template get<cols_traits::PRIMARY>(row);
            } else {
                return enc.get_length(row);
            }
        };

        size_t rlbwt_run_start = 0;
        size_t structure_run_start = 0;
        size_t structure_run_idx = 0;
        for (size_t i = 0; i < rlbwt_chars.size(); ++i) {
            while (structure_run_idx < structure.size() && structure_run_start < rlbwt_run_start + lengths[i]) {
                structure.template set<to_cols(cols_traits::CHARACTER)>(structure_run_idx, rlbwt_chars[i]);
                structure_run_start += get_structure_length(structure_run_idx++);
            }
            rlbwt_run_start += lengths[i];
        }
    }
};

} // namespace orbit::rlbwt

#endif /* end of include guard: _RLBWT_STRUCTURE_HPP */