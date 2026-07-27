#include "f2l_base.h"
#include "../masks.h"
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Beginner F2L - decomposed, two searches per slot.
//
// Same machinery as the advanced strategy, smaller subgoals: seat the corner
// first, then bring the edge in without disturbing it. Each search is shallower
// (so it returns sooner) but the two optima do not compose into the pair
// optimum, which is precisely why the beginner route spends more moves. That
// decomposition is the whole difference between the two settings - neither
// consults a case table.
// ---------------------------------------------------------------------------

class F2LBeginner : public F2LStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        CubeState cur = state;
        std::vector<Move> out;

        TrackedPiecesMask mask;
        mask.edge_ids.assign(std::begin(CROSS_EDGE_IDS), std::end(CROSS_EDGE_IDS));

        for (int i = 0; i < 4; ++i) {
            // Step 1: seat the corner; the edge is still free to sit anywhere.
            mask.corner_ids.push_back(F2L_CORNER_IDS[i]);
            if (!step(cur, out, mask)) return out;

            // Step 2: insert the edge while holding the corner in place.
            mask.edge_ids.push_back(F2L_SLOT_EDGE_IDS[i]);
            if (!step(cur, out, mask)) return out;
        }

        return out;
    }

    const char* name() const override { return "Beginner F2L (corner then edge)"; }
    int numPairs() const override { return 4; }

private:
    static bool step(CubeState& cur, std::vector<Move>& out,
                     const TrackedPiecesMask& mask) {
        auto r = bidirectionalBFS<Key64>(cur, CubeState::solved(), mask,
                                         /*max_depth=*/7, MOVES_ALL, 18,
                                         /*max_nodes=*/2000000);
        if (!r.found) return false;
        cur.applyMoves(r.moves);
        out.insert(out.end(), r.moves.begin(), r.moves.end());
        return true;
    }
};

F2LStrategyPtr createF2LBeginner() {
    return F2LStrategyPtr(new F2LBeginner());
}

} // namespace rubiks
