// Unit tests for MoveStructure build-time invariants.
// These are simple assert-based tests, no external framework.

#include "orbit/internal/move/move_structure_impl.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using std::size_t;
using std::vector;

using namespace orbit;

static vector<ulint> compute_starts(const vector<ulint>& lengths) {
    vector<ulint> starts(lengths.size() + 1, 0);
    for (size_t i = 0; i < lengths.size(); ++i) {
        starts[i + 1] = starts[i] + lengths[i];
    }
    return starts;
}

static size_t interval_of(const vector<ulint>& starts, ulint idx) {
    // returns i such that starts[i] <= idx < starts[i+1]
    for (size_t i = 0; i + 1 < starts.size(); ++i) {
        if (starts[i] <= idx && idx < starts[i + 1]) return i;
    }
    assert(false && "idx not in any interval");
    return 0;
}

template <typename ColumnsT>
static void assert_pointer_offset_correct(
    const move_structure_impl<ColumnsT>& ms,
    const vector<ulint>& lengths,
    const vector<ulint>& image
) {
    const auto starts = compute_starts(lengths);
    assert(starts.back() == ms.domain());
    assert(ms.runs() == lengths.size());

    for (size_t j = 0; j < lengths.size(); ++j) {
        const ulint mapped_start = image[j];
        const size_t k = interval_of(starts, mapped_start);
        const ulint expected_offset = mapped_start - starts[k];

        assert(ms.get_pointer(j) == k);
        assert(ms.get_offset(j) == expected_offset);
    }
}

static void test_move_structure_relative_build_invariants() {
    // A small valid run-decomposition of a permutation of [0..9]:
    // π = [4,5,6, 0,1, 9, 2,3, 7,8]
    const vector<ulint> lengths = {3, 2, 1, 2, 2};
    const vector<ulint> perm = {4, 0, 9, 2, 7};
    const ulint domain = 10;

    move_structure_impl<move_columns> ms(lengths, perm, NO_SPLITTING);
    assert(ms.domain() == domain);
    assert(ms.runs() == lengths.size());

    for (size_t i = 0; i < lengths.size(); ++i) {
        assert(ms.get_length(i) == lengths[i]);
    }

    assert_pointer_offset_correct(ms, lengths, perm);
}

static void test_move_structure_absolute_build_invariants() {
    // Same run-decomposition as above, but using absolute columns.
    const vector<ulint> lengths = {3, 2, 1, 2, 2};
    const vector<ulint> perm = {4, 0, 9, 2, 7};
    const ulint domain = 10;

    move_structure_impl<move_columns_idx> ms(lengths, perm, NO_SPLITTING);
    assert(ms.domain() == domain);
    assert(ms.runs() == lengths.size());

    const auto starts = compute_starts(lengths);
    for (size_t i = 0; i < lengths.size(); ++i) {
        assert(ms.get_start(i) == starts[i]);
        assert(ms.get_length(i) == lengths[i]);
    }
    assert(ms.get_start(lengths.size()) == domain);

    assert_pointer_offset_correct(ms, lengths, perm);
}

static void test_move_structure_serialize_roundtrip() {
    const vector<ulint> lengths = {3, 2, 1, 2, 2};
    const vector<ulint> perm = {4, 0, 9, 2, 7};
    const ulint domain = 10;

    move_structure_impl<move_columns_idx> ms(lengths, perm, NO_SPLITTING);

    std::stringstream ss;
    const size_t bytes = ms.serialize(ss);
    assert(bytes > 0);

    move_structure_impl<move_columns_idx> loaded;
    loaded.load(ss);

    assert(loaded.domain() == ms.domain());
    assert(loaded.runs() == ms.runs());

    for (size_t i = 0; i < lengths.size(); ++i) {
        assert(loaded.get_start(i) == ms.get_start(i));
        assert(loaded.get_pointer(i) == ms.get_pointer(i));
        assert(loaded.get_offset(i) == ms.get_offset(i));
    }
}

static void test_move_structure_widths_relative_and_absolute_with_and_without_splitting() {
    // Base example from earlier tests.
    const vector<ulint> lengths = {3, 2, 1, 2, 2};
    const vector<ulint> perm = {4, 0, 9, 2, 7};
    const ulint domain = 10;

    // Relative, no splitting.
    move_structure_impl<move_columns> ms_rel(lengths, perm, NO_SPLITTING);
    auto widths_rel = ms_rel.get_widths();

    // PRIMARY and OFFSET widths should match for relative representation.
    uchar w_primary_rel = widths_rel[static_cast<size_t>(move_cols_traits<move_columns>::PRIMARY)];
    uchar w_pointer_rel = widths_rel[static_cast<size_t>(move_cols_traits<move_columns>::POINTER)];
    uchar w_offset_rel  = widths_rel[static_cast<size_t>(move_cols_traits<move_columns>::OFFSET)];
    assert(w_primary_rel == w_offset_rel);
    // Pointer width must be enough to index all runs.
    assert(w_pointer_rel >= bit_width(ms_rel.runs()));

    // Relative, with length-capping splitting.
    split_params split = ONLY_LENGTH_CAPPING;
    move_structure_impl<move_columns> ms_rel_split(lengths, perm, split);
    auto widths_rel_split = ms_rel_split.get_widths();

    uchar w_primary_rel_split =
        widths_rel_split[static_cast<size_t>(move_cols_traits<move_columns>::PRIMARY)];
    uchar w_pointer_rel_split =
        widths_rel_split[static_cast<size_t>(move_cols_traits<move_columns>::POINTER)];
    uchar w_offset_rel_split =
        widths_rel_split[static_cast<size_t>(move_cols_traits<move_columns>::OFFSET)];

    // Still must have PRIMARY == OFFSET after splitting.
    assert(w_primary_rel_split == w_offset_rel_split);
    // Pointer width must match the (possibly increased) number of runs.
    assert(w_pointer_rel_split >= bit_width(ms_rel_split.runs()));

    // Absolute, no splitting.
    move_structure_impl<move_columns_idx> ms_abs(lengths, perm, NO_SPLITTING);
    auto widths_abs = ms_abs.get_widths();

    uchar w_primary_abs =
        widths_abs[static_cast<size_t>(move_cols_traits<move_columns_idx>::PRIMARY)];
    uchar w_pointer_abs =
        widths_abs[static_cast<size_t>(move_cols_traits<move_columns_idx>::POINTER)];
    uchar w_offset_abs =
        widths_abs[static_cast<size_t>(move_cols_traits<move_columns_idx>::OFFSET)];

    // PRIMARY width should be enough for domain indices.
    assert(w_primary_abs >= bit_width(domain));
    // Pointer width enough for runs, offset width reused for relative length.
    assert(w_pointer_abs >= bit_width(ms_abs.runs()));
    assert(w_offset_abs >= bit_width(ms_abs.get_length(0)));
}

template <typename Fn>
static void expect_abort(Fn&& fn) {
    const pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        fn();
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    assert(WIFSIGNALED(status));
    assert(WTERMSIG(status) == SIGABRT);
}

static ulint prefix_index(const move_structure_impl<move_columns>& ms, move_structure_impl<move_columns>::position pos) {
    ulint idx = 0;
    for (size_t i = 0; i < pos.interval; ++i) idx += ms.get_length(i);
    return idx + pos.offset;
}

static void test_ef_get_idx_matches_prefix_and_move() {
    const vector<ulint> lengths = {40, 10, 50};
    const vector<ulint> images = {0, 40, 90};
    const split_params cap(0.25, std::nullopt);

    move_structure_impl<move_columns> plain(lengths, images, cap, false);
    move_structure_impl<move_columns> ef(lengths, images, cap, true);
    assert(ef.stores_ef_high_bits());
    assert(!plain.stores_ef_high_bits());
    assert(ef.intervals() == plain.intervals());

    for (size_t i = 0; i < ef.intervals(); ++i) {
        assert(ef.get_length(i) == plain.get_length(i));
    }

    auto pos_plain = plain.first();
    auto pos_ef = ef.first();
    ulint idx = 0;
    for (size_t interval = 0; interval < ef.intervals(); ++interval) {
        const ulint len = ef.get_length(interval);
        for (ulint offset = 0; offset < len; ++offset) {
            pos_plain = {interval, offset};
            pos_ef = {interval, offset};
            assert(ef.get_idx(pos_ef) == idx);
            assert(prefix_index(plain, pos_plain) == idx);
            const auto next_plain = plain.move(pos_plain);
            const auto next_ef = ef.move(pos_ef);
            assert(ef.get_idx(next_ef) == prefix_index(plain, next_plain));
            ++idx;
        }
    }
    assert(idx == ef.domain());

    std::stringstream ss;
    assert(ef.serialize(ss) > 0);
    move_structure_impl<move_columns> loaded;
    loaded.load(ss);
    assert(loaded.stores_ef_high_bits());
    auto loaded_pos = loaded.first();
    ulint loaded_idx = 0;
    for (size_t interval = 0; interval < loaded.intervals(); ++interval) {
        for (ulint offset = 0; offset < loaded.get_length(interval); ++offset) {
            loaded_pos = {interval, offset};
            assert(loaded.get_idx(loaded_pos) == loaded_idx);
            ++loaded_idx;
        }
    }
    assert(loaded_idx == loaded.domain());
}

static void test_get_idx_absolute_and_gates() {
    const vector<ulint> lengths = {3, 2, 1, 2, 2};
    const vector<ulint> perm = {4, 0, 9, 2, 7};

    move_structure_impl<move_columns_idx> absolute(lengths, perm, NO_SPLITTING);
    auto pos = absolute.first();
    assert(absolute.get_idx(pos) == pos.idx);
    pos = absolute.move(pos);
    assert(absolute.get_idx(pos) == pos.idx);

    expect_abort([&] {
        move_structure_impl<move_columns> relative(lengths, perm, NO_SPLITTING);
        relative.get_idx(relative.first());
    });
    expect_abort([&] {
        move_structure_impl<move_columns> rejected(lengths, perm, NO_SPLITTING, true);
        (void)rejected;
    });
    expect_abort([&] {
        move_structure_impl<move_columns_idx> rejected(lengths, perm, ONLY_LENGTH_CAPPING, true);
        (void)rejected;
    });
}

int main() {
    test_move_structure_relative_build_invariants();
    test_move_structure_absolute_build_invariants();
    test_move_structure_serialize_roundtrip();
    test_move_structure_widths_relative_and_absolute_with_and_without_splitting();
    test_ef_get_idx_matches_prefix_and_move();
    test_get_idx_absolute_and_gates();

    std::cout << "move_structure unit tests passed" << std::endl;
    return 0;
}
