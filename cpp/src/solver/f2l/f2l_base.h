#pragma once

#include "../../core/cube_state.h"
#include <vector>
#include <memory>

namespace rubiks {

// Strategy interface for F2L (First Two Layers)
class F2LStrategy {
public:
    virtual ~F2LStrategy() = default;

    // Solve F2L given a cube state with cross already solved.
    // Returns move sequence for completing F2L.
    virtual std::vector<Move> solve(const CubeState& state) = 0;

    // Get display name for this strategy
    virtual const char* name() const = 0;

    // Get the number of F2L pairs this strategy handles
    virtual int numPairs() const { return 4; }
};

using F2LStrategyPtr = std::unique_ptr<F2LStrategy>;

// Factory functions
F2LStrategyPtr createF2LAdvanced();
F2LStrategyPtr createF2LBeginner();

} // namespace rubiks
