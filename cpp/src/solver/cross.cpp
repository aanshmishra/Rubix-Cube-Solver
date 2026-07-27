#include "cross.h"
#include "masks.h"
#include "../core/bidirectional_bfs.h"

namespace rubiks {

bool isCrossSolvedFast(const CubeState& state) {
    for (uint8_t id : CROSS_EDGE_IDS) {
        if (state.edge_perm[id] != id || state.edge_orient[id] != 0) return false;
    }
    return true;
}

// The cross is the 4 D-layer edges and nothing else - 12P4 * 2^4 = 190080
// reachable projections, so a plain meet-in-the-middle over the full move set
// closes it instantly. Any cross is solvable in 8 moves, and 5 + 5 leaves room.
std::vector<Move> solveCross(const CubeState& state) {
    if (isCrossSolvedFast(state)) return {};

    TrackedPiecesMask mask;
    mask.edge_ids.assign(std::begin(CROSS_EDGE_IDS), std::end(CROSS_EDGE_IDS));

    auto r = bidirectionalBFS<Key64>(state, CubeState::solved(), mask,
                                     /*max_depth=*/5, MOVES_ALL, 18);
    return r.found ? r.moves : std::vector<Move>{};
}

} // namespace rubiks
