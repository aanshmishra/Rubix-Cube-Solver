#pragma once

#include "cube_state.h"
#include <array>
#include <string>
#include <unordered_map>

namespace rubiks {

// ---------------------------------------------------------------------------
// Facelet <-> cubie conversion.
//
// Facelet layout (54 chars), matching src/lib/solver/cubieCoords.ts:
//   U = 0..8, R = 9..17, F = 18..26, D = 27..35, L = 36..44, B = 45..53
//   each face row-major, top-left to bottom-right.
//
// The colour scheme is NOT hardcoded. An encoder is built from a "target"
// string - the facelets of a solved cube in whatever physical orientation the
// user has selected - exactly like makeCubieEncoder() on the TypeScript side.
// That keeps every orientation working without any orientation logic in C++.
// ---------------------------------------------------------------------------

constexpr uint8_t NUM_FACELETS = 54;

// Facelet indices of each corner slot, ordered [U/D sticker, then clockwise].
extern const std::array<std::array<uint8_t, 3>, NUM_CORNERS> CORNER_SLOTS;
// Facelet indices of each edge slot, ordered [primary, secondary].
extern const std::array<std::array<uint8_t, 2>, NUM_EDGES> EDGE_SLOTS;

class CubieEncoder {
public:
    explicit CubieEncoder(const std::string& target_facelets);

    // True when the target string is a well-formed solved cube.
    bool valid() const { return valid_; }

    // Reads 54 stickers into cubie coordinates. Returns false when the
    // stickers do not form a legal set of pieces (e.g. a hand-painted cube
    // with two identical corners), which is the cheapest impossible-cube check
    // available.
    bool encode(const std::string& facelets, CubeState& out) const;

    // Renders cubie coordinates back to 54 stickers in the target's scheme.
    std::string decode(const CubeState& state) const;

private:
    struct Piece { uint8_t id; uint8_t ori; };

    std::unordered_map<std::string, Piece> corner_lut_;
    std::unordered_map<std::string, Piece> edge_lut_;
    std::string target_;
    bool valid_ = false;
};

} // namespace rubiks
