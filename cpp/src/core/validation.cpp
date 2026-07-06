#include "validation.h"
#include <numeric>
#include <unordered_set>

namespace rubiks {

ValidationResult validateState(const CubeState& state) {
    // Check corner orientation sum
    uint8_t corner_orient_sum = 0;
    for (uint8_t o : state.corner_orient) {
        corner_orient_sum += o;
    }
    if (corner_orient_sum % 3 != 0) {
        return ValidationResult(false, "Invalid corner orientation sum (cube is physically impossible)");
    }

    // Check edge orientation sum
    uint8_t edge_orient_sum = 0;
    for (uint8_t o : state.edge_orient) {
        edge_orient_sum += o;
    }
    if (edge_orient_sum % 2 != 0) {
        return ValidationResult(false, "Invalid edge orientation sum (cube is physically impossible)");
    }

    // Check permutation parity
    // Count inversions in corner permutation
    int corner_inversions = 0;
    for (int i = 0; i < NUM_CORNERS; ++i) {
        for (int j = i + 1; j < NUM_CORNERS; ++j) {
            if (state.corner_perm[i] > state.corner_perm[j]) {
                corner_inversions++;
            }
        }
    }

    // Count inversions in edge permutation
    int edge_inversions = 0;
    for (int i = 0; i < NUM_EDGES; ++i) {
        for (int j = i + 1; j < NUM_EDGES; ++j) {
            if (state.edge_perm[i] > state.edge_perm[j]) {
                edge_inversions++;
            }
        }
    }

    // Corner and edge parity must match (both even or both odd)
    if ((corner_inversions % 2) != (edge_inversions % 2)) {
        return ValidationResult(false, "Permutation parity mismatch (cube is physically impossible)");
    }

    // Check that all corner positions contain valid corner indices
    std::unordered_set<uint8_t> corner_set;
    for (uint8_t c : state.corner_perm) {
        if (c >= NUM_CORNERS) {
            return ValidationResult(false, "Invalid corner index in permutation");
        }
        corner_set.insert(c);
    }
    if (corner_set.size() != NUM_CORNERS) {
        return ValidationResult(false, "Duplicate corners in permutation");
    }

    // Check that all edge positions contain valid edge indices
    std::unordered_set<uint8_t> edge_set;
    for (uint8_t e : state.edge_perm) {
        if (e >= NUM_EDGES) {
            return ValidationResult(false, "Invalid edge index in permutation");
        }
        edge_set.insert(e);
    }
    if (edge_set.size() != NUM_EDGES) {
        return ValidationResult(false, "Duplicate edges in permutation");
    }

    return ValidationResult(true, "");
}

ValidationResult validateFacelets(const std::string& facelets) {
    if (facelets.length() != 54) {
        return ValidationResult(false, "Facelet string must be exactly 54 characters");
    }

    // Count each color
    int counts[6] = {0};
    for (char c : facelets) {
        switch (c) {
            case 'W': counts[0]++; break;
            case 'R': counts[1]++; break;
            case 'G': counts[2]++; break;
            case 'Y': counts[3]++; break;
            case 'O': counts[4]++; break;
            case 'B': counts[5]++; break;
            default:
                return ValidationResult(false, std::string("Invalid character in facelets: ") + c);
        }
    }

    for (int i = 0; i < 6; ++i) {
        if (counts[i] != 9) {
            return ValidationResult(false, "Each color must appear exactly 9 times in facelets");
        }
    }

    return ValidationResult(true, "");
}

bool isSolved(const CubeState& state) {
    for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
        if (state.corner_perm[i] != i || state.corner_orient[i] != 0) return false;
    }
    for (uint8_t i = 0; i < NUM_EDGES; ++i) {
        if (state.edge_perm[i] != i || state.edge_orient[i] != 0) return false;
    }
    return true;
}

bool isCrossSolved(const CubeState& state, Face face) {
    // Check if the 4 edges around the given face center are solved
    // For standard orientation (White=U, Green=F):
    // Cross on D face means DF, DR, DB, DL edges are solved
    uint8_t cross_edges[4];
    switch (face) {
        case Face::D: // White cross (typically on D)
            cross_edges[0] = 5;  // DF
            cross_edges[1] = 4;  // DR
            cross_edges[2] = 7;  // DB
            cross_edges[3] = 6;  // DL
            break;
        case Face::U: // Yellow cross
            cross_edges[0] = 1;  // UF
            cross_edges[1] = 0;  // UR
            cross_edges[2] = 3;  // UB
            cross_edges[3] = 2;  // UL
            break;
        default:
            return false;
    }

    for (uint8_t e : cross_edges) {
        if (state.edge_perm[e] != e || state.edge_orient[e] != 0) {
            return false;
        }
    }
    return true;
}

bool isF2LSolved(const CubeState& state) {
    // First two layers: D face + middle layer
    // Check cross edges + 4 F2L corner-edge pairs
    if (!isCrossSolved(state, Face::D)) return false;

    // F2L pairs: DFR(4)+FR(8), DLF(5)+FL(9), DBL(6)+BL(10), DRB(7)+BR(11)
    const uint8_t f2l_corners[4] = {4, 5, 6, 7};
    const uint8_t f2l_edges[4] = {8, 9, 10, 11};

    for (int i = 0; i < 4; ++i) {
        if (state.corner_perm[f2l_corners[i]] != f2l_corners[i] ||
            state.corner_orient[f2l_corners[i]] != 0) return false;
        if (state.edge_perm[f2l_edges[i]] != f2l_edges[i] ||
            state.edge_orient[f2l_edges[i]] != 0) return false;
    }

    return true;
}

bool isOLLSolved(const CubeState& state) {
    // Last layer (U face) oriented: all U-face stickers on U face
    // Check that U layer corners and edges are oriented correctly
    // U corners: URF(0), UFL(1), ULB(2), UBR(3)
    // U edges: UR(0), UF(1), UL(2), UB(3)
    const uint8_t u_corners[4] = {0, 1, 2, 3};
    const uint8_t u_edges[4] = {0, 1, 2, 3};

    for (uint8_t c : u_corners) {
        if (state.corner_orient[c] != 0) return false;
    }
    for (uint8_t e : u_edges) {
        if (state.edge_orient[e] != 0) return false;
    }

    return true;
}

int identifyOLLCase(const CubeState& state) {
    // Simplified: check basic patterns
    // Full 57-case identification requires full facelet state
    // Return -1 for unimplemented (caller should handle)
    (void)state;
    return -1;
}

int identifyPLLCase(const CubeState& state) {
    // Simplified: check basic patterns
    // Full 21-case identification requires full facelet state
    // Return -1 for unimplemented (caller should handle)
    (void)state;
    return -1;
}

} // namespace rubiks
