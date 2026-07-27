#include "cube_state.h"
#include <sstream>

namespace rubiks {

// ---------------------------------------------------------------------------
// Quarter-turn move tables, indexed by face (U, R, F, D, L, B).
//
// GENERATED from the TypeScript engine (src/lib/cubeEngine.ts +
// src/lib/solver/cubieCoords.ts) by applying each base move to a solved cube
// and reading back the resulting cubie coordinates. Do not hand-edit: if the
// TS facelet geometry ever changes, regenerate rather than patch.
//
// Semantics (destination-indexed, so both tables are read with the same index):
//   cp_new[i] = cp_old[CORNER_SRC[f][i]]
//   co_new[i] = (co_old[CORNER_SRC[f][i]] + CORNER_TWIST[f][i]) % 3
// ---------------------------------------------------------------------------

static const uint8_t CORNER_SRC[NUM_FACES][NUM_CORNERS] = {
    { 3, 0, 1, 2, 4, 5, 6, 7 },  // U
    { 4, 1, 2, 0, 7, 5, 6, 3 },  // R
    { 1, 5, 2, 3, 0, 4, 6, 7 },  // F
    { 0, 1, 2, 3, 5, 6, 7, 4 },  // D
    { 0, 2, 6, 3, 4, 1, 5, 7 },  // L
    { 0, 1, 3, 7, 4, 5, 2, 6 },  // B
};

static const uint8_t CORNER_TWIST[NUM_FACES][NUM_CORNERS] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },  // U
    { 2, 0, 0, 1, 1, 0, 0, 2 },  // R
    { 1, 2, 0, 0, 2, 1, 0, 0 },  // F
    { 0, 0, 0, 0, 0, 0, 0, 0 },  // D
    { 0, 1, 2, 0, 0, 2, 1, 0 },  // L
    { 0, 0, 1, 2, 0, 0, 2, 1 },  // B
};

static const uint8_t EDGE_SRC[NUM_FACES][NUM_EDGES] = {
    { 3, 0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11 },  // U
    { 8, 1, 2, 3, 11, 5, 6, 7, 4, 9, 10, 0 },  // R
    { 0, 9, 2, 3, 4, 8, 6, 7, 1, 5, 10, 11 },  // F
    { 0, 1, 2, 3, 5, 6, 7, 4, 8, 9, 10, 11 },  // D
    { 0, 1, 10, 3, 4, 5, 9, 7, 8, 2, 6, 11 },  // L
    { 0, 1, 2, 11, 4, 5, 6, 10, 8, 9, 3, 7 },  // B
};

static const uint8_t EDGE_FLIP[NUM_FACES][NUM_EDGES] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // U
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // R
    { 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0 },  // F
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // D
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // L
    { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 },  // B
};

// ---------------------------------------------------------------------------

CubeState::CubeState() {
    corner_perm = {};
    corner_orient = {};
    edge_perm = {};
    edge_orient = {};
}

CubeState CubeState::solved() {
    CubeState s;
    for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
        s.corner_perm[i] = i;
        s.corner_orient[i] = 0;
    }
    for (uint8_t i = 0; i < NUM_EDGES; ++i) {
        s.edge_perm[i] = i;
        s.edge_orient[i] = 0;
    }
    return s;
}

void CubeState::applyMove(Move move) {
    const uint8_t m = static_cast<uint8_t>(move);
    if (m >= NUM_MOVES) return;

    // Only the six quarter-turn tables exist; doubles and inverses are composed
    // from them so derived tables cannot drift out of sync.
    const uint8_t face = m / 3;
    const uint8_t turns = (m % 3) + 1;

    for (uint8_t t = 0; t < turns; ++t) {
        const auto old_cp = corner_perm;
        const auto old_co = corner_orient;
        const auto old_ep = edge_perm;
        const auto old_eo = edge_orient;

        for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
            const uint8_t src = CORNER_SRC[face][i];
            corner_perm[i] = old_cp[src];
            corner_orient[i] = static_cast<uint8_t>((old_co[src] + CORNER_TWIST[face][i]) % 3);
        }
        for (uint8_t i = 0; i < NUM_EDGES; ++i) {
            const uint8_t src = EDGE_SRC[face][i];
            edge_perm[i] = old_ep[src];
            edge_orient[i] = static_cast<uint8_t>((old_eo[src] + EDGE_FLIP[face][i]) % 2);
        }
    }
}

void CubeState::applyMoves(const std::vector<Move>& moves) {
    for (Move m : moves) applyMove(m);
}

void CubeState::applyNotation(const std::string& notation) {
    applyMoves(parseNotation(notation));
}

bool CubeState::operator==(const CubeState& other) const {
    return corner_perm == other.corner_perm &&
           corner_orient == other.corner_orient &&
           edge_perm == other.edge_perm &&
           edge_orient == other.edge_orient;
}

// ---------------------------------------------------------------------------
// Move sets available to the searches. The last-layer stages use the <U,R,F>
// and <U,R> subgroups: a smaller branching factor, and the first two layers
// cannot be disturbed by moves that never touch them.
// ---------------------------------------------------------------------------

const Move MOVES_ALL[18] = {
    Move::U, Move::U2, Move::Up,
    Move::R, Move::R2, Move::Rp,
    Move::F, Move::F2, Move::Fp,
    Move::D, Move::D2, Move::Dp,
    Move::L, Move::L2, Move::Lp,
    Move::B, Move::B2, Move::Bp,
};

const Move MOVES_URF[9] = {
    Move::U, Move::U2, Move::Up,
    Move::R, Move::R2, Move::Rp,
    Move::F, Move::F2, Move::Fp,
};

const Move MOVES_UR[6] = {
    Move::U, Move::U2, Move::Up,
    Move::R, Move::R2, Move::Rp,
};

// ---------------------------------------------------------------------------
// Move utilities
// ---------------------------------------------------------------------------

static const char* const MOVE_NAMES[NUM_MOVES] = {
    "U", "U2", "U'",
    "R", "R2", "R'",
    "F", "F2", "F'",
    "D", "D2", "D'",
    "L", "L2", "L'",
    "B", "B2", "B'",
};

Move moveFromString(const std::string& s) {
    for (uint8_t i = 0; i < NUM_MOVES; ++i) {
        if (s == MOVE_NAMES[i]) return static_cast<Move>(i);
    }
    return Move::NONE;
}

std::string moveToString(Move m) {
    const uint8_t i = static_cast<uint8_t>(m);
    return i < NUM_MOVES ? MOVE_NAMES[i] : "?";
}

Move inverseMove(Move m) {
    const uint8_t i = static_cast<uint8_t>(m);
    if (i >= NUM_MOVES) return Move::NONE;
    const uint8_t face = i / 3;
    const uint8_t amount = i % 3;          // 0 = quarter, 1 = half, 2 = inverse
    const uint8_t inv = amount == 1 ? 1 : (amount == 0 ? 2 : 0);
    return static_cast<Move>(face * 3 + inv);
}

Face moveFace(Move m) {
    const uint8_t i = static_cast<uint8_t>(m);
    return i < NUM_MOVES ? static_cast<Face>(i / 3) : Face::COUNT;
}

std::vector<Move> parseNotation(const std::string& notation) {
    std::vector<Move> moves;
    std::istringstream iss(notation);
    std::string token;
    while (iss >> token) {
        Move m = moveFromString(token);
        if (m != Move::NONE) moves.push_back(m);
    }
    return moves;
}

std::vector<Move> simplifyMoves(const std::vector<Move>& moves) {
    // Quarter-turn count for each amount slot: X -> 1, X2 -> 2, X' -> 3.
    static const uint8_t TURNS[3] = {1, 2, 3};
    // Inverse mapping, quarter-turns -> amount slot.
    static const int8_t SLOT[4] = {-1, 0, 1, 2};

    std::vector<Move> out = moves;
    bool changed = true;

    // A cancellation can expose a new neighbouring pair (R U U' R -> R R -> R2),
    // so sweep until the sequence stops shrinking. These are a few dozen moves
    // at most, so the repeated passes cost nothing worth measuring.
    while (changed) {
        changed = false;
        std::vector<Move> pass;
        pass.reserve(out.size());

        for (Move m : out) {
            if (!pass.empty() && moveFace(pass.back()) == moveFace(m)) {
                const uint8_t face = static_cast<uint8_t>(moveFace(m));
                const uint8_t total = (TURNS[static_cast<uint8_t>(pass.back()) % 3] +
                                       TURNS[static_cast<uint8_t>(m) % 3]) % 4;
                pass.pop_back();
                if (total != 0) pass.push_back(static_cast<Move>(face * 3 + SLOT[total]));
                changed = true;
            } else {
                pass.push_back(m);
            }
        }
        out.swap(pass);
    }

    return out;
}

std::string movesToNotation(const std::vector<Move>& moves) {
    std::string result;
    for (size_t i = 0; i < moves.size(); ++i) {
        if (i > 0) result += " ";
        result += moveToString(moves[i]);
    }
    return result;
}

} // namespace rubiks
