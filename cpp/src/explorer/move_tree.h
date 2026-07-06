#pragma once

#include "../core/cube_state.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace rubiks {

constexpr uint8_t MAX_TREE_DEPTH = 5;

// A node in the move tree
struct MoveTreeNode {
    CubeState state;
    Move incoming_move;       // The move that led to this state from parent
    std::vector<Move> path;   // Full move sequence from root to this node
    uint8_t depth;
    std::vector<std::shared_ptr<MoveTreeNode>> children;
    bool expanded = false;

    // For graph mode: canonical hash of the state
    size_t state_hash;

    MoveTreeNode(const CubeState& s, Move m, const std::vector<Move>& p, uint8_t d)
        : state(s), incoming_move(m), path(p), depth(d), state_hash(s.hash()) {}
};

using MoveTreeNodePtr = std::shared_ptr<MoveTreeNode>;

// Move tree explorer with lazy expansion
class MoveTreeExplorer {
public:
    MoveTreeExplorer(const CubeState& root_state, bool graph_mode = false);

    // Get the root node
    MoveTreeNodePtr getRoot() const { return root_; }

    // Lazily expand a node's children
    // Returns the newly created child nodes
    std::vector<MoveTreeNodePtr> expandNode(MoveTreeNodePtr node);

    // Check if a node can be expanded further
    bool canExpand(MoveTreeNodePtr node) const;

    // Get all nodes at a specific depth (useful for UI rendering)
    std::vector<MoveTreeNodePtr> getNodesAtDepth(uint8_t depth);

    // Enable/disable graph mode
    void setGraphMode(bool enabled);
    bool isGraphMode() const { return graph_mode_; }

    // Get statistics
    int getTotalNodeCount() const { return node_count_; }

private:
    MoveTreeNodePtr root_;
    bool graph_mode_;
    int node_count_;

    // In graph mode: map from state hash to canonical node
    std::unordered_map<size_t, MoveTreeNodePtr> canonical_nodes_;

    // Track all nodes for depth queries
    std::vector<std::vector<MoveTreeNodePtr>> nodes_by_depth_;

    // Prune moves: don't generate same-face repeated moves
    // or commuting opposite-face orderings
    std::vector<Move> getValidMoves(MoveTreeNodePtr node);

    // Check if two moves are on opposite faces
    static bool areOppositeFaces(Move a, Move b);

    // Check if two moves are on the same face
    static bool areSameFace(Move a, Move b);
};

} // namespace rubiks
