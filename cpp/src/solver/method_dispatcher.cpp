#include "method_dispatcher.h"
#include "cross.h"
#include "../core/validation.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// MethodConfig
// ---------------------------------------------------------------------------

std::string MethodConfig::getMethodName() const {
    int advanced_count = (f2l_advanced ? 1 : 0) + (oll_advanced ? 1 : 0) + (pll_advanced ? 1 : 0);

    if (advanced_count == 3) {
        return "Full CFOP";
    } else if (advanced_count == 0) {
        return "Beginner Layer-by-Layer";
    } else {
        std::string name = "Hybrid (";
        if (f2l_advanced) {
            name += "Adv-F2L";
        } else {
            name += "Beg-F2L";
        }
        name += " / ";
        if (oll_advanced) {
            name += "Adv-OLL";
        } else {
            name += "Beg-OLL";
        }
        name += " / ";
        if (pll_advanced) {
            name += "Adv-PLL";
        } else {
            name += "Beg-PLL";
        }
        name += ")";
        return name;
    }
}

// ---------------------------------------------------------------------------
// MethodDispatcher
// ---------------------------------------------------------------------------

MethodDispatcher::MethodDispatcher(const MethodConfig& config) : config_(config) {}

SolutionResult MethodDispatcher::solve(const CubeState& scramble) {
    SolutionResult result;
    result.config = config_;

    // Validate state
    auto validation = validateState(scramble);
    if (!validation.valid) {
        result.success = false;
        result.error_message = validation.error_message;
        return result;
    }

    if (isSolved(scramble)) {
        result.success = true;
        return result;
    }

    // Stage 1: Cross
    result.cross_moves = solveCross(scramble);

    CubeState after_cross = scramble;
    after_cross.applyMoves(result.cross_moves);

    if (!isCrossSolvedFast(after_cross)) {
        result.success = false;
        result.error_message = "Failed to solve cross";
        return result;
    }

    if (isSolved(after_cross)) {
        result.success = true;
        return result;
    }

    // Stage 2: F2L
    {
        auto f2l = config_.f2l_advanced ? createF2LAdvanced() : createF2LBeginner();
        result.f2l_moves = f2l->solve(after_cross);
    }

    CubeState after_f2l = after_cross;
    after_f2l.applyMoves(result.f2l_moves);

    if (!isF2LSolved(after_f2l)) {
        result.success = false;
        result.error_message = "Failed to solve F2L";
        return result;
    }

    if (isSolved(after_f2l)) {
        result.success = true;
        return result;
    }

    // Stage 3: OLL
    {
        auto oll = config_.oll_advanced ? createOLLAdvanced() : createOLLBeginner();
        result.oll_moves = oll->solve(after_f2l);
    }

    CubeState after_oll = after_f2l;
    after_oll.applyMoves(result.oll_moves);

    if (!isOLLSolved(after_oll) || !isF2LSolved(after_oll)) {
        result.success = false;
        result.error_message = "Failed to orient the last layer";
        return result;
    }

    if (isSolved(after_oll)) {
        result.success = true;
        return result;
    }

    // Stage 4: PLL
    {
        auto pll = config_.pll_advanced ? createPLLAdvanced() : createPLLBeginner();
        result.pll_moves = pll->solve(after_oll);
    }

    CubeState after_pll = after_oll;
    after_pll.applyMoves(result.pll_moves);

    if (!isSolved(after_pll)) {
        result.success = false;
        result.error_message = "Failed to completely solve cube";
        return result;
    }

    result.success = true;
    return result;
}

} // namespace rubiks
