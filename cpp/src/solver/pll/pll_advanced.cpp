#include "pll_base.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Advanced PLL: Full 21-case lookup
//
// Uses pattern matching on U-face permutation to identify the case,
// then applies the pre-computed algorithm.
// For this implementation, we use a simplified BFS.
// ---------------------------------------------------------------------------

class PLLAdvanced : public PLLStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        if (isSolved(state)) return {};

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
            return this->hashPermutation(s);
        };
        config.max_depth = 14;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    const char* name() const override { return "Advanced PLL (21 cases)"; }
    int numCases() const override { return 21; }

private:
    bool isSolved(const CubeState& state) {
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
            if (state.corner_perm[i] != i) return false;
            if (state.corner_orient[i] != 0) return false;
        }
        for (uint8_t i = 0; i < NUM_EDGES; ++i) {
            if (state.edge_perm[i] != i) return false;
            if (state.edge_orient[i] != 0) return false;
        }
        return true;
    }

    uint64_t hashPermutation(const CubeState& state) {
        // Hash corner and edge permutation of U layer
        uint64_t h = 0;
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
            h = h * 8 + state.corner_perm[i];
        }
        for (uint8_t i = 0; i < NUM_EDGES; ++i) {
            h = h * 12 + state.edge_perm[i];
        }
        return h;
    }
};

PLLStrategyPtr createPLLAdvanced() {
    return std::make_unique<PLLAdvanced>();
}

} // namespace rubiks
