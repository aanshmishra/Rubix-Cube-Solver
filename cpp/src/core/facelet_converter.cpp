#include "facelet_converter.h"

namespace rubiks {

const std::array<std::array<uint8_t, 3>, NUM_CORNERS> CORNER_SLOTS = {{
    {{  8,  9, 20 }},  // URF
    {{  6, 18, 38 }},  // UFL
    {{  0, 36, 47 }},  // ULB
    {{  2, 45, 11 }},  // UBR
    {{ 29, 26, 15 }},  // DFR
    {{ 27, 44, 24 }},  // DLF
    {{ 33, 53, 42 }},  // DBL
    {{ 35, 17, 51 }},  // DRB
}};

const std::array<std::array<uint8_t, 2>, NUM_EDGES> EDGE_SLOTS = {{
    {{  5, 10 }},  // UR
    {{  7, 19 }},  // UF
    {{  3, 37 }},  // UL
    {{  1, 46 }},  // UB
    {{ 32, 16 }},  // DR
    {{ 28, 25 }},  // DF
    {{ 30, 43 }},  // DL
    {{ 34, 52 }},  // DB
    {{ 23, 12 }},  // FR
    {{ 21, 41 }},  // FL
    {{ 50, 39 }},  // BL
    {{ 48, 14 }},  // BR
}};

CubieEncoder::CubieEncoder(const std::string& target_facelets)
    : target_(target_facelets) {
    if (target_.size() != NUM_FACELETS) return;

    // A corner twisted clockwise r times shows its home colours (c0,c1,c2)
    // cyclically shifted so that c0 lands at tuple index r.
    for (uint8_t id = 0; id < NUM_CORNERS; ++id) {
        const char c0 = target_[CORNER_SLOTS[id][0]];
        const char c1 = target_[CORNER_SLOTS[id][1]];
        const char c2 = target_[CORNER_SLOTS[id][2]];
        corner_lut_[{c0, c1, c2}] = Piece{id, 0};
        corner_lut_[{c2, c0, c1}] = Piece{id, 1};
        corner_lut_[{c1, c2, c0}] = Piece{id, 2};
    }

    for (uint8_t id = 0; id < NUM_EDGES; ++id) {
        const char c0 = target_[EDGE_SLOTS[id][0]];
        const char c1 = target_[EDGE_SLOTS[id][1]];
        edge_lut_[{c0, c1}] = Piece{id, 0};
        edge_lut_[{c1, c0}] = Piece{id, 1};
    }

    // A well-formed target yields 3 distinct keys per corner and 2 per edge.
    valid_ = corner_lut_.size() == NUM_CORNERS * 3 &&
             edge_lut_.size() == NUM_EDGES * 2;
}

bool CubieEncoder::encode(const std::string& facelets, CubeState& out) const {
    if (!valid_ || facelets.size() != NUM_FACELETS) return false;

    bool corner_seen[NUM_CORNERS] = {false};
    bool edge_seen[NUM_EDGES] = {false};

    for (uint8_t s = 0; s < NUM_CORNERS; ++s) {
        const std::string key{
            facelets[CORNER_SLOTS[s][0]],
            facelets[CORNER_SLOTS[s][1]],
            facelets[CORNER_SLOTS[s][2]],
        };
        auto it = corner_lut_.find(key);
        if (it == corner_lut_.end()) return false;
        if (corner_seen[it->second.id]) return false;  // duplicate piece
        corner_seen[it->second.id] = true;
        out.corner_perm[s] = it->second.id;
        out.corner_orient[s] = it->second.ori;
    }

    for (uint8_t s = 0; s < NUM_EDGES; ++s) {
        const std::string key{
            facelets[EDGE_SLOTS[s][0]],
            facelets[EDGE_SLOTS[s][1]],
        };
        auto it = edge_lut_.find(key);
        if (it == edge_lut_.end()) return false;
        if (edge_seen[it->second.id]) return false;
        edge_seen[it->second.id] = true;
        out.edge_perm[s] = it->second.id;
        out.edge_orient[s] = it->second.ori;
    }

    return true;
}

std::string CubieEncoder::decode(const CubeState& state) const {
    if (!valid_) return std::string();

    std::string out(NUM_FACELETS, '?');

    // Centres are fixed by the orientation; copy them straight from the target.
    for (uint8_t f = 0; f < NUM_FACES; ++f) {
        const uint8_t centre = static_cast<uint8_t>(f * 9 + 4);
        out[centre] = target_[centre];
    }

    for (uint8_t s = 0; s < NUM_CORNERS; ++s) {
        const uint8_t id = state.corner_perm[s];
        const uint8_t ori = state.corner_orient[s];
        const char h[3] = {
            target_[CORNER_SLOTS[id][0]],
            target_[CORNER_SLOTS[id][1]],
            target_[CORNER_SLOTS[id][2]],
        };
        // Inverse of the encoder's shift: home colour k shows at tuple slot
        // (k + ori) % 3.
        for (uint8_t k = 0; k < 3; ++k) {
            out[CORNER_SLOTS[s][(k + ori) % 3]] = h[k];
        }
    }

    for (uint8_t s = 0; s < NUM_EDGES; ++s) {
        const uint8_t id = state.edge_perm[s];
        const uint8_t ori = state.edge_orient[s];
        const char h[2] = {
            target_[EDGE_SLOTS[id][0]],
            target_[EDGE_SLOTS[id][1]],
        };
        for (uint8_t k = 0; k < 2; ++k) {
            out[EDGE_SLOTS[s][(k + ori) % 2]] = h[k];
        }
    }

    return out;
}

} // namespace rubiks
