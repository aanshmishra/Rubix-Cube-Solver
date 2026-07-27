#pragma once

#include "cube_state.h"
#include <string>
#include <vector>

namespace rubiks {

// Validation result
struct ValidationResult {
    bool valid;
    std::string error_message;
    
    ValidationResult(bool v = true, const std::string& msg = "")
        : valid(v), error_message(msg) {}
};

// Validate that a cubie-level state is physically reachable (solvable).
// Checks:
//   1. Corner orientation sum is divisible by 3
//   2. Edge orientation sum is divisible by 2
//   3. Corner permutation parity matches edge permutation parity
ValidationResult validateState(const CubeState& state);

// Check if cube is solved
bool isSolved(const CubeState& state);

// Check if cross is solved (for a specific face)
bool isCrossSolved(const CubeState& state, Face face);

// Check if F2L is solved
bool isF2LSolved(const CubeState& state);

// Check if OLL is solved (last layer oriented)
bool isOLLSolved(const CubeState& state);

} // namespace rubiks
