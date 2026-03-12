#ifndef _RUNPERM_PHI_HPP
#define _RUNPERM_PHI_HPP

#include "internal/common.hpp"
#include "internal/runperm/runperm.hpp"
#include "internal/rlbwt/runperm_lf.hpp"

// TODO give tau_inv instead of interval permutation

namespace phi {

using IntVec = IntVectorAligned;

template<typename LFType>
std::tuple<IntVec, IntVec, IntVec, size_t, size_t> rlbwt_to_phi_lengths_and_samples(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, LFType& lf, size_t* domain = nullptr, ulint* max_length = nullptr) {
    IntVec phi_lengths(lf.runs(), bit_width(lf.domain() - 1));

    size_t UNUSED_INTERVAL = MAX_VAL(bit_width(lf.intervals()) + 1);
    size_t UNUSED_SA = MAX_VAL(bit_width(lf.domain()) + 1);
    IntVec move_run_to_phi(lf.intervals(), bit_width(lf.intervals()) + 1); // Map move run to its Phi interval (only set those corresponding to RLBWT runs)
    IntVec run_tail_sa_samples(lf.intervals(), bit_width(lf.domain()) + 1); // The SA samples at the tail of each move run (only set those corresponding to RLBWT runs)

    ulint max_length_seen = 0;
    auto pos = lf.first();
    size_t last_sample = lf.domain();
    size_t sa = lf.domain() - 1;
    // Phi intervals correspond to the original (unsplit) permutation runs, not move runs.
    size_t curr_phi_interval = lf.runs() - 1;
    // Step through entire BWT to recover Phi structure and SA samples at tails
    for (size_t i = 0; i < lf.domain(); ++i) {
        size_t interval = pos.interval;
        size_t offset = pos.offset;
        // If at BWT runhead
        if (offset == 0) {
            if (interval == 0 || lf.get_character(interval - 1) != lf.get_character(interval)) {
                phi_lengths[curr_phi_interval] = last_sample - sa;
                max_length_seen = std::max(max_length_seen, static_cast<ulint>(phi_lengths[curr_phi_interval]));
                move_run_to_phi.set(interval, curr_phi_interval);
                last_sample = sa;
                --curr_phi_interval;
            }
            else {
                move_run_to_phi.set(interval, UNUSED_INTERVAL);
            }
        }
        // If at BWT run tail
        if (offset == lf.get_length(interval) - 1) {
            if (interval == lf.intervals() - 1 || lf.get_character(interval + 1) != lf.get_character(interval)) {
                run_tail_sa_samples.set(interval, sa);
            }
            else {
                run_tail_sa_samples.set(interval, UNUSED_SA);
            }
        }
        --sa;
        pos = lf.LF(pos);
    }

    if (domain != nullptr) {
        *domain = lf.domain();
    }
    if (max_length != nullptr) {
        *max_length = max_length_seen;
    }

    return {phi_lengths, move_run_to_phi, run_tail_sa_samples, UNUSED_INTERVAL, UNUSED_SA};
}

template<typename LFType>
IntVec samples_to_phi_interval_permutations(const IntVec& move_run_to_phi, const IntVec& run_tail_sa_samples, const size_t UNUSED_SA, const LFType& lf) {
    IntVec phi_interval_permutations(lf.runs(), bit_width(lf.domain() - 1));
    
    // Step through BWT tail samples to fill in Phi interval permutations
    for (size_t i = 0; i < run_tail_sa_samples.size(); ++i) {
        if (run_tail_sa_samples.get(i) == UNUSED_SA) continue;
        phi_interval_permutations[move_run_to_phi.get((i == lf.intervals() - 1) ? 0 : i + 1)] = run_tail_sa_samples.get(i);
    }

    return phi_interval_permutations;
}

template<typename LFType>
std::tuple<IntVec, IntVec> rlbwt_to_phi(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, LFType& lf, size_t* domain = nullptr, ulint* max_length = nullptr) {
    assert(bwt_heads.size() == bwt_run_lengths.size());
    auto [phi_lengths, move_run_to_phi, run_tail_sa_samples, _, UNUSED_SA] = rlbwt_to_phi_lengths_and_samples(bwt_heads, bwt_run_lengths, lf, domain, max_length);
    auto phi_interval_permutations = samples_to_phi_interval_permutations(move_run_to_phi, run_tail_sa_samples, UNUSED_SA, lf);
    return {phi_lengths, phi_interval_permutations};
}

template<typename AlphabetType=Nucleotide>
inline std::tuple<IntVec, IntVec> rlbwt_to_phi(const std::vector<uchar>& bwt_heads, const std::vector<ulint>& bwt_run_lengths, size_t* domain = nullptr, ulint* max_length = nullptr) {
    // Need a move structure with LF to find SA samples
    MoveLFImplDefault<AlphabetType> move_lf(bwt_heads, bwt_run_lengths);
    return rlbwt_to_phi(bwt_heads, bwt_run_lengths, move_lf, domain, max_length);
}

} // end namespace phi

// Always use absolute positions for Phi
template<typename RunColsType,
         bool IntegratedMoveStructure = DEFAULT_INTEGRATED_MOVE_STRUCTURE,
         bool ExponentialSearch = DEFAULT_EXPONENTIAL_SEARCH,
         template<typename> class TableType = MoveVector>
class RunPermPhi : public RunPermImpl<RunColsType, IntegratedMoveStructure, true, ExponentialSearch, MoveCols, MoveStructure, TableType> {
    using Base = RunPermImpl<RunColsType, IntegratedMoveStructure, true, ExponentialSearch, MoveCols, MoveStructure, TableType>;
public:
    using Base::Base;
    using Base::operator=;
    using Position = typename Base::Position;

    Position Phi(Position pos) {
        return Base::next(pos);
    }

    Position Phi(Position pos, ulint steps) {
        return Base::next(pos, steps);
    }

    ulint SA(Position pos) {
        return pos.idx;
    }
};

// Always use absolute positions for Phi
class MovePhi : public MovePermImpl<true, DEFAULT_EXPONENTIAL_SEARCH, MoveCols, MoveStructure, MoveVector> {
    using Base = MovePermImpl<true, DEFAULT_EXPONENTIAL_SEARCH, MoveCols, MoveStructure, MoveVector>;
public:
    using Base::Base;
    using Base::operator=;
    using Position = typename Base::Position;

    Position Phi(Position pos) {
        return Base::next(pos);
    }

    Position Phi(Position pos, ulint steps) {
        return Base::next(pos, steps);
    }

    ulint SA(Position pos) {
        return pos.idx;
    }
};


#endif /* end of include guard: _RUNPERM_PHI_HPP */