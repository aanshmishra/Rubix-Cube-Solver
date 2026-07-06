#include "f2l_base.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Beginner F2L: Decomposed sub-steps
//
// Instead of 1-look pair insertion, we break it down:
// 1. Insert corner to first layer (below its slot)
// 2. Insert edge to middle layer
// This is easier to follow but requires more moves.
// ---------------------------------------------------------------------------

class F2LBeginner : public F2LStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        CubeState working = state;
        std::vector<Move> all_moves;

        // Solve corners first, then edges
        const uint8_t corners[4] = {4, 5, 6, 7}; // DFR, DLF, DBL, DRB
        const uint8_t edges[4] = {8, 9, 10, 11};  // FR, FL, BL, BR

        // Step 1: Solve each corner
        for (int i = 0; i < 4; ++i) {
            std::vector<Move> moves = solveCorner(working, corners[i]);
            working.applyMoves(moves);
            all_moves.insert(all_moves.end(), moves.begin(), moves.end());
        }

        // Step 2: Solve each middle layer edge
        for (int i = 0; i < 4; ++i) {
            std::vector<Move> moves = solveMiddleEdge(working, edges[i]);
            working.applyMoves(moves);
            all_moves.insert(all_moves.end(), moves.begin(), moves.end());
        }

        return all_moves;
    }

    const char* name() const override { return "Beginner F2L"; }

private:
    std::vector<Move> solveCorner(CubeState state, uint8_t corner_idx) {
        if (isCornerSolved(state, corner_idx)) return {};
        
        const Move MOVES[11] = {
            Move::U, Move::U2, Move::Up,
            Move::R, Move::Rp,
            Move::F, Move::Fp,
            Move::L, Move::Lp,
            Move::B, Move::Bp,
        };

        BidiBFSConfig config;
        config.moves = MOVES;
        config.num_moves = 11;
        config.hash_fn = [this, corner_idx](const CubeState& s) -> uint64_t {
            return this->hashCorner(s, corner_idx);
        };
        config.max_depth = 12;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    std::vector<Move> solveMiddleEdge(CubeState state, uint8_t edge_idx) {
        if (isEdgeSolved(state, edge_idx)) return {};
        
        const Move MOVES[11] = {
            Move::U, Move::U2, Move::Up,
            Move::R, Move::Rp,
            Move::F, Move::Fp,
            Move::L, Move::Lp,
            Move::B, Move::Bp,
        };

        BidiBFSConfig config;
        config.moves = MOVES;
        config.num_moves = 11;
        config.hash_fn = [this, edge_idx](const CubeState& s) -> uint64_t {
            return this->hashEdge(s, edge_idx);
        };
        config.max_depth = 12;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    bool isCornerSolved(const CubeState& state, uint8_t corner_idx) {
        return state.corner_perm[corner_idx] == corner_idx &&
               state.corner_orient[corner_idx] == 0;
    }

    bool isEdgeSolved(const CubeState& state, uint8_t edge_idx) {
        return state.edge_perm[edge_idx] == edge_idx &&
               state.edge_orient[edge_idx] == 0;
    }

    uint32_t hashCorner(const CubeState& state, uint8_t corner_idx) {
        for (int i = 0; i < 8; ++i) {
            if (state.corner_perm[i] == corner_idx) {
                return i * 3 + state.corner_orient[i];
            }
        }
        return 0;
    }

    uint32_t hashEdge(const CubeState& state, uint8_t edge_idx) {
        for (int i = 0; i < 12; ++i) {
            if (state.edge_perm[i] == edge_idx) {
                return i * 2 + state.edge_orient[i] + 1000;
            }
        }
        return 0;
    }
};

F2LStrategyPtr createF2LBeginner() {
    return std::make_unique<F2LBeginner>();
}

} // namespace rubiks
