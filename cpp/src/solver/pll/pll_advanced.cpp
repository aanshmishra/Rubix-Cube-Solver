#include "pll_base.h"
#include "../masks.h"
#include "../../core/bidirectional_bfs.h"
#include "../../core/validation.h"

namespace rubiks {

// Depth of the one-look attempt, per side. Measured over the harness's fixed
// scrambles (native, -O2):
//     depth 6 -> 105 ms/solve, 57.7 moves, one-look lands ~1 time in 3
//     depth 7 -> 471 ms/solve, 55.4 moves, one-look lands ~1 time in 2
// Depth 7 buys about two moves for four times the work, because a *miss* has
// to exhaust the whole frontier before the two-look fallback even starts. 6 is
// the better default; raise it if you would rather have the shorter solution.
int PLL_ONE_LOOK_DEPTH = 6;

// Hit counters - worth keeping for a search-based solver, since "how often does
// the cheap path work" is the number that drives the tuning above.
long g_pll_one_look_hits = 0;
long g_pll_one_look_misses = 0;

// ---------------------------------------------------------------------------
// Advanced PLL - one search for the whole permutation ("1-look").
//
// Corners and edges are placed together by a single meet-in-the-middle search
// against the exact solved state, restricted to <U,R,F>. Where it succeeds it
// beats the two-look route, because splitting the goal forces the corner stage
// to commit to a sequence chosen without regard for what it costs the edges.
//
// Some cases (the G-perm family in particular) sit deeper than 7 + 7 in this
// subgroup, and pushing the depth further makes the frontier explode. Rather
// than gamble, an exhausted search falls back to the two-look decomposition,
// which always closes. That fallback is a search too - still no case table.
// ---------------------------------------------------------------------------

class PLLAdvanced : public PLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        if (isSolved(state)) return {};

        auto one_look = bidirectionalBFS<Key128>(state, CubeState::solved(), FullStateMask{},
                                                 /*max_depth=*/PLL_ONE_LOOK_DEPTH, MOVES_URF, 9,
                                                 /*max_nodes=*/2000000);
        if (one_look.found) { ++g_pll_one_look_hits; return one_look.moves; }

        ++g_pll_one_look_misses;
        return solvePLLTwoLook(state);
    }

    const char* name() const override { return "Advanced PLL (1-look)"; }
    int numCases() const override { return 21; }
};

PLLStrategyPtr createPLLAdvanced() {
    return PLLStrategyPtr(new PLLAdvanced());
}

} // namespace rubiks
