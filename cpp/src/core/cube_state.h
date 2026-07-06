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
// CORNER indices (solved positions):
//   0: URF  1: UFL  2: ULB  3: UBR  4: DFR  5: DLF  6: DBL  7: DRB
// EDGE indices (solved positions):
//   0: UR   1: UF   2: UL   3: UB   4: DR   5: DF   6: DL   7: DB
//   8: FR   9: FL  10: BL  11: BR
// ---------------------------------------------------------------------------

namespace rubiks {

// Corner and edge definitions for facelet mapping
enum class Corner : uint8_t {
    URF = 0, UFL = 1, ULB = 2, UBR = 3,
    DFR = 4, DLF = 5, DBL = 6, DRB = 7,
    COUNT = 8
};

enum class Edge : uint8_t {
    UR = 0, UF = 1, UL = 2, UB = 3,
    DR = 4, DF = 5, DL = 6, DB = 7,
    FR = 8, FL = 9, BL = 10, BR = 11,
    COUNT = 12
};

// Face indices for facelet-level representation
enum class Face : uint8_t {
    U = 0, R = 1, F = 2, D = 3, L = 4, B = 5,
    COUNT = 6
};

// Face colors (W=White, R=Red, G=Green, Y=Yellow, O=Orange, B=Blue)
enum class Color : uint8_t {
    W = 0, R = 1, G = 2, Y = 3, O = 4, B = 5,
    COUNT = 6, INVALID = 255
};

// 18 possible moves: 6 faces × 3 turn amounts
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

// A cubie-level cube state
struct CubeState {
    // cp[i] = which corner is at position i
    std::array<uint8_t, NUM_CORNERS> corner_perm;
    // co[i] = orientation of the corner at position i (0..2)
    std::array<uint8_t, NUM_CORNERS> corner_orient;
    // ep[i] = which edge is at position i
    std::array<uint8_t, NUM_EDGES> edge_perm;
    // eo[i] = orientation of the edge at position i (0..1)
    std::array<uint8_t, NUM_EDGES> edge_orient;

    CubeState();

    // Solved state factory
    static CubeState solved();

    // Apply a single move
    void applyMove(Move move);

    // Apply a sequence of moves
    void applyMoves(const std::vector<Move>& moves);

    // Apply moves from notation string (e.g., "R U R' U'")
    void applyNotation(const std::string& notation);

    // Check equality
    bool operator==(const CubeState& other) const;

    // Hash function for use in unordered containers
    size_t hash() const;
};

// Move utility functions
Move moveFromString(const std::string& s);
std::string moveToString(Move m);
Move inverseMove(Move m);
Face moveFace(Move m);

// Convert notation string to vector of moves
std::vector<Move> parseNotation(const std::string& notation);

// Convert moves back to notation
std::string movesToNotation(const std::vector<Move>& moves);

} // namespace rubiks

// Specialization for std::hash
namespace std {
template<>
struct hash<rubiks::CubeState> {
    size_t operator()(const rubiks::CubeState& s) const {
        return s.hash();
    }
};
} // namespace std
