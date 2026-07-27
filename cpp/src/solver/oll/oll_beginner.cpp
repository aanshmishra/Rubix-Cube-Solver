#include "oll_base.h"
#include "../masks.h"
#include "../../core/bidirectional_bfs.h"
#include "../../core/validation.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Beginner OLL - the classic 2-look split, done by search.
//
//   Look 1: flip the last-layer edges only (the "cross on top" step). The mask
//           drops corner orientation entirely, so corners may be left twisted.
//   Look 2: orient the corners, with edge orientation back in the mask so the
//           first look cannot be undone.
//
// Two shallow searches instead of one deeper one: faster to find, longer to
// execute. Same BFS, same absence of any case table.
// ---------------------------------------------------------------------------

class OLLBeginner : public OLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        if (isOLLSolved(state)) return {};

        CubeState cur = state;
        std::vector<Move> out;

        OrientationMask edges_only;
        edges_only.include_corner_orient = false;
        run(cur, out, edges_only, 6);

        OrientationMask everything;
        everything.include_corner_orient = true;
        run(cur, out, everything, 7);

        return out;
    }

    const char* name() const override { return "Beginner OLL (2-look)"; }
    int numCases() const override { return 2; }

private:
    template <typename MaskFn>
    static void run(CubeState& cur, std::vector<Move>& out, MaskFn mask, int depth) {
        auto r = bidirectionalBFS<Key128>(cur, CubeState::solved(), mask,
                                          depth, MOVES_URF, 9,
                                          /*max_nodes=*/2000000);
        if (!r.found) return;
        cur.applyMoves(r.moves);
        out.insert(out.end(), r.moves.begin(), r.moves.end());
    }
};

OLLStrategyPtr createOLLBeginner() {
    return OLLStrategyPtr(new OLLBeginner());
}

} // namespace rubiks
