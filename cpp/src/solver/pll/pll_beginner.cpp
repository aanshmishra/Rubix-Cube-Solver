#include "pll_base.h"
#include "../masks.h"
#include "../../core/bidirectional_bfs.h"
#include "../../core/validation.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Two-look permutation, used directly by the beginner strategy and as the
// advanced strategy's fallback.
//
//   Look 1: place the corners. The mask omits the U-layer edge order, so the
//           search is free to churn the edges while it works - a much easier
//           goal to reach than "everything at once".
//   Look 2: permute the four remaining U edges. That is an EPLL state, always
//           solvable inside the small <U,R> subgroup, so the search can afford
//           to go deeper over a branching factor of only 6.
// ---------------------------------------------------------------------------
std::vector<Move> solvePLLTwoLook(const CubeState& state) {
    CubeState cur = state;
    std::vector<Move> out;

    auto corners = bidirectionalBFS<Key128>(cur, CubeState::solved(), CornerPermMask{},
                                            /*max_depth=*/7, MOVES_URF, 9,
                                            /*max_nodes=*/2000000);
    if (!corners.found) return out;
    cur.applyMoves(corners.moves);
    out.insert(out.end(), corners.moves.begin(), corners.moves.end());

    auto edges = bidirectionalBFS<Key128>(cur, CubeState::solved(), FullStateMask{},
                                          /*max_depth=*/9, MOVES_UR, 6,
                                          /*max_nodes=*/2000000);
    if (!edges.found) return out;
    cur.applyMoves(edges.moves);
    out.insert(out.end(), edges.moves.begin(), edges.moves.end());

    return out;
}

class PLLBeginner : public PLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        if (isSolved(state)) return {};
        return solvePLLTwoLook(state);
    }

    const char* name() const override { return "Beginner PLL (2-look)"; }
    int numCases() const override { return 2; }
};

PLLStrategyPtr createPLLBeginner() {
    return PLLStrategyPtr(new PLLBeginner());
}

} // namespace rubiks
