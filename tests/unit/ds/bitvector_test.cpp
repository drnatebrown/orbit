// Unit tests for bit_vector simple select.

#include "orbit/internal/ds/bitvector.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

using namespace orbit;

static std::vector<size_t> brute_ones(const bit_vector& bv) {
    std::vector<size_t> ones;
    for (size_t i = 0; i < bv.size(); ++i) {
        if (bv.get(i)) ones.push_back(i);
    }
    return ones;
}

static void check_select(bit_vector& bv) {
    bv.build_select();
    const auto ones = brute_ones(bv);
    assert(ones.size() == bv.num_ones());
    for (size_t k = 0; k < ones.size(); ++k) {
        assert(bv.select1(k) == ones[k]);
    }
}

static void test_dense_and_sparse() {
    bit_vector dense(200);
    for (size_t i = 0; i < 200; i += 2) dense.set(i);
    check_select(dense);

    bit_vector sparse(100000);
    sparse.set(0);
    sparse.set(1);
    sparse.set(70000);
    sparse.set(70001);
    sparse.set(99999);
    check_select(sparse);

    bit_vector single(8);
    single.set(5);
    check_select(single);

    bit_vector empty(0);
    check_select(empty);
}

static void test_long_block_and_many_ones() {
    // Gaps larger than 16 bits force the long-block spill path, and more
    // than 256 ones crosses an inventory boundary.
    bit_vector bv(300000);
    for (size_t i = 0; i < 300; ++i) {
        bv.set(i * 1000);
    }
    check_select(bv);
}

static void test_serialize_roundtrip() {
    bit_vector bv(1000);
    for (size_t i = 0; i < 1000; i += 3) bv.set(i);
    bv.build_select();

    std::stringstream ss;
    bv.serialize(ss);
    bit_vector loaded;
    loaded.load(ss);
    assert(loaded.size() == bv.size());
    assert(loaded.num_ones() == bv.num_ones());
    for (size_t k = 0; k < bv.num_ones(); ++k) {
        assert(loaded.select1(k) == bv.select1(k));
    }
}

int main() {
    test_dense_and_sparse();
    test_long_block_and_many_ones();
    test_serialize_roundtrip();
    std::cout << "bitvector tests passed" << std::endl;
    return 0;
}
