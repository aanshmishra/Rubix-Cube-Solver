#pragma once

#include <string>

namespace rubiks {

// Flat, string-in / string-out result so the JS boundary stays trivial:
// no vectors, no enums, no embind memory management on the caller's side.
struct SolveOutput {
    std::string crossMoves;   // space-separated notation, may be empty
    std::string f2lMoves;
    std::string ollMoves;
    std::string pllMoves;
    std::string methodName;
    bool success = false;
    std::string errorMessage;
};

// `scrambled` and `target` are both 54-character facelet strings. `target` is
// a solved cube in the user's chosen physical orientation, which is what tells
// the solver the colour scheme - there is no orientation logic in C++.
SolveOutput solveFacelets(const std::string& scrambled,
                          const std::string& target,
                          bool f2l_advanced,
                          bool oll_advanced,
                          bool pll_advanced);

} // namespace rubiks
