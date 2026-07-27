#pragma once

#include "../core/bidirectional_bfs.h"
#include "../core/cube_state.h"
#include <vector>

namespace rubiks {

// Piece ids (a piece's id is its home slot).
//   cross edges  : DR 4, DF 5, DL 6, DB 7
//   F2L corners  : DFR 4, DLF 5, DBL 6, DRB 7
//   F2L slot edges: FR 8, FL 9, BL 10, BR 11
constexpr uint8_t CROSS_EDGE_IDS[4] = {4, 5, 6, 7};
constexpr uint8_t F2L_CORNER_IDS[4] = {4, 5, 6, 7};
constexpr uint8_t F2L_SLOT_EDGE_IDS[4] = {8, 9, 10, 11};
// Every non-U edge: the eight that must be home once F2L is finished.
constexpr uint8_t F2L_EDGE_IDS[8] = {4, 5, 6, 7, 8, 9, 10, 11};

inline uint8_t cornerSlotOf(const CubeState& s, uint8_t id) {
    for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
        if (s.corner_perm[i] == id) return i;
    }
    return 0;
}

inline uint8_t edgeSlotOf(const CubeState& s, uint8_t id) {
    for (uint8_t i = 0; i < NUM_EDGES; ++i) {
        if (s.edge_perm[i] == id) return i;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Masks. Every one of these is piece-based, hence congruent under any move.
// Values are packed in mixed radix rather than hashed, so distinct projections
// can never collide - a hash collision here would silently emit a wrong
// solution, which is not a risk worth taking to save a few bits.
// ---------------------------------------------------------------------------

// Where a set of tracked pieces sit and how they are twisted. Each entry is
// < 24 (corner: slot*3 + twist; edge: slot*2 + flip), so 12 entries fit in
// 24^12 < 2^55.
struct TrackedPiecesMask {
    std::vector<uint8_t> corner_ids;
    std::vector<uint8_t> edge_ids;

    Key64 operator()(const CubeState& s) const {
        Key64 k = 0;
        for (uint8_t id : corner_ids) {
            const uint8_t slot = cornerSlotOf(s, id);
            k = k * 24 + (slot * 3 + s.corner_orient[slot]);
        }
        for (uint8_t id : edge_ids) {
            const uint8_t slot = edgeSlotOf(s, id);
            k = k * 24 + (slot * 2 + s.edge_orient[slot]);
        }
        return k;
    }
};

// OLL: the whole orientation vector, plus where the F2L pieces sit. The
// last-layer *permutation* is deliberately left out, so the backward search
// from solved accepts any U-layer arrangement - that is what makes "oriented
// but not yet permuted" a reachable goal.
//
// include_corner_orient=false gives the first look of 2-look OLL (edges only).
// Each half below is annotated with its worst-case width, to show it stays
// inside 64 bits: 3^8 = 13 bits, 2^12 = 12 bits, 8^4 = 12 bits,
// 12^8 = 29 bits, 8^8 = 24 bits, 12^12 = 43 bits.
struct OrientationMask {
    bool include_corner_orient = true;

    Key128 operator()(const CubeState& s) const {
        Key128 k;
        if (include_corner_orient) {
            for (uint8_t i = 0; i < NUM_CORNERS; ++i) k.a = k.a * 3 + s.corner_orient[i];
        }
        for (uint8_t i = 0; i < NUM_EDGES; ++i) k.a = k.a * 2 + s.edge_orient[i];
        for (uint8_t id : F2L_CORNER_IDS) k.a = k.a * 8 + cornerSlotOf(s, id);  // <= 37 bits
        for (uint8_t id : F2L_EDGE_IDS) k.b = k.b * 12 + edgeSlotOf(s, id);     // <= 29 bits
        return k;
    }
};

// PLL first look: corners home and oriented, F2L edges home, U-edge order
// left free so the search may scramble it on the way.
struct CornerPermMask {
    Key128 operator()(const CubeState& s) const {
        Key128 k;
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) k.a = k.a * 8 + s.corner_perm[i];
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) k.a = k.a * 3 + s.corner_orient[i];  // <= 37
        for (uint8_t i = 0; i < NUM_EDGES; ++i) k.b = k.b * 2 + s.edge_orient[i];
        for (uint8_t id : F2L_EDGE_IDS) k.b = k.b * 12 + edgeSlotOf(s, id);            // <= 41
        return k;
    }
};

// The whole cube, exactly. Used for the final edge permutation and for the
// one-look PLL attempt.
struct FullStateMask {
    Key128 operator()(const CubeState& s) const {
        Key128 k;
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) k.a = k.a * 8 + s.corner_perm[i];
        for (uint8_t i = 0; i < NUM_CORNERS; ++i) k.a = k.a * 3 + s.corner_orient[i];  // <= 37
        for (uint8_t i = 0; i < NUM_EDGES; ++i) k.b = k.b * 12 + s.edge_perm[i];
        for (uint8_t i = 0; i < NUM_EDGES; ++i) k.b = k.b * 2 + s.edge_orient[i];      // <= 55
        return k;
    }
};

} // namespace rubiks
