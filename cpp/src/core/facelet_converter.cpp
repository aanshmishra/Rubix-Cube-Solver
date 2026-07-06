#include "facelet_converter.h"
#include "validation.h"
#include <unordered_map>
#include <stdexcept>

namespace rubiks {

// ---------------------------------------------------------------------------
// Corner facelet definitions
//
// Each corner has 3 facelets. We define which face+row+col each sticker
// appears at in the solved state (with standard orientation).
//
// Corner URF: U(2,2), R(0,0), F(0,2) -> indices 2, 9, 20
// Corner UFL: U(2,0), F(0,0), L(0,2) -> indices 0, 18, 44
// etc.
// ---------------------------------------------------------------------------

struct FaceletPos {
    Face face;
    uint8_t row;
    uint8_t col;
};

// In solved state, each corner has 3 facelets at these positions
static const std::array<std::array<FaceletPos, 3>, NUM_CORNERS> CORNER_FACELETS = {{
    // URF: U bottom-right, R top-left, F top-right
    {{ {Face::U, 2, 2}, {Face::R, 0, 0}, {Face::F, 0, 2} }},
    // UFL: U bottom-left, F top-left, L top-right
    {{ {Face::U, 2, 0}, {Face::F, 0, 0}, {Face::L, 0, 2} }},
    // ULB: U top-left, L top-left, B top-right
    {{ {Face::U, 0, 0}, {Face::L, 0, 0}, {Face::B, 0, 2} }},
    // UBR: U top-right, B top-left, R top-right
    {{ {Face::U, 0, 2}, {Face::B, 0, 0}, {Face::R, 0, 2} }},
    // DFR: D top-right, F bottom-right, R bottom-left
    {{ {Face::D, 0, 2}, {Face::F, 2, 2}, {Face::R, 2, 0} }},
    // DLF: D top-left, L bottom-right, F bottom-left
    {{ {Face::D, 0, 0}, {Face::L, 2, 2}, {Face::F, 2, 0} }},
    // DBL: D bottom-left, B bottom-right, L bottom-left
    {{ {Face::D, 2, 0}, {Face::B, 2, 2}, {Face::L, 2, 0} }},
    // DRB: D bottom-right, R bottom-right, B bottom-left
    {{ {Face::D, 2, 2}, {Face::R, 2, 2}, {Face::B, 2, 0} }},
}};

// In solved state, each edge has 2 facelets at these positions
static const std::array<std::array<FaceletPos, 2>, NUM_EDGES> EDGE_FACELETS = {{
    // UR: U right-mid, R top-mid
    {{ {Face::U, 1, 2}, {Face::R, 0, 1} }},
    // UF: U bottom-mid, F top-mid
    {{ {Face::U, 2, 1}, {Face::F, 0, 1} }},
    // UL: U left-mid, L top-mid
    {{ {Face::U, 1, 0}, {Face::L, 0, 1} }},
    // UB: U top-mid, B top-mid
    {{ {Face::U, 0, 1}, {Face::B, 0, 1} }},
    // DR: D right-mid, R bottom-mid
    {{ {Face::D, 1, 2}, {Face::R, 2, 1} }},
    // DF: D top-mid, F bottom-mid
    {{ {Face::D, 0, 1}, {Face::F, 2, 1} }},
    // DL: D left-mid, L bottom-mid
    {{ {Face::D, 1, 0}, {Face::L, 2, 1} }},
    // DB: D bottom-mid, B bottom-mid
    {{ {Face::D, 2, 1}, {Face::B, 2, 1} }},
    // FR: F right-mid, R left-mid
    {{ {Face::F, 1, 2}, {Face::R, 1, 0} }},
    // FL: F left-mid, L right-mid
    {{ {Face::F, 1, 0}, {Face::L, 1, 2} }},
    // BL: B right-mid, L left-mid
    {{ {Face::B, 1, 2}, {Face::L, 1, 0} }},
    // BR: B left-mid, R right-mid
    {{ {Face::B, 1, 0}, {Face::R, 1, 2} }},
}};

// Which face each corner "belongs to" based on its first facelet color
static const Color CORNER_COLORS[NUM_CORNERS][3] = {
    {Color::W, Color::R, Color::G}, // URF: White, Red, Green
    {Color::W, Color::G, Color::O}, // UFL: White, Green, Orange
    {Color::W, Color::O, Color::B}, // ULB: White, Orange, Blue
    {Color::W, Color::B, Color::R}, // UBR: White, Blue, Red
    {Color::Y, Color::G, Color::R}, // DFR: Yellow, Green, Red
    {Color::Y, Color::O, Color::G}, // DLF: Yellow, Orange, Green
    {Color::Y, Color::B, Color::O}, // DBL: Yellow, Blue, Orange
    {Color::Y, Color::R, Color::B}, // DRB: Yellow, Red, Blue
};

static const Color EDGE_COLORS[NUM_EDGES][2] = {
    {Color::W, Color::R}, // UR
    {Color::W, Color::G}, // UF
    {Color::W, Color::O}, // UL
    {Color::W, Color::B}, // UB
    {Color::Y, Color::R}, // DR
    {Color::Y, Color::G}, // DF
    {Color::Y, Color::O}, // DL
    {Color::Y, Color::B}, // DB
    {Color::G, Color::R}, // FR
    {Color::G, Color::O}, // FL
    {Color::B, Color::O}, // BL
    {Color::B, Color::R}, // BR
};

static uint8_t faceletPosToIndex(const FaceletPos& pos) {
    uint8_t face_base = 0;
    switch (pos.face) {
        case Face::U: face_base = 0; break;
        case Face::R: face_base = 9; break;
        case Face::F: face_base = 18; break;
        case Face::D: face_base = 27; break;
        case Face::L: face_base = 36; break;
        case Face::B: face_base = 45; break;
        default: face_base = 0;
    }
    return face_base + pos.row * 3 + pos.col;
}

static Color charToColor(char c) {
    switch (c) {
        case 'W': return Color::W;
        case 'R': return Color::R;
        case 'G': return Color::G;
        case 'Y': return Color::Y;
        case 'O': return Color::O;
        case 'B': return Color::B;
        default: return Color::INVALID;
    }
}

static char colorToChar(Color c) {
    switch (c) {
        case Color::W: return 'W';
        case Color::R: return 'R';
        case Color::G: return 'G';
        case Color::Y: return 'Y';
        case Color::O: return 'O';
        case Color::B: return 'B';
        default: return '?';
    }
}

// ---------------------------------------------------------------------------
// faceletsToState
// ---------------------------------------------------------------------------

CubeState faceletsToState(const std::string& facelets, const std::string& physical_orientation) {
    (void)physical_orientation; // TODO: handle non-standard orientations

    CubeState state;

    // Build mapping from facelet index to color
    std::array<Color, NUM_FACELETS> facelet_colors;
    for (uint8_t i = 0; i < NUM_FACELETS; ++i) {
        facelet_colors[i] = charToColor(facelets[i]);
    }

    // Determine center colors (these define which face is which color)
    Color center_colors[6] = {
        facelet_colors[4],   // U center
        facelet_colors[13],  // R center
        facelet_colors[22],  // F center
        facelet_colors[31],  // D center
        facelet_colors[40],  // L center
        facelet_colors[49],  // B center
    };

    // Build a map: color -> face (based on center colors)
    std::unordered_map<char, Face> color_to_face;
    for (int f = 0; f < 6; ++f) {
        color_to_face[colorToChar(center_colors[f])] = static_cast<Face>(f);
    }

    // ---- Identify corners ----
    // For each corner position, look at the 3 facelets and identify which corner cubie is there
    for (uint8_t pos = 0; pos < NUM_CORNERS; ++pos) {
        // Get the 3 facelets at this corner position
        Color facelet_colors_at_pos[3];
        for (uint8_t s = 0; s < 3; ++s) {
            uint8_t idx = faceletPosToIndex(CORNER_FACELETS[pos][s]);
            facelet_colors_at_pos[s] = facelet_colors[idx];
        }

        // Find which corner cubie has these 3 colors
        bool found = false;
        for (uint8_t cubie = 0; cubie < NUM_CORNERS; ++cubie) {
            // Check if the 3 colors match (in any order)
            std::array<Color, 3> cubie_colors = {CORNER_COLORS[cubie][0], CORNER_COLORS[cubie][1], CORNER_COLORS[cubie][2]};
            std::array<Color, 3> pos_colors = {facelet_colors_at_pos[0], facelet_colors_at_pos[1], facelet_colors_at_pos[2]};

            // Check all permutations
            for (int rot = 0; rot < 3; ++rot) {
                if (cubie_colors[0] == pos_colors[rot % 3] &&
                    cubie_colors[1] == pos_colors[(rot + 1) % 3] &&
                    cubie_colors[2] == pos_colors[(rot + 2) % 3]) {
                    state.corner_perm[pos] = cubie;
                    state.corner_orient[pos] = static_cast<uint8_t>((3 - rot) % 3);
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        if (!found) {
            // Invalid corner - this shouldn't happen with valid facelets
            state.corner_perm[pos] = pos;
            state.corner_orient[pos] = 0;
        }
    }

    // ---- Identify edges ----
    for (uint8_t pos = 0; pos < NUM_EDGES; ++pos) {
        Color facelet_colors_at_pos[2];
        for (uint8_t s = 0; s < 2; ++s) {
            uint8_t idx = faceletPosToIndex(EDGE_FACELETS[pos][s]);
            facelet_colors_at_pos[s] = facelet_colors[idx];
        }

        // Find which edge cubie has these 2 colors
        bool found = false;
        for (uint8_t cubie = 0; cubie < NUM_EDGES; ++cubie) {
            Color c0 = EDGE_COLORS[cubie][0];
            Color c1 = EDGE_COLORS[cubie][1];

            if ((c0 == facelet_colors_at_pos[0] && c1 == facelet_colors_at_pos[1])) {
                state.edge_perm[pos] = cubie;
                state.edge_orient[pos] = 0;
                found = true;
                break;
            }
            if ((c0 == facelet_colors_at_pos[1] && c1 == facelet_colors_at_pos[0])) {
                state.edge_perm[pos] = cubie;
                state.edge_orient[pos] = 1;
                found = true;
                break;
            }
        }

        if (!found) {
            state.edge_perm[pos] = pos;
            state.edge_orient[pos] = 0;
        }
    }

    return state;
}

// ---------------------------------------------------------------------------
// stateToFacelets
// ---------------------------------------------------------------------------

std::string stateToFacelets(const CubeState& state, const std::string& physical_orientation) {
    (void)physical_orientation; // TODO: handle non-standard orientations

    std::string facelets(54, '?');

    // Set centers (solved)
    facelets[4] = 'W';   // U center = White
    facelets[13] = 'R';  // R center = Red
    facelets[22] = 'G';  // F center = Green
    facelets[31] = 'Y';  // D center = Yellow
    facelets[40] = 'O';  // L center = Orange
    facelets[49] = 'B';  // B center = Blue

    // For each corner position, determine the 3 facelet colors
    for (uint8_t pos = 0; pos < NUM_CORNERS; ++pos) {
        uint8_t cubie = state.corner_perm[pos];
        uint8_t orient = state.corner_orient[pos];

        // The cubie's colors (in reference frame)
        std::array<Color, 3> cubie_colors = {
            CORNER_COLORS[cubie][0],
            CORNER_COLORS[cubie][1],
            CORNER_COLORS[cubie][2]
        };

        // Rotate based on orientation
        // orient 0: no twist, orient 1: clockwise, orient 2: counter-clockwise
        for (uint8_t s = 0; s < 3; ++s) {
            uint8_t color_idx = (s + 3 - orient) % 3;
            uint8_t facelet_idx = faceletPosToIndex(CORNER_FACELETS[pos][s]);
            facelets[facelet_idx] = colorToChar(cubie_colors[color_idx]);
        }
    }

    // For each edge position, determine the 2 facelet colors
    for (uint8_t pos = 0; pos < NUM_EDGES; ++pos) {
        uint8_t cubie = state.edge_perm[pos];
        uint8_t orient = state.edge_orient[pos];

        Color colors[2] = {EDGE_COLORS[cubie][0], EDGE_COLORS[cubie][1]};

        for (uint8_t s = 0; s < 2; ++s) {
            uint8_t color_idx = (s + orient) % 2;
            uint8_t facelet_idx = faceletPosToIndex(EDGE_FACELETS[pos][s]);
            facelets[facelet_idx] = colorToChar(colors[color_idx]);
        }
    }

    return facelets;
}

// ---------------------------------------------------------------------------
// validateFaceletSolvable
// ---------------------------------------------------------------------------

bool validateFaceletSolvable(const std::string& facelets) {
    auto val = validateFacelets(facelets);
    if (!val.valid) return false;

    CubeState state = faceletsToState(facelets);
    val = validateState(state);
    return val.valid;
}

// ---------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------

Color getSolvedCornerColor(Corner corner, uint8_t sticker_index) {
    uint8_t c = static_cast<uint8_t>(corner);
    if (c >= NUM_CORNERS || sticker_index >= 3) return Color::INVALID;
    return CORNER_COLORS[c][sticker_index];
}

Color getSolvedEdgeColor(Edge edge, uint8_t sticker_index) {
    uint8_t e = static_cast<uint8_t>(edge);
    if (e >= NUM_EDGES || sticker_index >= 2) return Color::INVALID;
    return EDGE_COLORS[e][sticker_index];
}

uint8_t faceletIndex(Face face, uint8_t row, uint8_t col) {
    uint8_t face_base = 0;
    switch (face) {
        case Face::U: face_base = 0; break;
        case Face::R: face_base = 9; break;
        case Face::F: face_base = 18; break;
        case Face::D: face_base = 27; break;
        case Face::L: face_base = 36; break;
        case Face::B: face_base = 45; break;
        default: face_base = 0;
    }
    return face_base + row * 3 + col;
}

Face faceFromFaceletIndex(uint8_t idx) {
    if (idx < 9) return Face::U;
    if (idx < 18) return Face::R;
    if (idx < 27) return Face::F;
    if (idx < 36) return Face::D;
    if (idx < 45) return Face::L;
    return Face::B;
}

} // namespace rubiks
