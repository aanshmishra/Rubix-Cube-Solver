#include <emscripten/bind.h>

#include "../solver/solve_api.h"

using namespace emscripten;
using namespace rubiks;

// ---------------------------------------------------------------------------
// The JS boundary is deliberately narrow: facelet strings in, notation strings
// out. No vectors, no enums, no embind objects for the caller to free - which
// removes a whole class of leaks and marshalling bugs, and costs nothing since
// the crossing happens once per solve rather than once per search node.
// ---------------------------------------------------------------------------

EMSCRIPTEN_BINDINGS(rubiks_solver) {

    value_object<SolveOutput>("SolveOutput")
        .field("crossMoves", &SolveOutput::crossMoves)
        .field("f2lMoves", &SolveOutput::f2lMoves)
        .field("ollMoves", &SolveOutput::ollMoves)
        .field("pllMoves", &SolveOutput::pllMoves)
        .field("methodName", &SolveOutput::methodName)
        .field("success", &SolveOutput::success)
        .field("errorMessage", &SolveOutput::errorMessage);

    function("solveFacelets", &solveFacelets);
}
