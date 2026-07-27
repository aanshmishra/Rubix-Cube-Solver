#pragma once

#include "../core/cube_state.h"
#include "f2l/f2l_base.h"
#include "oll/oll_base.h"
#include "pll/pll_base.h"
#include <vector>
#include <memory>
#include <string>

namespace rubiks {

// Toggle configuration for adaptive solving
struct MethodConfig {
    bool f2l_advanced = true;  // true = advanced 1-look, false = beginner decomposed
    bool oll_advanced = true;  // true = 57-case full OLL, false = 2-step
    bool pll_advanced = true;  // true = 21-case full PLL, false = 2-step

    // Default: full CFOP (all advanced)
    static MethodConfig cfop() {
        return MethodConfig{true, true, true};
    }

    // Full beginner method
    static MethodConfig beginner() {
        return MethodConfig{false, false, false};
    }

    // Get a human-readable method name
    std::string getMethodName() const;
};

// Solution result with stage breakdown
struct SolutionResult {
    std::vector<Move> cross_moves;
    std::vector<Move> f2l_moves;
    std::vector<Move> oll_moves;
    std::vector<Move> pll_moves;
    MethodConfig config;
    bool success;
    std::string error_message;
};

// Main solver dispatcher
class MethodDispatcher {
public:
    MethodDispatcher(const MethodConfig& config = MethodConfig::cfop());

    // Solve the cube using the configured method
    SolutionResult solve(const CubeState& scramble);

private:
    MethodConfig config_;
};

} // namespace rubiks
