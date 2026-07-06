#pragma once

#include "cube_state.h"
#include <array>
#include <string>
#include <vector>

namespace rubiks {

// Facelet indices for a standard cube net layout:
//
//        0  1  2
//        3  4  5
//        6  7  8
//  9 10 11 12 13 14 15 16 17
// 18 19 20 21 22 23 24 25 26
// 27 28 29 30 31 32 33 34 35
//       36 37 38
//       39 40 41
//       42 43 44
//       45 46 47
//       48 49 50
//       51 52 53
//
// Face order: U(0-8), R(9-17), F(18-26), D(27-35), L(36-44), B(45-53)
//
// Within each face: row-major (top-left to bottom-right)

constexpr uint8_t NUM_FACELETS = 54;

// Convert 54 facelet colors (as characters: W,R,G,Y,O,B) to cubie state.
// The center pieces define the color-to-face mapping:
// face 0 (U) center = facelet[4], face 1 (R) center = facelet[13], etc.
//
// physical_orientation: which face is "down" and which is "front".
// Format: "{down_face}{front_face}" e.g., "WF" means White center Down, Green center Front.
CubeState faceletsToState(const std::string& facelets, const std::string& physical_orientation = "WF");

// Convert cubie state back to facelets (for a given orientation)
std::string stateToFacelets(const CubeState& state, const std::string& physical_orientation = "WF");

// Validate that facelets form a solvable cube
bool validateFaceletSolvable(const std::string& facelets);

// Get the color of a specific sticker on a specific cubie (in solved state)
Color getSolvedCornerColor(Corner corner, uint8_t sticker_index);
Color getSolvedEdgeColor(Edge edge, uint8_t sticker_index);

// Facelet index helpers
uint8_t faceletIndex(Face face, uint8_t row, uint8_t col);
Face faceFromFaceletIndex(uint8_t idx);

} // namespace rubiks
