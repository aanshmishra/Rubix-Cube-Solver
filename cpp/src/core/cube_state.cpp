#include "cube_state.h"
#include <algorithm>
#include <sstream>

namespace rubiks {

// ---------------------------------------------------------------------------
// Move tables — pre-computed permutation data for each move
// ---------------------------------------------------------------------------

// Corner permutation: corner_perm[move][pos] = new position mapping
static const uint8_t CORNER_PERM[NUM_MOVES][NUM_CORNERS] = {
    // U
    {1, 2, 3, 0, 4, 5, 6, 7},
    // U2
    {2, 3, 0, 1, 4, 5, 6, 7},
    // U'
    {3, 0, 1, 2, 4, 5, 6, 7},
    // R
    {0, 1, 3, 4, 7, 5, 6, 2},
    // R2
    {0, 1, 4, 7, 3, 5, 6, 2},
    // R'
    {0, 1, 7, 2, 3, 5, 6, 4},
    // F
    {2, 1, 5, 3, 0, 4, 6, 7},
    // F2
    {5, 1, 4, 3, 2, 0, 6, 7},
    // F'
    {4, 1, 0, 3, 5, 2, 6, 7},
    // D
    {0, 1, 2, 3, 5, 6, 7, 4},
    // D2
    {0, 1, 2, 3, 6, 7, 4, 5},
    // D'
    {0, 1, 2, 3, 7, 4, 5, 6},
    // L
    {0, 5, 1, 3, 4, 6, 2, 7},
    // L2
    {0, 6, 2, 3, 4, 1, 5, 7},
    // L'
    {0, 6, 2, 3, 4, 1, 5, 7},
    // B
    {0, 7, 2, 1, 4, 5, 3, 6},
    // B2
    {0, 6, 2, 7, 4, 5, 1, 3},
    // B'
    {0, 3, 2, 6, 4, 5, 7, 1},
};

// Corner orientation change: corner_orient[move][pos] = orientation delta
static const uint8_t CORNER_ORIENT[NUM_MOVES][NUM_CORNERS] = {
    {0, 0, 0, 0, 0, 0, 0, 0},  // U
    {0, 0, 0, 0, 0, 0, 0, 0},  // U2
    {0, 0, 0, 0, 0, 0, 0, 0},  // U'
    {0, 0, 1, 2, 2, 0, 0, 1},  // R
    {0, 0, 0, 0, 0, 0, 0, 0},  // R2
    {0, 0, 2, 1, 1, 0, 0, 2},  // R'
    {1, 0, 2, 0, 2, 1, 0, 0},  // F
    {0, 0, 0, 0, 0, 0, 0, 0},  // F2
    {2, 0, 1, 0, 1, 2, 0, 0},  // F'
    {0, 0, 0, 0, 0, 0, 0, 0},  // D
    {0, 0, 0, 0, 0, 0, 0, 0},  // D2
    {0, 0, 0, 0, 0, 0, 0, 0},  // D'
    {0, 1, 2, 0, 0, 2, 1, 0},  // L
    {0, 0, 0, 0, 0, 0, 0, 0},  // L2
    {0, 1, 0, 0, 0, 2, 1, 0},  // L'
    {0, 2, 0, 1, 0, 0, 2, 1},  // B
    {0, 0, 0, 0, 0, 0, 0, 0},  // B2
    {0, 1, 0, 2, 0, 0, 1, 2},  // B'
};

// Edge permutation: edge_perm[move][pos] = new position mapping
static const uint8_t EDGE_PERM[NUM_MOVES][NUM_EDGES] = {
    {1, 2, 3, 0, 4, 5, 6, 7, 8, 9, 10, 11},  // U
    {2, 3, 0, 1, 4, 5, 6, 7, 8, 9, 10, 11},  // U2
    {3, 0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11},  // U'
    {0, 1, 2, 11, 4, 5, 6, 9, 8, 3, 10, 7},  // R
    {0, 1, 2, 9, 4, 5, 6, 11, 8, 7, 10, 3},  // R2
    {0, 1, 2, 9, 4, 5, 6, 11, 8, 7, 10, 3},  // R'
    {0, 9, 2, 3, 8, 5, 6, 7, 1, 4, 10, 11},  // F
    {0, 4, 2, 3, 9, 5, 6, 7, 1, 8, 10, 11},  // F2
    {0, 8, 2, 3, 9, 5, 6, 7, 4, 1, 10, 11},  // F'
    {0, 1, 2, 3, 5, 6, 7, 4, 8, 9, 10, 11},  // D
    {0, 1, 2, 3, 6, 7, 4, 5, 8, 9, 10, 11},  // D2
    {0, 1, 2, 3, 7, 4, 5, 6, 8, 9, 10, 11},  // D'
    {0, 1, 10, 3, 4, 9, 6, 7, 8, 2, 5, 11},  // L
    {0, 1, 5, 3, 4, 10, 6, 7, 8, 2, 9, 11},  // L2
    {0, 1, 5, 3, 4, 10, 6, 7, 8, 2, 9, 11},  // L'
    {0, 1, 2, 7, 4, 5, 10, 3, 8, 9, 6, 11},  // B
    {0, 1, 2, 10, 4, 5, 7, 6, 8, 9, 3, 11},  // B2
    {0, 1, 2, 6, 4, 5, 10, 3, 8, 9, 7, 11},  // B'
};

// Edge orientation change: edge_orient[move][pos] = orientation delta
static const uint8_t EDGE_ORIENT[NUM_MOVES][NUM_EDGES] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // U
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // U2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // U'
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // R
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // R2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // R'
    {0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0},  // F
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // F2
    {0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0},  // F'
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // D
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // D2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // D'
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // L
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // L2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // L'
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // B
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // B2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // B'
};

// ---------------------------------------------------------------------------
// CubeState implementation
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
    uint8_t m = static_cast<uint8_t>(move);
    if (m >= NUM_MOVES) return;

    // The quarter-turn tables are the source of truth. Double and inverse
    // turns are composed from them so hand-written derived tables cannot drift.
    const uint8_t base_move = static_cast<uint8_t>((m / 3) * 3);
    const uint8_t turns = static_cast<uint8_t>((m % 3) + 1);

    for (uint8_t turn = 0; turn < turns; ++turn) {
        auto old_cp = corner_perm;
        auto old_co = corner_orient;
        auto old_ep = edge_perm;
        auto old_eo = edge_orient;

        for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
            uint8_t src = CORNER_PERM[base_move][i];
            corner_perm[i] = old_cp[src];
            corner_orient[i] = (old_co[src] + CORNER_ORIENT[base_move][src]) % 3;
        }

        for (uint8_t i = 0; i < NUM_EDGES; ++i) {
            uint8_t src = EDGE_PERM[base_move][i];
            edge_perm[i] = old_ep[src];
            edge_orient[i] = (old_eo[src] + EDGE_ORIENT[base_move][src]) % 2;
        }
    }
}

void CubeState::applyMoves(const std::vector<Move>& moves) {
    for (Move m : moves) {
        applyMove(m);
    }
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

size_t CubeState::hash() const {
    // FNV-1a hash
    uint64_t h = 14695981039346656037ULL;
    auto mix = [&h](uint8_t v) {
        h ^= static_cast<uint64_t>(v);
        h *= 1099511628211ULL;
    };
    for (auto v : corner_perm) mix(v);
    for (auto v : corner_orient) mix(v);
    for (auto v : edge_perm) mix(v);
    for (auto v : edge_orient) mix(v);
    return static_cast<size_t>(h);
}

// ---------------------------------------------------------------------------
// Move utilities
// ---------------------------------------------------------------------------

Move moveFromString(const std::string& s) {
    if (s == "U") return Move::U;
    if (s == "U2") return Move::U2;
    if (s == "U'") return Move::Up;
    if (s == "R") return Move::R;
    if (s == "R2") return Move::R2;
    if (s == "R'") return Move::Rp;
    if (s == "F") return Move::F;
    if (s == "F2") return Move::F2;
    if (s == "F'") return Move::Fp;
    if (s == "D") return Move::D;
    if (s == "D2") return Move::D2;
    if (s == "D'") return Move::Dp;
    if (s == "L") return Move::L;
    if (s == "L2") return Move::L2;
    if (s == "L'") return Move::Lp;
    if (s == "B") return Move::B;
    if (s == "B2") return Move::B2;
    if (s == "B'") return Move::Bp;
    return Move::NONE;
}

std::string moveToString(Move m) {
    switch (m) {
        case Move::U: return "U";
        case Move::U2: return "U2";
        case Move::Up: return "U'";
        case Move::R: return "R";
        case Move::R2: return "R2";
        case Move::Rp: return "R'";
        case Move::F: return "F";
        case Move::F2: return "F2";
        case Move::Fp: return "F'";
        case Move::D: return "D";
        case Move::D2: return "D2";
        case Move::Dp: return "D'";
        case Move::L: return "L";
        case Move::L2: return "L2";
        case Move::Lp: return "L'";
        case Move::B: return "B";
        case Move::B2: return "B2";
        case Move::Bp: return "B'";
        default: return "?";
    }
}

Move inverseMove(Move m) {
    switch (m) {
        case Move::U: return Move::Up;
        case Move::U2: return Move::U2;
        case Move::Up: return Move::U;
        case Move::R: return Move::Rp;
        case Move::R2: return Move::R2;
        case Move::Rp: return Move::R;
        case Move::F: return Move::Fp;
        case Move::F2: return Move::F2;
        case Move::Fp: return Move::F;
        case Move::D: return Move::Dp;
        case Move::D2: return Move::D2;
        case Move::Dp: return Move::D;
        case Move::L: return Move::Lp;
        case Move::L2: return Move::L2;
        case Move::Lp: return Move::L;
        case Move::B: return Move::Bp;
        case Move::B2: return Move::B2;
        case Move::Bp: return Move::B;
        default: return Move::NONE;
    }
}

Face moveFace(Move m) {
    uint8_t mi = static_cast<uint8_t>(m);
    if (mi < 3) return Face::U;
    if (mi < 6) return Face::R;
    if (mi < 9) return Face::F;
    if (mi < 12) return Face::D;
    if (mi < 15) return Face::L;
    return Face::B;
}

std::vector<Move> parseNotation(const std::string& notation) {
    std::vector<Move> moves;
    std::istringstream iss(notation);
    std::string token;
    while (iss >> token) {
        Move m = moveFromString(token);
        if (m != Move::NONE) {
            moves.push_back(m);
        }
    }
    return moves;
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
