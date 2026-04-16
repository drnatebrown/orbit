// Integration test: verify that the RLBWT, Move*, and RunPerm* components
// work together to reconstruct the original text and suffix array from a
// small, known example.

#include <iostream>
#include <cassert>
#include <vector>
#include <sstream>

#include "orbit/rlbwt.hpp"
#include "orbit/permutation.hpp"

using namespace orbit;
using namespace orbit::rlbwt;

void test_move_lf(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
    lf_move<> mlf(bwt_heads, bwt_run_lengths);

    using position = typename lf_move<>::position;
    auto pos = mlf.first();
    for (size_t i = 0; i < mlf.domain(); ++i) {
        pos = mlf.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text(text.size(), '\0');
    pos = mlf.first();
    for (size_t i = 1; i < mlf.domain(); ++i) {
        recovered_text[text.size() - i] = (char)mlf.get_character(pos);
        pos = mlf.next(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_move_lf_with_splitting(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
    lf_move<> mlf(bwt_heads, bwt_run_lengths, DEFAULT_SPLITTING);

    using position = typename lf_move<>::position;
    auto pos = mlf.first();
    for (size_t i = 0; i < mlf.domain(); ++i) {
        pos = mlf.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text(text.size(), '\0');
    pos = mlf.first();
    for (size_t i = 1; i < mlf.domain(); ++i) {
        recovered_text[text.size() - i] = (char)mlf.get_character(pos);
        pos = mlf.next(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_move_lf_serialize_roundtrip(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
    lf_move<> move_lf_obj(bwt_heads, bwt_run_lengths);

    std::stringstream ss;
    size_t bytes = move_lf_obj.serialize(ss);
    assert(bytes > 0);

    lf_move<> loaded;
    loaded.load(ss);

    using position = typename lf_move<>::position;
    auto pos = loaded.first();
    for (size_t i = 0; i < loaded.domain(); ++i) {
        pos = loaded.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text(text.size(), '\0');
    pos = loaded.first();
    for (size_t i = 1; i < loaded.domain(); ++i) {
        recovered_text[text.size() - i] = (char)loaded.get_character(pos);
        pos = loaded.next(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_move_fl(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
    fl_move<> mfl(bwt_heads, bwt_run_lengths);

    using position = typename fl_move<>::position;
    position pos = mfl.first();
    for (size_t i = 0; i < mfl.domain(); ++i) {
        pos = mfl.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text = "";
    pos = mfl.first();
    pos = mfl.FL(pos);
    for (size_t i = 1; i < mfl.domain(); ++i) {
        recovered_text += (char)mfl.get_character(pos);
        pos = mfl.FL(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_runperm_lf(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 10;  // Some dummy value
        run_data[i][1] = i * 5;   // Another dummy value
    }
    
    // Try the constructor without SplitParams
    lf_permutation<RunData> rp_lf(bwt_heads, bwt_run_lengths, run_data);
    
                
    rp_lf.first();
    using position = typename lf_permutation<RunData>::position;
    position pos = rp_lf.first();
    
    for (size_t i = 0; i < rp_lf.domain(); ++i) {
        pos = rp_lf.LF(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text(text.size(), '\0');
    pos = rp_lf.first();
    for (size_t i = 1; i < rp_lf.domain(); ++i) {
        recovered_text[text.size() - i] = (char)rp_lf.get_character(pos);
        pos = rp_lf.LF(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_runperm_lf_with_splitting(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 10;  // Some dummy value
        run_data[i][1] = i * 5;   // Another dummy value
    }
    
    // Try the constructor with SplitParams that enable splitting
    lf_permutation<RunData> rp_lf(bwt_heads, bwt_run_lengths, DEFAULT_SPLITTING, run_data);
    
                
    rp_lf.first();
    using position = typename lf_permutation<RunData>::position;
    position pos = rp_lf.first();
    
    for (size_t i = 0; i < rp_lf.domain(); ++i) {
        pos = rp_lf.LF(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text(text.size(), '\0');
    pos = rp_lf.first();
    for (size_t i = 1; i < rp_lf.domain(); ++i) {
        recovered_text[text.size() - i] = (char)rp_lf.get_character(pos);
        pos = rp_lf.LF(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_runperm_lf_serialize_roundtrip(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 10;  // Some dummy value
        run_data[i][1] = i * 5;   // Another dummy value
    }
    
    lf_permutation<RunData> runperm_lf_obj(bwt_heads, bwt_run_lengths, run_data);

    std::stringstream ss;
    size_t bytes = runperm_lf_obj.serialize(ss);
    assert(bytes > 0);

    lf_permutation<RunData> loaded;
        loaded.load(ss);
    
    using position = typename lf_permutation<RunData>::position;
    position pos = loaded.first();
    
    for (size_t i = 0; i < loaded.domain(); ++i) {
        pos = loaded.LF(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text(text.size(), '\0');
    pos = loaded.first();
    for (size_t i = 1; i < loaded.domain(); ++i) {
        recovered_text[text.size() - i] = (char)loaded.get_character(pos);
        pos = loaded.LF(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_runperm_fl(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 7;   // Some dummy value
        run_data[i][1] = i * 3;   // Another dummy value
    }
    
    // Try the constructor without SplitParams
    fl_permutation<RunData> rp_fl(bwt_heads, bwt_run_lengths, run_data);
    
                
    using position = typename fl_permutation<RunData>::position;
    position pos = rp_fl.first();
    
    for (size_t i = 0; i < rp_fl.domain(); ++i) {
        pos = rp_fl.FL(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = rp_fl.first();
    pos = rp_fl.FL(pos);
    std::string recovered_text = "";
    for (size_t i = 1; i < rp_fl.domain(); ++i) {
        recovered_text += (char)rp_fl.get_character(pos);
        pos = rp_fl.FL(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_move_fl_with_splitting(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
    fl_move<> mfl(bwt_heads, bwt_run_lengths, DEFAULT_SPLITTING);

    using position = typename fl_move<>::position;
    position pos = mfl.first();
    for (size_t i = 0; i < mfl.domain(); ++i) {
        pos = mfl.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    std::string recovered_text = "";
    pos = mfl.first();
    pos = mfl.FL(pos);
    for (size_t i = 1; i < mfl.domain(); ++i) {
        recovered_text += (char)mfl.get_character(pos);
        pos = mfl.FL(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_runperm_fl_with_splitting(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::string text) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 7;   // Some dummy value
        run_data[i][1] = i * 3;   // Another dummy value
    }
    
    fl_permutation<RunData> rp_fl(bwt_heads, bwt_run_lengths, DEFAULT_SPLITTING, run_data);
    
                
    using position = typename fl_permutation<RunData>::position;
    position pos = rp_fl.first();
    
    for (size_t i = 0; i < rp_fl.domain(); ++i) {
        pos = rp_fl.FL(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = rp_fl.first();
    pos = rp_fl.FL(pos);
    std::string recovered_text = "";
    for (size_t i = 1; i < rp_fl.domain(); ++i) {
        recovered_text += (char)rp_fl.get_character(pos);
        pos = rp_fl.FL(pos);
    }
    assert(recovered_text.compare(text) == 0);
}

void test_runperm_phi(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::vector<ulint> sa) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 7;   // Some dummy value
        run_data[i][1] = i * 3;   // Another dummy value
    }

    size_t phi_domain;
    ulint max_length;
    auto [phi_lengths, phi_images] = rlbwt_to_phi_images(bwt_heads, bwt_run_lengths, &phi_domain, &max_length);
    phi_permutation<RunData> runperm_phi_obj(phi_lengths, phi_images, run_data);
            
    using position = typename phi_permutation<RunData>::position;
    position pos = runperm_phi_obj.first();
    for (size_t i = 0; i < runperm_phi_obj.domain(); ++i) {
        pos = runperm_phi_obj.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = runperm_phi_obj.last();
    pos = runperm_phi_obj.phi(pos);
    std::vector<ulint> sa_recovered(sa.size());
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        sa_recovered[sa.size() - i - 1] = runperm_phi_obj.SA(pos);
        pos = runperm_phi_obj.next(pos);
    }
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        assert(sa_recovered[i] == sa[i]);
    }
}

void test_move_phi_with_splitting(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::vector<ulint> sa) {
    size_t phi_domain;
    ulint max_length;
    auto [phi_lengths, phi_images] = rlbwt_to_phi_images(bwt_heads, bwt_run_lengths, &phi_domain, &max_length);
    phi_move move_phi_obj(phi_lengths, phi_images, DEFAULT_SPLITTING);
    using position = typename phi_move::position;
    position pos = move_phi_obj.first();
    for (size_t i = 0; i < move_phi_obj.domain(); ++i) {
        pos = move_phi_obj.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = move_phi_obj.last();
    pos = move_phi_obj.phi(pos);
    std::vector<ulint> sa_recovered(sa.size());
    for (size_t i = 0; i < move_phi_obj.domain(); ++i) {
        sa_recovered[sa.size() - i - 1] = move_phi_obj.SA(pos);
        pos = move_phi_obj.phi(pos);
    }
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        assert(sa_recovered[i] == sa[i]);
    }
}

void test_move_phi(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::vector<ulint> sa) {
    size_t phi_domain;
    ulint max_length;
    auto [phi_lengths, phi_images] = rlbwt_to_phi_images(bwt_heads, bwt_run_lengths, &phi_domain, &max_length);
    phi_move move_phi_obj(phi_lengths, phi_images);
    using position = typename phi_move::position;
    position pos = move_phi_obj.first();
    for (size_t i = 0; i < move_phi_obj.domain(); ++i) {
        pos = move_phi_obj.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = move_phi_obj.last();
    pos = move_phi_obj.phi(pos);
    std::vector<ulint> sa_recovered(sa.size());
    for (size_t i = 0; i < move_phi_obj.domain(); ++i) {
        sa_recovered[sa.size() - i - 1] = move_phi_obj.SA(pos);
        pos = move_phi_obj.phi(pos);
    }
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        assert(sa_recovered[i] == sa[i]);
    }
}

void test_runperm_phi_inv(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::vector<ulint> sa) {
        
    // Define dummy run data similar to RunPerm tests
    enum class RunData {
        VAL_1,
        VAL_2,
        COUNT
    };
    static constexpr size_t NUM_FIELDS = static_cast<size_t>(RunData::COUNT);
    std::vector<std::array<ulint, NUM_FIELDS>> run_data(bwt_heads.size());
    
    // Fill with dummy data
    for (size_t i = 0; i < bwt_heads.size(); ++i) {
        run_data[i][0] = i * 7;   // Some dummy value
        run_data[i][1] = i * 3;   // Another dummy value
    }

    size_t inv_domain;
    ulint max_length_inv;
    auto [phi_inv_lengths, phi_inv_images] = rlbwt_to_phi_inv_images(bwt_heads, bwt_run_lengths, &inv_domain, &max_length_inv);
    phi_inv_permutation<RunData> runperm_phi_inv_obj(phi_inv_lengths, phi_inv_images, run_data);
            
    runperm_phi_inv_obj.first();
    using position = typename phi_inv_permutation<RunData>::position;
    position pos = runperm_phi_inv_obj.first();
    for (size_t i = 0; i < runperm_phi_inv_obj.domain(); ++i) {
        pos = runperm_phi_inv_obj.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = runperm_phi_inv_obj.last();
    std::vector<ulint> sa_recovered(sa.size());
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        sa_recovered[i] = runperm_phi_inv_obj.SA(pos);
        pos = runperm_phi_inv_obj.phi_inv(pos);
    }
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        assert(sa_recovered[i] == sa[i]);
    }
}

void test_move_phi_inv(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::vector<ulint> sa) {
    size_t inv_domain;
    ulint max_length_inv;
    auto [phi_inv_lengths, phi_inv_images] = rlbwt_to_phi_inv_images(bwt_heads, bwt_run_lengths, &inv_domain, &max_length_inv);
    phi_inv_move move_phi_inv_obj(phi_inv_lengths, phi_inv_images);
    using position = typename phi_inv_move::position;
    position pos = move_phi_inv_obj.first();
    for (size_t i = 0; i < move_phi_inv_obj.domain(); ++i) {
        pos = move_phi_inv_obj.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = move_phi_inv_obj.last();
    std::vector<ulint> sa_recovered(sa.size());
    for (size_t i = 0; i < move_phi_inv_obj.domain(); ++i) {
        sa_recovered[i] = move_phi_inv_obj.SA(pos);
        pos = move_phi_inv_obj.phi_inv(pos);
    }
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        assert(sa_recovered[i] == sa[i]);
    }
}

void test_move_phi_inv_with_splitting(std::vector<uchar> bwt_heads, std::vector<ulint> bwt_run_lengths, std::vector<ulint> sa) {
    size_t inv_domain;
    ulint max_length_inv;
    auto [phi_inv_lengths, phi_inv_images] = rlbwt_to_phi_inv_images(bwt_heads, bwt_run_lengths, &inv_domain, &max_length_inv);
    phi_inv_move move_phi_inv_obj(phi_inv_lengths, phi_inv_images, DEFAULT_SPLITTING);
    using position = typename phi_inv_move::position;
    position pos = move_phi_inv_obj.first();
    for (size_t i = 0; i < move_phi_inv_obj.domain(); ++i) {
        pos = move_phi_inv_obj.next(pos);
    }
    assert(pos.interval == 0);
    assert(pos.offset == 0);
    
    pos = move_phi_inv_obj.last();
    std::vector<ulint> sa_recovered(sa.size());
    for (size_t i = 0; i < move_phi_inv_obj.domain(); ++i) {
        sa_recovered[i] = move_phi_inv_obj.SA(pos);
        pos = move_phi_inv_obj.phi_inv(pos);
    }
    for (size_t i = 0; i < sa_recovered.size(); ++i) {
        assert(sa_recovered[i] == sa[i]);
    }
}

int main() {
    // TEXT: GATTACATGATTACATAGATTACATT$
    // BWT:  TTTTTCCCGGGAAAT$ATTTTAAAAAA
    // RLBWT: TCGAT$ATA
    std::string text = "GATTACATGATTACATAGATTACATT";
    std::vector<ulint> sa = {26, 12, 4, 21, 16, 14, 6, 23, 9, 1, 18, 13, 5, 22, 8, 0, 17, 25, 11, 3, 20, 15, 7, 24, 10, 2, 19};
    std::vector<uchar> bwt_heads =       {'T','C','G','A','T', 1 ,'A','T','A'};
    std::vector<ulint> bwt_run_lengths = { 5 , 3 , 3 , 3 , 1 , 1 , 1 , 4 , 6 };
    test_move_lf(bwt_heads, bwt_run_lengths, text);
    test_move_lf_with_splitting(bwt_heads, bwt_run_lengths, text);
    test_move_lf_serialize_roundtrip(bwt_heads, bwt_run_lengths, text);
    test_move_fl(bwt_heads, bwt_run_lengths, text);
    test_move_fl_with_splitting(bwt_heads, bwt_run_lengths, text);
    test_runperm_lf(bwt_heads, bwt_run_lengths, text);
    test_runperm_lf_with_splitting(bwt_heads, bwt_run_lengths, text);
    test_runperm_lf_serialize_roundtrip(bwt_heads, bwt_run_lengths, text);
    test_runperm_fl(bwt_heads, bwt_run_lengths, text);
    test_runperm_fl_with_splitting(bwt_heads, bwt_run_lengths, text);
    test_runperm_phi(bwt_heads, bwt_run_lengths, sa);
    test_move_phi(bwt_heads, bwt_run_lengths, sa);
    test_move_phi_with_splitting(bwt_heads, bwt_run_lengths, sa);
    test_runperm_phi_inv(bwt_heads, bwt_run_lengths, sa);
    test_move_phi_inv(bwt_heads, bwt_run_lengths, sa);
    test_move_phi_inv_with_splitting(bwt_heads, bwt_run_lengths, sa);
    std::cout << "rlbwt integration tests passed" << std::endl;
    return 0;
}
