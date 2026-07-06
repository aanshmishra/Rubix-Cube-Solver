#pragma once

#include "../core/cube_state.h"
#include <vector>
#include <string>

namespace rubiks {

// Solve the cross (first layer cross) using BFS over the 4-edge state space.
// Returns a move sequence that solves the cross on the D face (white center down).
// The BFS explores states of just the 4 cross edges (DF, DR, DB, DL),
// which is a very small state space (~12! / 8! = ~12k states).
std::vector<Move> solveCross(const CubeState& state);

// Check if the cross is already solved
bool isCrossSolvedFast(const CubeState& state);

} // namespace rubiks
