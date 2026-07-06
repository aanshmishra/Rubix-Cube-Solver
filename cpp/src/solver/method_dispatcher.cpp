#include "method_dispatcher.h"
#include "cross.h"
#include "../core/validation.h"
#include <sstream>

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
// SolutionResult
// ---------------------------------------------------------------------------

std::vector<Move> SolutionResult::allMoves() const {
    std::vector<Move> all;
    all.insert(all.end(), cross_moves.begin(), cross_moves.end());
    all.insert(all.end(), f2l_moves.begin(), f2l_moves.end());
    all.insert(all.end(), oll_moves.begin(), oll_moves.end());
    all.insert(all.end(), pll_moves.begin(), pll_moves.end());
    return all;
}

std::string SolutionResult::toNotation() const {
    std::stringstream ss;

    if (!cross_moves.empty()) {
        ss << "[Cross] ";
        for (size_t i = 0; i < cross_moves.size(); ++i) {
            if (i > 0) ss << " ";
            ss << moveToString(cross_moves[i]);
        }
        ss << "\n";
    }

    if (!f2l_moves.empty()) {
        ss << "[F2L] ";
        for (size_t i = 0; i < f2l_moves.size(); ++i) {
            if (i > 0) ss << " ";
            ss << moveToString(f2l_moves[i]);
        }
        ss << "\n";
    }

    if (!oll_moves.empty()) {
        ss << "[OLL] ";
        for (size_t i = 0; i < oll_moves.size(); ++i) {
            if (i > 0) ss << " ";
            ss << moveToString(oll_moves[i]);
        }
        ss << "\n";
    }

    if (!pll_moves.empty()) {
        ss << "[PLL] ";
        for (size_t i = 0; i < pll_moves.size(); ++i) {
            if (i > 0) ss << " ";
            ss << moveToString(pll_moves[i]);
        }
        ss << "\n";
    }

    return ss.str();
}

int SolutionResult::totalMoves() const {
    return static_cast<int>(cross_moves.size() + f2l_moves.size() + oll_moves.size() + pll_moves.size());
}

// ---------------------------------------------------------------------------
// MethodDispatcher
// ---------------------------------------------------------------------------

MethodDispatcher::MethodDispatcher(const MethodConfig& config) : config_(config) {}

void MethodDispatcher::setConfig(const MethodConfig& config) {
    config_ = config;
}

std::string MethodDispatcher::getMethodName() const {
    return config_.getMethodName();
}

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
