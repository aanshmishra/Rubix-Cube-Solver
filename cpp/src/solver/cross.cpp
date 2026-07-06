#include "cross.h"
#include "../core/validation.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Cross state encoding
//
// We only care about 4 edges: DF(5), DR(4), DB(7), DL(6)
// Each edge has:
//   - position (0..3): which cross slot it's in (DF, DR, DB, DL)
//   - orientation (0..1): correct or flipped
//
// We encode: position * 2 + orientation for each of 4 edges
// Total states: 8! * 2^4 / (some sym) = manageable
// ---------------------------------------------------------------------------

static constexpr uint8_t CROSS_EDGE_INDICES[4] = {5, 4, 7, 6}; // DF, DR, DB, DL

// Encode just the cross-relevant state into a compact integer
static uint32_t encodeCrossState(const CubeState& state) {
    uint32_t encoded = 0;
    // Track the locations and orientations of the 4 cross pieces
    for (int i = 0; i < 4; ++i) {
        uint8_t target_piece = CROSS_EDGE_INDICES[i];
        for (int j = 0; j < 12; ++j) {
            if (state.edge_perm[j] == target_piece) {
                encoded = encoded * 24 + j * 2 + state.edge_orient[j];
                break;
            }
        }
    }
    return encoded;
}

// Check if cross is solved
bool isCrossSolvedFast(const CubeState& state) {
    for (int i = 0; i < 4; ++i) {
        uint8_t edge_idx = CROSS_EDGE_INDICES[i];
        if (state.edge_perm[edge_idx] != edge_idx || state.edge_orient[edge_idx] != 0) {
            return false;
        }
    }
    return true;
}

// Apply only cross-relevant tracking (for BFS state expansion)
static CubeState applyMoveCross(const CubeState& state, Move move) {
    CubeState next = state;
    next.applyMove(move);
    return next;
}

// Simplified: use bidirectional BFS with cross-only encoding
std::vector<Move> solveCross(const CubeState& state) {
    if (isCrossSolvedFast(state)) {
        return {};
    }

    const Move ALL_MOVES[18] = {
        Move::U, Move::U2, Move::Up,
        Move::R, Move::R2, Move::Rp,
        Move::F, Move::F2, Move::Fp,
        Move::D, Move::D2, Move::Dp,
        Move::L, Move::L2, Move::Lp,
        Move::B, Move::B2, Move::Bp,
    };

    BidiBFSConfig config;
    config.moves = ALL_MOVES;
    config.num_moves = 18;
    config.hash_fn = encodeCrossState;
    config.max_depth = 8;
    config.max_iterations = 50000;

    return bidirectionalBFS(state, CubeState::solved(), config);
}

} // namespace rubiks
