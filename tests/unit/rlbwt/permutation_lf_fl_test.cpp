// Tiny unit tests for LF/FL-based RLBWT components.
// Focus: wrapper methods (LF/FL) behave like next(), not full text reconstruction.

#include "orbit/rlbwt.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace orbit;
using namespace orbit::rlbwt;

using std::size_t;
using std::vector;

void test_move_lf_wrapper_equivalence() {
    // Same small RLBWT example as in the integration test, but we only
    // check a few LF steps.
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    lf_move<> mlf(bwt_heads, bwt_run_lengths);

    using position = typename lf_move<>::position;
    position start = mlf.first();

    // Single-step LF must match next().
    position lf1 = mlf.LF(start);
    position next1 = mlf.next(start);
    assert(lf1.interval == next1.interval);
    assert(lf1.offset == next1.offset);

    // Multi-step LF(pos, k) must match k repeated LF calls.
    position lf3 = mlf.LF(start, 3);
    position iter = start;
    iter = mlf.LF(iter);
    iter = mlf.LF(iter);
    iter = mlf.LF(iter);
    assert(lf3.interval == iter.interval);
    assert(lf3.offset == iter.offset);
}

void test_runperm_lf_wrapper_equivalence() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    enum class RunCols {
        V,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunCols::COUNT);
    vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    for (size_t i = 0; i < run_data.size(); ++i) {
        run_data[i][0] = static_cast<ulint>(i);
    }

    lf_permutation<RunCols> rp_lf(bwt_heads, bwt_run_lengths, run_data);

    using position = typename lf_permutation<RunCols>::position;
    position start = rp_lf.first();

    position lf1 = rp_lf.LF(start);
    position next1 = rp_lf.next(start);
    assert(lf1.interval == next1.interval);
    assert(lf1.offset == next1.offset);

    position lf2 = rp_lf.LF(start, 2);
    position iter = start;
    iter = rp_lf.LF(iter);
    iter = rp_lf.LF(iter);
    assert(lf2.interval == iter.interval);
    assert(lf2.offset == iter.offset);
}

void test_move_fl_wrapper_equivalence() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    fl_move<> mfl(bwt_heads, bwt_run_lengths);

    using position = typename fl_move<>::position;
    position start = mfl.first();

    position fl1 = mfl.FL(start);
    position next1 = mfl.next(start);
    assert(fl1.interval == next1.interval);
    assert(fl1.offset == next1.offset);

    position fl4 = mfl.FL(start, 4);
    position iter = start;
    for (int i = 0; i < 4; ++i) {
        iter = mfl.FL(iter);
    }
    assert(fl4.interval == iter.interval);
    assert(fl4.offset == iter.offset);
}

void test_runperm_fl_wrapper_equivalence() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    enum class RunCols {
        V,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunCols::COUNT);
    vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    for (size_t i = 0; i < run_data.size(); ++i) {
        run_data[i][0] = static_cast<ulint>(i * 2);
    }

    fl_permutation<RunCols> rp_fl(bwt_heads, bwt_run_lengths, run_data);

    using position = typename fl_permutation<RunCols>::position;
    position start = rp_fl.first();

    position fl1 = rp_fl.FL(start);
    position next1 = rp_fl.next(start);
    assert(fl1.interval == next1.interval);
    assert(fl1.offset == next1.offset);

    position fl2 = rp_fl.FL(start, 2);
    position iter = start;
    iter = rp_fl.FL(iter);
    iter = rp_fl.FL(iter);
    assert(fl2.interval == iter.interval);
    assert(fl2.offset == iter.offset);
}

void test_move_lf_pred_succ_char_basic() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    lf_move<> mlf(bwt_heads, bwt_run_lengths);
    using position = typename lf_move<>::position;

    // Same-character query returns the same position unchanged.
    position p_same{4, 0};
    auto same = mlf.pred_char(p_same, static_cast<uchar>('T'));
    assert(same.has_value());
    assert(same->interval == p_same.interval);
    assert(same->offset == p_same.offset);
    same = mlf.succ_char(p_same, static_cast<uchar>('T'));
    assert(same.has_value());
    assert(same->interval == p_same.interval);
    assert(same->offset == p_same.offset);

    // Previous/next matching runs set offset to run end/run start respectively.
    position from_t{4, 0}; // T run between A and terminator.
    auto pred_a = mlf.pred_char(from_t, static_cast<uchar>('A'));
    assert(pred_a.has_value());
    assert(pred_a->interval == 3);
    assert(pred_a->offset == bwt_run_lengths[3] - 1);

    auto succ_a = mlf.succ_char(from_t, static_cast<uchar>('A'));
    assert(succ_a.has_value());
    assert(succ_a->interval == 6);
    assert(succ_a->offset == 0);

    // Boundary misses return nullopt.
    position from_c{1, 2};
    auto pred_term = mlf.pred_char(from_c, static_cast<uchar>(1));
    assert(!pred_term.has_value());

    position from_t_right{7, 1};
    auto succ_c = mlf.succ_char(from_t_right, static_cast<uchar>('C'));
    assert(!succ_c.has_value());
}

void test_move_lf_absolute_pred_succ_char_updates_idx() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    lf_move<true> move_lf_abs(bwt_heads, bwt_run_lengths);
    using position = typename lf_move<true>::position;

    position from_t{4, 0, 14};
    auto pred_a = move_lf_abs.pred_char(from_t, static_cast<uchar>('A'));
    assert(pred_a.has_value());
    assert(pred_a->interval == 3);
    assert(pred_a->offset == 2);
    assert(pred_a->idx == 13);

    auto succ_a = move_lf_abs.succ_char(from_t, static_cast<uchar>('A'));
    assert(succ_a.has_value());
    assert(succ_a->interval == 6);
    assert(succ_a->offset == 0);
    assert(succ_a->idx == 16);
}

void test_runperm_lf_pred_succ_char_basic() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    enum class RunCols {
        V,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunCols::COUNT);
    vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    for (size_t i = 0; i < run_data.size(); ++i) {
        run_data[i][0] = static_cast<ulint>(10 + i);
    }

    lf_permutation<RunCols> rp_lf(bwt_heads, bwt_run_lengths, run_data);
    using position = typename lf_permutation<RunCols>::position;

    position from_t{4, 0};
    auto pred_a = rp_lf.pred_char(from_t, static_cast<uchar>('A'));
    assert(pred_a.has_value());
    assert(pred_a->interval == 3);
    assert(pred_a->offset == bwt_run_lengths[3] - 1);

    auto succ_a = rp_lf.succ_char(from_t, static_cast<uchar>('A'));
    assert(succ_a.has_value());
    assert(succ_a->interval == 6);
    assert(succ_a->offset == 0);
}

void test_runpermlf_construct_from_precomputed_permutation_no_splitting() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    auto perm = rlbwt_interval_encoding<>::lf_interval_encoding(bwt_heads, bwt_run_lengths, NO_SPLITTING);

    enum class RunCols {
        V,
        COUNT
    };
    using RP = lf_permutation<RunCols>;
    using RunData = typename RP::data_tuple;

    vector<RunData> run_data(perm.intervals());
    for (size_t i = 0; i < run_data.size(); ++i) {
        run_data[i][0] = static_cast<ulint>(1000 + i);
    }

    RP rp_from_perm(perm, run_data);
    RP rp_direct(bwt_heads, bwt_run_lengths, NO_SPLITTING, run_data);

    assert(rp_from_perm.domain() == rp_direct.domain());
    assert(rp_from_perm.runs() == rp_direct.runs());
    assert(rp_from_perm.intervals() == rp_direct.intervals());
    assert(rp_from_perm.get_split_params() == NO_SPLITTING);

    auto p1 = rp_from_perm.first();
    auto p2 = rp_direct.first();
    for (size_t i = 0; i < rp_direct.domain(); ++i) {
        assert(rp_from_perm.get_character(p1) == rp_direct.get_character(p2));
        p1 = rp_from_perm.LF(p1);
        p2 = rp_direct.LF(p2);
    }
    assert(p1.interval == 0 && p1.offset == 0);
    assert(p2.interval == 0 && p2.offset == 0);

    for (size_t i = 0; i < rp_from_perm.intervals(); ++i) {
        assert(rp_from_perm.template get<RunCols::V>(static_cast<ulint>(i)) == static_cast<ulint>(1000 + i));
        assert(rp_direct .template get<RunCols::V>(static_cast<ulint>(i)) == static_cast<ulint>(1000 + i));
    }
}

void test_runpermlf_construct_from_precomputed_permutation_with_splitting() {
    vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    auto perm = rlbwt_interval_encoding<>::lf_interval_encoding(bwt_heads, bwt_run_lengths, DEFAULT_SPLITTING);
    assert(perm.intervals() >= perm.runs());

    enum class RunCols {
        V,
        COUNT
    };
    using RP = lf_permutation<RunCols>;
    using RunData = typename RP::data_tuple;

    vector<RunData> run_data_per_run(perm.runs());
    for (size_t i = 0; i < run_data_per_run.size(); ++i) {
        run_data_per_run[i][0] = static_cast<ulint>(2000 + i);
    }
    auto run_data_split = perm.split_run_data_with_copy(bwt_run_lengths, run_data_per_run);
    assert(run_data_split.size() == perm.intervals());

    RP rp_from_perm(perm, run_data_split);
    assert(rp_from_perm.get_split_params() == DEFAULT_SPLITTING);
    assert(rp_from_perm.intervals() == perm.intervals());

    for (size_t i = 0; i < rp_from_perm.intervals(); ++i) {
        assert(rp_from_perm.template get<RunCols::V>(static_cast<ulint>(i)) == run_data_split[i][0]);
    }
}

void test_bwt_to_rlbwt_basic() {
    {
        // Empty input -> empty output.
        vector<uchar> bwt;
        auto [heads, lengths] = bwt_to_rlbwt(bwt);
        assert(heads.empty());
        assert(lengths.empty());
    }
    {
        // Single run.
        vector<uchar> bwt = {'A', 'A', 'A'};
        auto [heads, lengths] = bwt_to_rlbwt(bwt);
        assert(heads.size() == 1);
        assert(lengths.size() == 1);
        assert(heads[0] == static_cast<uchar>('A'));
        assert(lengths[0] == 3);
    }
    {
        // Multiple runs.
        vector<uchar> bwt = {'A', 'A', 'C', 'C', 'C', 'G', 'G'};
        auto [heads, lengths] = bwt_to_rlbwt(bwt);

        assert(heads.size() == 3);
        assert(lengths.size() == 3);

        assert(heads[0] == static_cast<uchar>('A'));
        assert(lengths[0] == 2);

        assert(heads[1] == static_cast<uchar>('C'));
        assert(lengths[1] == 3);

        assert(heads[2] == static_cast<uchar>('G'));
        assert(lengths[2] == 2);

        // Sum of run lengths equals original length.
        ulint total = 0;
        for (ulint v : lengths) {
            total += v;
        }
        assert(total == bwt.size());
    }
}

void test_consuming_lf_encoding_matches_const_ctor() {
    vector<uchar> heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> lengths  =    { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };

    auto enc = rlbwt_interval_encoding<>::lf_interval_encoding(heads, lengths, NO_SPLITTING);
    auto enc_empty = enc;
    lf_move<> kept_empty(enc);
    assert(enc.get_heads().size() == kept_empty.intervals());
    lf_move<> consumed_empty(std::move(enc_empty));
    assert(consumed_empty.domain() == kept_empty.domain());
    assert(consumed_empty.intervals() == kept_empty.intervals());
    for (size_t i = 0; i < kept_empty.intervals(); ++i) {
        assert(consumed_empty.get_character(static_cast<ulint>(i)) == kept_empty.get_character(static_cast<ulint>(i)));
    }

    enum class RunCols { V, COUNT };
    using Row = std::array<ulint, 1>;
    vector<Row> rows(enc.intervals());
    vector<ulint> col(enc.intervals());
    for (size_t i = 0; i < rows.size(); ++i) {
        rows[i] = {static_cast<ulint>(i + 3)};
        col[i] = static_cast<ulint>(i + 3);
    }
    auto enc_rows = enc;
    vector<Row> rows_copy = rows;
    lf_permutation<RunCols> kept_rows(enc, rows);
    lf_permutation<RunCols> consumed_rows(std::move(enc_rows), std::move(rows_copy));

    auto enc_cols = enc;
    auto enc_cols_move = enc;
    std::array<vector<ulint>, 1> cols{col};
    std::array<vector<ulint>, 1> cols_copy = cols;
    lf_permutation<RunCols> kept_cols(enc_cols, cols);
    lf_permutation<RunCols> consumed_cols(std::move(enc_cols_move), std::move(cols_copy));

    assert(consumed_rows.domain() == kept_rows.domain());
    assert(consumed_cols.intervals() == kept_cols.intervals());
    for (size_t i = 0; i < kept_rows.intervals(); ++i) {
        assert(consumed_rows.get_character(static_cast<ulint>(i)) == kept_rows.get_character(static_cast<ulint>(i)));
        assert(consumed_rows.get<RunCols::V>(static_cast<ulint>(i)) == kept_rows.get<RunCols::V>(static_cast<ulint>(i)));
        assert(consumed_cols.get_character(static_cast<ulint>(i)) == kept_cols.get_character(static_cast<ulint>(i)));
        assert(consumed_cols.get<RunCols::V>(static_cast<ulint>(i)) == static_cast<ulint>(i + 3));
    }
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

template <typename Absolute, typename Relative>
static void assert_lf_ef_matches(Absolute& absolute, Relative& ef) {
    assert(ef.stores_ef_high_bits());
    assert(ef.domain() == absolute.domain());
    assert(ef.intervals() == absolute.intervals());
    auto pos_abs = absolute.first();
    auto pos_ef = ef.first();
    for (ulint i = 0; i < absolute.domain(); ++i) {
        assert(ef.get_character(pos_ef) == absolute.get_character(pos_abs));
        assert(ef.get_idx(pos_ef) == absolute.get_idx(pos_abs));
        assert(pos_ef.interval == pos_abs.interval);
        assert(pos_ef.offset == pos_abs.offset);
        pos_abs = absolute.LF(pos_abs);
        pos_ef = ef.LF(pos_ef);
    }
}

template <typename Absolute, typename Relative>
static void assert_fl_ef_matches(Absolute& absolute, Relative& ef) {
    assert(ef.stores_ef_high_bits());
    auto pos_abs = absolute.first();
    auto pos_ef = ef.first();
    for (ulint i = 0; i < absolute.domain(); ++i) {
        assert(ef.get_character(pos_ef) == absolute.get_character(pos_abs));
        assert(ef.get_idx(pos_ef) == absolute.get_idx(pos_abs));
        pos_abs = absolute.FL(pos_abs);
        pos_ef = ef.FL(pos_ef);
    }
}

static void test_rlbwt_relative_ef_lf_fl() {
    vector<uchar> heads = {'T','C','G','A','T', 1 ,'A','T','A'};
    vector<ulint> lens  = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };
    // Tight cap forces splits so character assignment cannot read PRIMARY as a length.
    const split_params cap(0.25, std::nullopt);

    lf_move_absolute plain_lf(heads, lens, cap);
    lf_move_relative ef_lf(heads, lens, cap, true);
    assert_lf_ef_matches(plain_lf, ef_lf);

    fl_move_absolute plain_fl(heads, lens, cap);
    fl_move_relative ef_fl(heads, lens, cap, true);
    assert_fl_ef_matches(plain_fl, ef_fl);

    invertible_lf_move_absolute inv_lf(heads, lens, cap);
    invertible_lf_move_relative inv_ef_lf(heads, lens, cap, true);
    assert_lf_ef_matches(inv_lf, inv_ef_lf);
    invertible_lf_move_scan<> inv_scan_lf(heads, lens, cap, true);
    invertible_lf_move_spill<> inv_spill_lf(heads, lens, cap, true);
    assert(inv_scan_lf.stores_ef_high_bits());
    assert(inv_spill_lf.stores_ef_high_bits());
    auto scan_pos = inv_scan_lf.first();
    auto spill_pos = inv_spill_lf.first();
    auto abs_pos = inv_lf.first();
    for (ulint i = 0; i < inv_lf.domain(); ++i) {
        assert(inv_scan_lf.get_idx(scan_pos) == inv_lf.get_idx(abs_pos));
        assert(inv_spill_lf.get_idx(spill_pos) == inv_lf.get_idx(abs_pos));
        assert(inv_scan_lf.get_character(scan_pos) == inv_lf.get_character(abs_pos));
        scan_pos = inv_scan_lf.LF(scan_pos);
        spill_pos = inv_spill_lf.LF(spill_pos);
        abs_pos = inv_lf.LF(abs_pos);
    }

    invertible_fl_move_absolute inv_fl(heads, lens, cap);
    invertible_fl_move_relative inv_ef_fl(heads, lens, cap, true);
    assert_fl_ef_matches(inv_fl, inv_ef_fl);

    expect_abort([&] {
        lf_move_absolute rejected(heads, lens, cap, true);
        (void)rejected;
    });
    expect_abort([&] {
        lf_move_relative rejected(heads, lens, NO_SPLITTING, true);
        (void)rejected;
    });
}

int main() {
    test_move_lf_wrapper_equivalence();
    test_runperm_lf_wrapper_equivalence();
    test_move_fl_wrapper_equivalence();
    test_runperm_fl_wrapper_equivalence();
    test_move_lf_pred_succ_char_basic();
    test_move_lf_absolute_pred_succ_char_updates_idx();
    test_runperm_lf_pred_succ_char_basic();
    test_runpermlf_construct_from_precomputed_permutation_no_splitting();
    test_runpermlf_construct_from_precomputed_permutation_with_splitting();
    test_bwt_to_rlbwt_basic();
    test_consuming_lf_encoding_matches_const_ctor();
    test_rlbwt_relative_ef_lf_fl();

    std::cout << "permutation_lf_fl unit tests passed" << std::endl;
    return 0;
}
