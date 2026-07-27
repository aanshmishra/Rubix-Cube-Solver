#include "f2l_base.h"
#include "../masks.h"
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Advanced F2L - one search per pair ("1-look").
//
// Corner and edge of a slot are inserted together by a single meet-in-the-
// middle search. The mask carries every piece solved so far plus the current
// pair, so a later pair physically cannot wreck an earlier one: any sequence
// that displaced a tracked piece would change the mask and so would not reach
// the goal projection.
//
// No case table is consulted. "1-look" here means the pair is solved in one
// search, not that one of 41 memorised cases was recognised.
// ---------------------------------------------------------------------------

class F2LAdvanced : public F2LStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        CubeState cur = state;
        std::vector<Move> out;

        TrackedPiecesMask mask;
        mask.edge_ids.assign(std::begin(CROSS_EDGE_IDS), std::end(CROSS_EDGE_IDS));

        for (int i = 0; i < 4; ++i) {
            mask.corner_ids.push_back(F2L_CORNER_IDS[i]);
            mask.edge_ids.push_back(F2L_SLOT_EDGE_IDS[i]);

            // A pair always fits in 11 moves; 7 + 7 leaves margin.
            auto r = bidirectionalBFS<Key64>(cur, CubeState::solved(), mask,
                                             /*max_depth=*/7, MOVES_ALL, 18,
                                             /*max_nodes=*/2000000);
            if (!r.found) return out;  // dispatcher reports the failure

            cur.applyMoves(r.moves);
            out.insert(out.end(), r.moves.begin(), r.moves.end());
        }

        return out;
    }

    const char* name() const override { return "Advanced F2L (1-look pairs)"; }
    int numPairs() const override { return 4; }
};

F2LStrategyPtr createF2LAdvanced() {
    return F2LStrategyPtr(new F2LAdvanced());
}

} // namespace rubiks
