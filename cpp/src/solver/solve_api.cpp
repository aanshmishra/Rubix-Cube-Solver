#include "solve_api.h"
#include "method_dispatcher.h"
#include "../core/facelet_converter.h"
#include "../core/validation.h"

namespace rubiks {

static std::string fail(SolveOutput& out, const std::string& msg) {
    out.success = false;
    out.errorMessage = msg;
    return msg;
}

// Collapses a same-face pair straddling two stages, e.g. cross ending "... R2"
// against F2L opening "R ...". The merged turn is kept in the earlier stage so
// the per-stage counts the UI shows still add up to the total.
static void mergeSeam(std::vector<Move>& before, std::vector<Move>& after) {
    while (!before.empty() && !after.empty() &&
           moveFace(before.back()) == moveFace(after.front())) {
        std::vector<Move> pair{before.back(), after.front()};
        before.pop_back();
        after.erase(after.begin());

        const std::vector<Move> merged = simplifyMoves(pair);
        before.insert(before.end(), merged.begin(), merged.end());

        // A surviving turn has the same face as the one just consumed, and the
        // next move along cannot share it (each stage is already simplified),
        // so only a full cancellation can expose another mergeable pair.
        if (!merged.empty()) break;
    }
}

SolveOutput solveFacelets(const std::string& scrambled,
                          const std::string& target,
                          bool f2l_advanced,
                          bool oll_advanced,
                          bool pll_advanced) {
    SolveOutput out;

    MethodConfig config{f2l_advanced, oll_advanced, pll_advanced};
    out.methodName = config.getMethodName();

    CubieEncoder encoder(target);
    if (!encoder.valid()) {
        fail(out, "Invalid colour scheme: the target facelets are not a solved cube");
        return out;
    }

    CubeState state;
    if (!encoder.encode(scrambled, state)) {
        fail(out, "Cube state has invalid stickers (no such physical cube)");
        return out;
    }

    MethodDispatcher dispatcher(config);
    SolutionResult result = dispatcher.solve(state);

    // Tidy the seams the searches cannot see past: each stage (and each pair or
    // look inside it) is optimal for its own subgoal, but nothing stops one
    // ending on R2 and the next opening on R.
    result.cross_moves = simplifyMoves(result.cross_moves);
    result.f2l_moves = simplifyMoves(result.f2l_moves);
    result.oll_moves = simplifyMoves(result.oll_moves);
    result.pll_moves = simplifyMoves(result.pll_moves);

    mergeSeam(result.cross_moves, result.f2l_moves);
    mergeSeam(result.f2l_moves, result.oll_moves);
    mergeSeam(result.oll_moves, result.pll_moves);

    out.crossMoves = movesToNotation(result.cross_moves);
    out.f2lMoves = movesToNotation(result.f2l_moves);
    out.ollMoves = movesToNotation(result.oll_moves);
    out.pllMoves = movesToNotation(result.pll_moves);
    out.success = result.success;
    out.errorMessage = result.error_message;

    return out;
}

} // namespace rubiks
