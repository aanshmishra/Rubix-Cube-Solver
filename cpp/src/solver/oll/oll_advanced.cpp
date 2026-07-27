#include "oll_base.h"
#include "../masks.h"
#include "../../core/bidirectional_bfs.h"
#include "../../core/validation.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Advanced OLL - one search, every piece oriented at once ("1-look").
//
// The mask is the whole orientation vector plus the positions of the F2L
// pieces, with the last layer's permutation deliberately omitted. So the goal
// projection reads "every sticker oriented, first two layers home, U layer
// arranged however it likes", and the backward search from a solved cube
// reaches it under any U-layer permutation. That works because an orientation
// delta depends only on the move and the slot, never on which piece occupies
// the slot - the property that makes this mask congruent.
//
// Restricting to <U,R,F> keeps the branching at 9 and guarantees the finished
// first two layers survive untouched.
//
// There is no 57-case table here and none is needed: the search rediscovers an
// optimal sequence for whichever case it is handed.
// ---------------------------------------------------------------------------

class OLLAdvanced : public OLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        if (isOLLSolved(state)) return {};

        OrientationMask mask;
        mask.include_corner_orient = true;

        auto r = bidirectionalBFS<Key128>(state, CubeState::solved(), mask,
                                          /*max_depth=*/7, MOVES_URF, 9,
                                          /*max_nodes=*/2000000);
        return r.found ? r.moves : std::vector<Move>{};
    }

    const char* name() const override { return "Advanced OLL (1-look)"; }
    int numCases() const override { return 57; }
};

OLLStrategyPtr createOLLAdvanced() {
    return OLLStrategyPtr(new OLLAdvanced());
}

} // namespace rubiks
