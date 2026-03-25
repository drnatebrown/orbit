// Unit tests for packed_matrix / packed_vector and their aligned variants.
// These are simple assert-based tests, no external framework.

#include "orbit/internal/ds/packed_vector.hpp"
#include "orbit/internal/ds/packed_vector_aligned.hpp"

#include <cassert>
#include <array>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

using std::array;
using std::size_t;
using std::vector;

using namespace orbit;

// Basic sanity for packed_matrix with a single column.
void test_packed_matrix_single_column() {
    constexpr size_t NumCols = 1;
    constexpr uchar width = 6; // values in [0, 63]
    const size_t rows = 32;

    packed_matrix<NumCols> m(rows, array<uchar, NumCols>{width});
    assert(m.rows() == rows);
    assert(m.cols() == NumCols);

    for (size_t i = 0; i < rows; ++i) {
        ulint value = static_cast<ulint>(i % (1u << width));
        m.set<0>(i, value);
    }

    for (size_t i = 0; i < rows; ++i) {
        ulint expected = static_cast<ulint>(i % (1u << width));
        assert(m.get<0>(i) == expected);
    }
}

// Multi-column test that exercises crossing byte boundaries.
void test_packed_matrix_multi_column() {
    // Two 7-bit columns => 14 bits per row, so second column crosses byte boundary.
    constexpr size_t NumCols = 2;
    const array<uchar, NumCols> widths{7, 7};
    const size_t rows = 64;

    packed_matrix<NumCols> m(rows, widths);
    assert(m.rows() == rows);
    assert(m.cols() == NumCols);

    for (size_t i = 0; i < rows; ++i) {
        ulint v0 = static_cast<ulint>(i % 100);
        ulint v1 = static_cast<ulint>((rows - 1 - i) % 100);
        v0 &= mask(widths[0]);
        v1 &= mask(widths[1]);
        m.set<0>(i, v0);
        m.set<1>(i, v1);
    }

    for (size_t i = 0; i < rows; ++i) {
        ulint expected0 = static_cast<ulint>(i % 100) & mask(widths[0]);
        ulint expected1 = static_cast<ulint>((rows - 1 - i) % 100) & mask(widths[1]);
        assert(m.get<0>(i) == expected0);
        assert(m.get<1>(i) == expected1);
    }
}

// Round-trip serialize/load for int_vector.
void test_int_vector_serialize_roundtrip() {
    const size_t rows = 50;
    const uchar width = 10; // values < 1024

    int_vector v(rows, width);
    for (size_t i = 0; i < rows; ++i) {
        v.set(i, static_cast<ulint>((i * 7) % (1u << width)));
    }

    std::stringstream ss;
    size_t bytes_written = v.serialize(ss);
    assert(bytes_written > 0);

    int_vector loaded;
    loaded.load(ss);
    assert(loaded.rows() == rows);

    for (size_t i = 0; i < rows; ++i) {
        ulint expected = static_cast<ulint>((i * 7) % (1u << width));
        assert(loaded.get(i) == expected);
    }
}

void test_int_vector_vector_ctor_non_empty() {
    std::vector<ulint> data;
    data.reserve(64);
    for (size_t i = 0; i < 64; ++i) {
        data.push_back(static_cast<ulint>((i * 17) ^ (i >> 1)));
    }

    int_vector v(data);
    assert(v.rows() == data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        assert(v.get(i) == data[i]);
    }
}

void test_int_vector_vector_ctor_empty() {
    std::vector<ulint> data;
    int_vector v(data);
    assert(v.rows() == 0);
}

// (data, width) constructor: initialize from vector with explicit bit width.
void test_int_vector_vector_width_ctor() {
    std::vector<ulint> data = {0, 1, 15, 127, 255}; // fits in 8 bits
    const uchar width = 8;

    int_vector v(data, width);
    assert(v.rows() == data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        assert(v.get(i) == data[i]);
    }
}

// Iterator tests for int_vector: mutable and const access.
void test_int_vector_iterators() {
    const size_t rows = 32;
    const uchar width = 10;

    int_vector v(rows, width);

    // Fill via mutable iterators (exercise proxy assignment).
    ulint value = 0;
    for (auto it = v.begin(); it != v.end(); ++it, ++value) {
        *it = value;
    }

    // Verify via get and operator[].
    for (size_t i = 0; i < rows; ++i) {
        assert(v.get(i) == static_cast<ulint>(i));
        assert(v[i] == static_cast<ulint>(i));
    }

    // Exercise a standard algorithm (read/write through iterators).
    std::for_each(v.begin(), v.end(), [](int_vector::reference x) {
        ulint current = static_cast<ulint>(x);
        x = current * 2;
    });

    for (size_t i = 0; i < rows; ++i) {
        assert(v.get(i) == static_cast<ulint>(2 * i));
    }

    // Const iteration: range-for over const reference.
    const int_vector& cv = v;
    size_t idx = 0;
    for (auto x : cv) {
        assert(x == cv.get(idx));
        ++idx;
    }
    assert(idx == rows);
}

// Tests for the Columns-based packed_vector facade.
void test_packed_vector_with_enum() {
    enum class Columns {
        A,
        B,
        C,
        COUNT
    };

    constexpr size_t NumCols = static_cast<size_t>(Columns::COUNT);
    const array<uchar, NumCols> widths{4, 5, 6};
    const size_t rows = 40;

    packed_vector<Columns> vec(rows, widths);
    assert(vec.rows() == rows);
    assert(vec.cols() == NumCols);

    for (size_t i = 0; i < rows; ++i) {
        ulint a = static_cast<ulint>(i & mask(widths[0]));
        ulint b = static_cast<ulint>((i * 3) & mask(widths[1]));
        ulint c = static_cast<ulint>((i * 5) & mask(widths[2]));
        vec.set<Columns::A>(i, a);
        vec.set<Columns::B>(i, b);
        vec.set<Columns::C>(i, c);
    }

    for (size_t i = 0; i < rows; ++i) {
        ulint expected_a = static_cast<ulint>(i & mask(widths[0]));
        ulint expected_b = static_cast<ulint>((i * 3) & mask(widths[1]));
        ulint expected_c = static_cast<ulint>((i * 5) & mask(widths[2]));
        assert(vec.get<Columns::A>(i) == expected_a);
        assert(vec.get<Columns::B>(i) == expected_b);
        assert(vec.get<Columns::C>(i) == expected_c);
    }
}

// Aligned variants: layout differs but logical behaviour should match.
void test_packed_matrix_aligned_basic() {
    constexpr size_t NumCols = 3;
    const array<uchar, NumCols> widths{3, 5, 9};
    const size_t rows = 32;

    packed_matrix_aligned<NumCols> m(rows, widths);
    assert(m.rows() == rows);
    assert(m.cols() == NumCols);

    for (size_t i = 0; i < rows; ++i) {
        array<ulint, NumCols> vals{
            static_cast<ulint>(i & mask(widths[0])),
            static_cast<ulint>((i * 2) & mask(widths[1])),
            static_cast<ulint>((i * 7) & mask(widths[2]))
        };
        m.set_row(i, vals);
    }

    for (size_t i = 0; i < rows; ++i) {
        auto row_vals = m.get_row(i);
        assert(row_vals[0] == static_cast<ulint>(i & mask(widths[0])));
        assert(row_vals[1] == static_cast<ulint>((i * 2) & mask(widths[1])));
        assert(row_vals[2] == static_cast<ulint>((i * 7) & mask(widths[2])));
    }
}

void test_int_vector_aligned() {
    const size_t rows = 48;
    const uchar width = 11; // values < 2048

    int_vector_aligned v(rows, width);
    for (size_t i = 0; i < rows; ++i) {
        ulint val = static_cast<ulint>((i * 9) % (1u << width));
        v.set(i, val);
    }

    for (size_t i = 0; i < rows; ++i) {
        ulint expected = static_cast<ulint>((i * 9) % (1u << width));
        assert(v.get(i) == expected);
    }
}

void test_int_vector_aligned_vector_ctor_non_empty() {
    std::vector<ulint> data;
    data.reserve(64);
    for (size_t i = 0; i < 64; ++i) {
        // keep values small-ish but non-trivial
        data.push_back(static_cast<ulint>((i * 17) ^ (i >> 1)));
    }

    int_vector_aligned v(data);
    assert(v.rows() == data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        assert(v.get(i) == data[i]);
    }
}

void test_int_vector_aligned_vector_ctor_empty() {
    std::vector<ulint> data;
    int_vector_aligned v(data);
    assert(v.rows() == 0);
}

// (data, width) constructor for aligned variant.
void test_int_vector_aligned_vector_width_ctor() {
    std::vector<ulint> data = {0, 100, 500, 1023}; // fits in 10 bits
    const uchar width = 10;

    int_vector_aligned v(data, width);
    assert(v.rows() == data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        assert(v.get(i) == data[i]);
    }
}

// Iterator tests for int_vector_aligned: mutable and const access.
void test_int_vector_aligned_iterators() {
    const size_t rows = 40;
    const uchar width = 12;

    int_vector_aligned v(rows, width);

    // Use std::iota to fill via iterators.
    std::iota(v.begin(), v.end(), static_cast<ulint>(0));

    for (size_t i = 0; i < rows; ++i) {
        assert(v.get(i) == static_cast<ulint>(i));
        assert(v[i] == static_cast<ulint>(i));
    }

    // Use std::sort with a custom comparator to ensure compatibility.
    std::sort(v.begin(), v.end(), [](ulint a, ulint b) {
        return a > b; // sort in descending order
    });

    for (size_t i = 0; i < rows; ++i) {
        ulint expected = static_cast<ulint>(rows - 1 - i);
        assert(v.get(i) == expected);
    }

    // Const iteration over aligned vector.
    const int_vector_aligned& cv = v;
    size_t idx = 0;
    for (auto x : cv) {
        assert(x == cv.get(idx));
        ++idx;
    }
    assert(idx == rows);
}

// Round-trip serialize/load for int_vector_aligned.
void test_int_vector_aligned_serialize_roundtrip() {
    const size_t rows = 32;
    const uchar width = 9; // values < 512

    int_vector_aligned v(rows, width);
    for (size_t i = 0; i < rows; ++i) {
        ulint val = static_cast<ulint>((i * 13) % (1u << width));
        v.set(i, val);
    }

    std::stringstream ss;
    size_t bytes_written = v.serialize(ss);
    assert(bytes_written > 0);

    int_vector_aligned loaded;
    loaded.load(ss);
    assert(loaded.rows() == rows);

    for (size_t i = 0; i < rows; ++i) {
        ulint expected = static_cast<ulint>((i * 13) % (1u << width));
        assert(loaded.get(i) == expected);
    }
}

void test_packed_vector_aligned_with_enum() {
    enum class Columns {
        X,
        Y,
        COUNT
    };

    constexpr size_t NumCols = static_cast<size_t>(Columns::COUNT);
    const array<uchar, NumCols> widths{6, 10};
    const size_t rows = 37;

    packed_vector_aligned<Columns> vec(rows, widths);
    assert(vec.rows() == rows);
    assert(vec.cols() == NumCols);

    for (size_t i = 0; i < rows; ++i) {
        ulint x = static_cast<ulint>((i * 5) & mask(widths[0]));
        ulint y = static_cast<ulint>((i * 11) & mask(widths[1]));
        vec.set<Columns::X>(i, x);
        vec.set<Columns::Y>(i, y);
    }

    for (size_t i = 0; i < rows; ++i) {
        ulint expected_x = static_cast<ulint>((i * 5) & mask(widths[0]));
        ulint expected_y = static_cast<ulint>((i * 11) & mask(widths[1]));
        assert(vec.get<Columns::X>(i) == expected_x);
        assert(vec.get<Columns::Y>(i) == expected_y);
    }
}

int main() {
    test_packed_matrix_single_column();
    test_packed_matrix_multi_column();
    test_int_vector_serialize_roundtrip();
    test_int_vector_vector_ctor_non_empty();
    test_int_vector_vector_ctor_empty();
    test_int_vector_vector_width_ctor();
    test_int_vector_iterators();
    test_packed_vector_with_enum();
    test_packed_matrix_aligned_basic();
    test_int_vector_aligned();
    test_int_vector_aligned_vector_ctor_non_empty();
    test_int_vector_aligned_vector_ctor_empty();
    test_int_vector_aligned_vector_width_ctor();
    test_int_vector_aligned_serialize_roundtrip();
    test_packed_vector_aligned_with_enum();
    test_int_vector_aligned_iterators();

    std::cout << "packed_vector tests passed" << std::endl;
    return 0;
}
