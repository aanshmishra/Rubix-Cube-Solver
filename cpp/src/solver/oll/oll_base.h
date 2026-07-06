#pragma once

#include "../../core/cube_state.h"
#include <vector>
#include <memory>

namespace rubiks {

// Strategy interface for OLL (Orientation of Last Layer)
class OLLStrategy {
public:
    virtual ~OLLStrategy() = default;

    // Solve OLL: orient all last-layer pieces correctly.
    // Precondition: F2L is solved, last layer pieces may be unoriented.
    // Postcondition: All U-face stickers on U face.
    virtual std::vector<Move> solve(const CubeState& state) = 0;

    virtual const char* name() const = 0;

    // How many cases this strategy handles
    virtual int numCases() const = 0;
};

using OLLStrategyPtr = std::unique_ptr<OLLStrategy>;

// Factory functions
OLLStrategyPtr createOLLAdvanced();
OLLStrategyPtr createOLLBeginner();

} // namespace rubiks
