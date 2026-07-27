#pragma once

#include "../../core/cube_state.h"
#include <vector>
#include <memory>

namespace rubiks {

// Strategy interface for PLL (Permutation of Last Layer)
class PLLStrategy {
public:
    virtual ~PLLStrategy() = default;

    // Solve PLL: permute all last-layer pieces correctly.
    // Precondition: OLL is solved (all pieces oriented).
    // Postcondition: Entire cube is solved.
    virtual std::vector<Move> solve(const CubeState& state) = 0;

    virtual const char* name() const = 0;

    virtual int numCases() const = 0;
};

using PLLStrategyPtr = std::unique_ptr<PLLStrategy>;

// Two-look permutation: corners first (U-edge order left free), then the
// remaining edges. Shared because the advanced strategy falls back to it when
// its single combined search runs out of depth.
std::vector<Move> solvePLLTwoLook(const CubeState& state);

// Factory functions
PLLStrategyPtr createPLLAdvanced();
PLLStrategyPtr createPLLBeginner();

} // namespace rubiks
