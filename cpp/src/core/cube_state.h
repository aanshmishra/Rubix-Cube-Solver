#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Cubie-level representation of a 3x3 Rubik's cube.
//
// 8 corners: each has a position (0..7) and an orientation (0..2).
// 12 edges: each has a position (0..11) and an orientation (0..1).
//
// CORNER slots (solved positions):
//   0: URF  1: UFL  2: ULB  3: UBR  4: DFR  5: DLF  6: DBL  7: DRB
// EDGE slots (solved positions):
//   0: UR   1: UF   2: UL   3: UB   4: DR   5: DF   6: DL   7: DB
//   8: FR   9: FL  10: BL  11: BR
//
// This slot numbering matches src/lib/solver/cubieCoords.ts exactly, and the
// move tables in cube_state.cpp are generated from the TypeScript engine, so
// the two implementations agree move-for-move by construction.
// ---------------------------------------------------------------------------

namespace rubiks {

// Face indices. Ordering matters: opposite(f) == (f + 3) % 6.
enum class Face : uint8_t {
    U = 0, R = 1, F = 2, D = 3, L = 4, B = 5,
    COUNT = 6
};

// 18 moves: 6 faces x 3 turn amounts, grouped so that move / 3 == face.
enum class Move : uint8_t {
    U = 0, U2 = 1, Up = 2,
    R = 3, R2 = 4, Rp = 5,
    F = 6, F2 = 7, Fp = 8,
    D = 9, D2 = 10, Dp = 11,
    L = 12, L2 = 13, Lp = 14,
    B = 15, B2 = 16, Bp = 17,
    COUNT = 18, NONE = 255
};

constexpr uint8_t NUM_CORNERS = 8;
constexpr uint8_t NUM_EDGES = 12;
constexpr uint8_t NUM_FACES = 6;
constexpr uint8_t NUM_MOVES = 18;

struct CubeState {
    // cp[i] = which corner piece sits at slot i
    std::array<uint8_t, NUM_CORNERS> corner_perm;
    // co[i] = clockwise twist (0..2) of the piece at slot i
    std::array<uint8_t, NUM_CORNERS> corner_orient;
    std::array<uint8_t, NUM_EDGES> edge_perm;
    std::array<uint8_t, NUM_EDGES> edge_orient;

    CubeState();

    static CubeState solved();

    void applyMove(Move move);
    void applyMoves(const std::vector<Move>& moves);
    void applyNotation(const std::string& notation);

    bool operator==(const CubeState& other) const;
    bool operator!=(const CubeState& other) const { return !(*this == other); }
};

// Move utilities
Move moveFromString(const std::string& s);
std::string moveToString(Move m);
Move inverseMove(Move m);
Face moveFace(Move m);

std::vector<Move> parseNotation(const std::string& notation);
std::string movesToNotation(const std::vector<Move>& moves);

// Collapses runs of same-face turns (R R -> R2, R2 R2 -> nothing, R U U' R -> R2).
// Move pruning only applies inside a single search, so consecutive stages can
// still meet at a seam like "... R2 | R ..."; this cleans that up. Purely a
// rewrite of an already-correct sequence - it never changes what the cube does.
std::vector<Move> simplifyMoves(const std::vector<Move>& moves);

// Move sets available to the searches. The last-layer stages use the <U,R,F>
// and <U,R> subgroups: a smaller branching factor, and moves that never touch
// the first two layers cannot disturb them.
extern const Move MOVES_ALL[18];
extern const Move MOVES_URF[9];
extern const Move MOVES_UR[6];

// ---------------------------------------------------------------------------
// Search move-set pruning.
//
// Two reductions, both sound (they never remove every optimal solution):
//   1. Never turn the same face twice in a row - X then X is always expressible
//      as a single turn of X.
//   2. Opposite faces commute (U D == D U), so for such a pair only allow the
//      lower face index second. This halves the duplicated orderings.
// ---------------------------------------------------------------------------
inline bool isRedundantAfter(Move candidate, Move previous) {
    if (previous == Move::NONE) return false;
    const uint8_t cf = static_cast<uint8_t>(moveFace(candidate));
    const uint8_t pf = static_cast<uint8_t>(moveFace(previous));
    if (cf == pf) return true;
    if (cf % 3 == pf % 3 && cf < pf) return true;  // opposite pair, wrong order
    return false;
}

} // namespace rubiks
