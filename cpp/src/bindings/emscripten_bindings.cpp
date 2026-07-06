#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <sstream>

#include "../core/cube_state.h"
#include "../core/validation.h"
#include "../core/facelet_converter.h"
#include "../solver/cross.h"
#include "../solver/method_dispatcher.h"
#include "../explorer/move_tree.h"

using namespace emscripten;
using namespace rubiks;

// ---------------------------------------------------------------------------
// Helper: convert vector<Move> to string notation
// ---------------------------------------------------------------------------
static std::string movesToString(const std::vector<Move>& moves) {
    std::string result;
    for (size_t i = 0; i < moves.size(); ++i) {
        if (i > 0) result += " ";
        result += moveToString(moves[i]);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Embind: Move enum
// ---------------------------------------------------------------------------
EMSCRIPTEN_BINDINGS(rubiks_bindings) {

    // Register vector<Move> to allow binding std::vector<Move>
    register_vector<Move>("MoveVector");

    // ---- Enums ----
    enum_<Move>("Move")
        .value("U", Move::U)
        .value("U2", Move::U2)
        .value("Up", Move::Up)
        .value("R", Move::R)
        .value("R2", Move::R2)
        .value("Rp", Move::Rp)
        .value("F", Move::F)
        .value("F2", Move::F2)
        .value("Fp", Move::Fp)
        .value("D", Move::D)
        .value("D2", Move::D2)
        .value("Dp", Move::Dp)
        .value("L", Move::L)
        .value("L2", Move::L2)
        .value("Lp", Move::Lp)
        .value("B", Move::B)
        .value("B2", Move::B2)
        .value("Bp", Move::Bp)
        .value("NONE", Move::NONE);

    enum_<Face>("Face")
        .value("U", Face::U)
        .value("R", Face::R)
        .value("F", Face::F)
        .value("D", Face::D)
        .value("L", Face::L)
        .value("B", Face::B);

    // ---- CubeState ----
    value_array<std::array<uint8_t, 8>>("CornerPermArray")
        .element(emscripten::index<0>())
        .element(emscripten::index<1>())
        .element(emscripten::index<2>())
        .element(emscripten::index<3>())
        .element(emscripten::index<4>())
        .element(emscripten::index<5>())
        .element(emscripten::index<6>())
        .element(emscripten::index<7>());

    value_array<std::array<uint8_t, 12>>("EdgePermArray")
        .element(emscripten::index<0>())
        .element(emscripten::index<1>())
        .element(emscripten::index<2>())
        .element(emscripten::index<3>())
        .element(emscripten::index<4>())
        .element(emscripten::index<5>())
        .element(emscripten::index<6>())
        .element(emscripten::index<7>())
        .element(emscripten::index<8>())
        .element(emscripten::index<9>())
        .element(emscripten::index<10>())
        .element(emscripten::index<11>());

    value_object<CubeState>("CubeState")
        .field("corner_perm", &CubeState::corner_perm)
        .field("corner_orient", &CubeState::corner_orient)
        .field("edge_perm", &CubeState::edge_perm)
        .field("edge_orient", &CubeState::edge_orient);

    function("cubeStateSolved", &CubeState::solved);
    function("cubeStateFromFacelets", optional_override([](const std::string& facelets, const std::string& orientation) {
        return faceletsToState(facelets, orientation);
    }));
    function("stateToFacelets", optional_override([](const CubeState& state, const std::string& orientation) {
        return rubiks::stateToFacelets(state, orientation);
    }));

    // ---- Validation ----
    value_object<ValidationResult>("ValidationResult")
        .field("valid", &ValidationResult::valid)
        .field("errorMessage", &ValidationResult::error_message);

    function("validateCubeState", &validateState);
    function("validateFacelets", &rubiks::validateFacelets);
    function("isCubeSolved", &isSolved);
    function("isCrossSolved", optional_override([](const CubeState& state) {
        return isCrossSolvedFast(state);
    }));
    function("isF2LSolved", &rubiks::isF2LSolved);
    function("isOLLSolved", &rubiks::isOLLSolved);

    // ---- Method Config ----
    value_object<MethodConfig>("MethodConfig")
        .field("f2lAdvanced", &MethodConfig::f2l_advanced)
        .field("ollAdvanced", &MethodConfig::oll_advanced)
        .field("pllAdvanced", &MethodConfig::pll_advanced);

    function("methodConfigCFOP", &MethodConfig::cfop);
    function("methodConfigBeginner", &MethodConfig::beginner);
    function("methodConfigName", optional_override([](const MethodConfig& c) {
        return c.getMethodName();
    }));

    // ---- Solution Result ----
    value_object<SolutionResult>("SolutionResult")
        .field("crossMoves", &SolutionResult::cross_moves)
        .field("f2lMoves", &SolutionResult::f2l_moves)
        .field("ollMoves", &SolutionResult::oll_moves)
        .field("pllMoves", &SolutionResult::pll_moves)
        .field("config", &SolutionResult::config)
        .field("success", &SolutionResult::success)
        .field("errorMessage", &SolutionResult::error_message);

    // ---- Solver ----
    class_<MethodDispatcher>("MethodDispatcher")
        .constructor<MethodConfig>()
        .function("solve", &MethodDispatcher::solve, allow_raw_pointers())
        .function("setConfig", &MethodDispatcher::setConfig)
        .function("getMethodName", &MethodDispatcher::getMethodName);

    // ---- Move utilities ----
    function("moveToString", &rubiks::moveToString);
    function("parseNotation", &parseNotation);
    function("notationToString", &movesToNotation);

    // ---- Move Tree Explorer ----
    class_<MoveTreeNode>("MoveTreeNode")
        .smart_ptr<std::shared_ptr<MoveTreeNode>>("MoveTreeNodePtr")
        .property("incomingMove", &MoveTreeNode::incoming_move)
        .property("depth", &MoveTreeNode::depth)
        .property("expanded", &MoveTreeNode::expanded)
        .property("stateHash", &MoveTreeNode::state_hash);

    class_<MoveTreeExplorer>("MoveTreeExplorer")
        .constructor<CubeState, bool>()
        .function("getRoot", &MoveTreeExplorer::getRoot)
        .function("expandNode", &MoveTreeExplorer::expandNode)
        .function("canExpand", &MoveTreeExplorer::canExpand)
        .function("setGraphMode", &MoveTreeExplorer::setGraphMode)
        .function("isGraphMode", &MoveTreeExplorer::isGraphMode)
        .function("getTotalNodeCount", &MoveTreeExplorer::getTotalNodeCount);

    // ---- Standalone solver function ----
    function("solveCube", optional_override([](const CubeState& state, const MethodConfig& config) {
        MethodDispatcher dispatcher(config);
        SolutionResult result = dispatcher.solve(state);
        return result;
    }));

    function("solveCrossOnly", &solveCross);

    // ---- Facelet helpers ----
    function("faceletsToState", &faceletsToState);
    function("validateFaceletSolvable", &validateFaceletSolvable);

    // ---- Constants ----
    constant("MAX_TREE_DEPTH", MAX_TREE_DEPTH);
    constant("NUM_CORNERS", NUM_CORNERS);
    constant("NUM_EDGES", NUM_EDGES);
}
