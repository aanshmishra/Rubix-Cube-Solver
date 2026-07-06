#include "pll_base.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Beginner PLL: Two-step approach
//
// Step 1: Permute last-layer corners (get them in correct positions)
// Step 2: Permute last-layer edges
//
// Uses simplified BFS for each sub-step.
// ---------------------------------------------------------------------------

class PLLBeginner : public PLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        CubeState working = state;
        std::vector<Move> all_moves;

        // Step 1: Permute U corners to correct positions
        std::vector<Move> corner_moves = permuteCorners(working);
        working.applyMoves(corner_moves);
        all_moves.insert(all_moves.end(), corner_moves.begin(), corner_moves.end());

        // Step 2: Permute U edges to correct positions
        std::vector<Move> edge_moves = permuteEdges(working);
        working.applyMoves(edge_moves);
        all_moves.insert(all_moves.end(), edge_moves.begin(), edge_moves.end());

        return all_moves;
    }

    const char* name() const override { return "Beginner PLL (2-step)"; }
    int numCases() const override { return 2; }

private:
    std::vector<Move> permuteCorners(CubeState state) {
        if (areCornersPermuted(state)) return {};

        const Move MOVES[14] = {
            Move::R, Move::Rp, Move::R2,
            Move::L, Move::Lp, Move::L2,
            Move::F, Move::Fp, Move::F2,
            Move::B, Move::Bp,
            Move::U, Move::Up, Move::U2,
        };

        BidiBFSConfig config;
        config.moves = MOVES;
        config.num_moves = 14;
        config.hash_fn = [this](const CubeState& s) -> uint64_t {
            return this->hashCornerPerm(s);
        };
        config.max_depth = 14;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    std::vector<Move> permuteEdges(CubeState state) {
        if (areEdgesPermuted(state)) return {};

        const Move MOVES[14] = {
            Move::R, Move::Rp, Move::R2,
            Move::L, Move::Lp, Move::L2,
            Move::F, Move::Fp, Move::F2,
            Move::B, Move::Bp,
            Move::U, Move::Up, Move::U2,
        };

        BidiBFSConfig config;
        config.moves = MOVES;
        config.num_moves = 14;
        config.hash_fn = [this](const CubeState& s) -> uint64_t {
            return this->hashEdgePerm(s);
        };
        config.max_depth = 14;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    bool areCornersPermuted(const CubeState& state) {
        const uint8_t u_corners[4] = {0, 1, 2, 3};
        for (uint8_t c : u_corners) {
            if (state.corner_perm[c] != c) return false;
        }
        return true;
    }

    bool areEdgesPermuted(const CubeState& state) {
        const uint8_t u_edges[4] = {0, 1, 2, 3};
        for (uint8_t e : u_edges) {
            if (state.edge_perm[e] != e) return false;
        }
        return true;
    }

    uint64_t hashCornerPerm(const CubeState& state) {
        uint64_t h = 0;
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
            h = h * 8 + state.corner_perm[i];
        }
        return h;
    }

    uint64_t hashEdgePerm(const CubeState& state) {
        uint64_t h = 0;
        for (uint8_t i = 0; i < NUM_EDGES; ++i) {
            h = h * 12 + state.edge_perm[i];
        }
        return h;
    }
};

PLLStrategyPtr createPLLBeginner() {
    return std::make_unique<PLLBeginner>();
}

} // namespace rubiks
