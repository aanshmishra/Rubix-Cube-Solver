#include "oll_base.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Beginner OLL: Two-step approach
//
// Step 1: Orient last-layer edges (make cross on top)
// Step 2: Orient last-layer corners
//
// Uses simplified BFS for each sub-step.
// ---------------------------------------------------------------------------

class OLLBeginner : public OLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        CubeState working = state;
        std::vector<Move> all_moves;

        // Step 1: Orient U edges
        std::vector<Move> edge_moves = orientEdges(working);
        working.applyMoves(edge_moves);
        all_moves.insert(all_moves.end(), edge_moves.begin(), edge_moves.end());

        // Step 2: Orient U corners
        std::vector<Move> corner_moves = orientCorners(working);
        working.applyMoves(corner_moves);
        all_moves.insert(all_moves.end(), corner_moves.begin(), corner_moves.end());

        return all_moves;
    }

    const char* name() const override { return "Beginner OLL (2-step)"; }
    int numCases() const override { return 3; } // 3 edge cases + 7 corner cases simplified

private:
    std::vector<Move> orientEdges(CubeState state) {
        if (areEdgesOriented(state)) return {};

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
            return this->hashEdges(s);
        };
        config.max_depth = 12;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    std::vector<Move> orientCorners(CubeState state) {
        if (areCornersOriented(state)) return {};

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
            return this->hashCorners(s);
        };
        config.max_depth = 14;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    bool areEdgesOriented(const CubeState& state) {
        const uint8_t u_edges[4] = {0, 1, 2, 3};
        for (uint8_t e : u_edges) {
            if (state.edge_orient[e] != 0) return false;
        }
        return true;
    }

    bool areCornersOriented(const CubeState& state) {
        const uint8_t u_corners[4] = {0, 1, 2, 3};
        for (uint8_t c : u_corners) {
            if (state.corner_orient[c] != 0) return false;
        }
        return true;
    }

    uint64_t hashEdges(const CubeState& state) {
        uint64_t h = 0;
        const uint8_t u_edges[4] = {0, 1, 2, 3};
        for (uint8_t e : u_edges) {
            h = h * 2 + state.edge_orient[e];
        }
        return h;
    }

    uint64_t hashCorners(const CubeState& state) {
        uint64_t h = 0;
        const uint8_t u_corners[4] = {0, 1, 2, 3};
        for (uint8_t c : u_corners) {
            h = h * 3 + state.corner_orient[c];
        }
        return h;
    }
};

OLLStrategyPtr createOLLBeginner() {
    return std::make_unique<OLLBeginner>();
}

} // namespace rubiks
