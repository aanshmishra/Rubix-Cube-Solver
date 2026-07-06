#include "f2l_base.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../../core/bidirectional_bfs.h"

namespace rubiks {

// ---------------------------------------------------------------------------
// Advanced F2L: Standard 1-look pair insertion
//
// Uses simplified pattern matching for common F2L cases.
// In a full implementation, this would have 41 case lookup tables.
// Here we use a simplified BFS approach for each slot.
// ---------------------------------------------------------------------------

class F2LAdvanced : public F2LStrategy {
public:
    std::vector<Move> solve(const CubeState& state) override {
        CubeState working = state;
        std::vector<Move> all_moves;

        // Solve each F2L slot: DFR, DLF, DBL, DRB
        const uint8_t slots[4][2] = {
            {4, 8},  // corner DFR(4), edge FR(8)
            {5, 9},  // corner DLF(5), edge FL(9)
            {6, 10}, // corner DBL(6), edge BL(10)
            {7, 11}, // corner DRB(7), edge BR(11)
        };

        for (int i = 0; i < 4; ++i) {
            std::vector<Move> slot_moves = solveSlot(working, slots[i][0], slots[i][1]);
            working.applyMoves(slot_moves);
            all_moves.insert(all_moves.end(), slot_moves.begin(), slot_moves.end());
        }

        return all_moves;
    }

    const char* name() const override { return "Advanced F2L"; }

private:
    std::vector<Move> solveSlot(CubeState state, uint8_t corner_idx, uint8_t edge_idx) {
        if (isSlotSolved(state, corner_idx, edge_idx)) return {};

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
        config.hash_fn = [this, corner_idx, edge_idx](const CubeState& s) -> uint64_t {
            return this->hashSlotState(s, corner_idx, edge_idx);
        };
        config.max_depth = 12;

        return bidirectionalBFS(state, CubeState::solved(), config);
    }

    bool isSlotSolved(const CubeState& state, uint8_t corner_idx, uint8_t edge_idx) {
        return state.corner_perm[corner_idx] == corner_idx &&
               state.corner_orient[corner_idx] == 0 &&
               state.edge_perm[edge_idx] == edge_idx &&
               state.edge_orient[edge_idx] == 0;
    }

    uint32_t hashSlotState(const CubeState& state, uint8_t corner_idx, uint8_t edge_idx) {
        // Hash just the relevant pieces by finding where they actually are
        uint32_t h = 0;
        for (int i = 0; i < 8; ++i) {
            if (state.corner_perm[i] == corner_idx) {
                h = h * 8 + i;
                h = h * 3 + state.corner_orient[i];
                break;
            }
        }
        for (int i = 0; i < 12; ++i) {
            if (state.edge_perm[i] == edge_idx) {
                h = h * 12 + i;
                h = h * 2 + state.edge_orient[i];
                break;
            }
        }
        return h;
    }
};

F2LStrategyPtr createF2LAdvanced() {
    return std::make_unique<F2LAdvanced>();
}

} // namespace rubiks
