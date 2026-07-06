#include "move_tree.h"
#include <algorithm>

namespace rubiks {

// ---------------------------------------------------------------------------
// MoveTreeExplorer
// ---------------------------------------------------------------------------

MoveTreeExplorer::MoveTreeExplorer(const CubeState& root_state, bool graph_mode)
    : graph_mode_(graph_mode), node_count_(1) {
    root_ = std::make_shared<MoveTreeNode>(root_state, Move::NONE, std::vector<Move>{}, 0);

    if (graph_mode_) {
        canonical_nodes_[root_->state_hash] = root_;
    }

    nodes_by_depth_.resize(MAX_TREE_DEPTH + 1);
    nodes_by_depth_[0].push_back(root_);
}

std::vector<MoveTreeNodePtr> MoveTreeExplorer::expandNode(MoveTreeNodePtr node) {
    if (!node || node->depth >= MAX_TREE_DEPTH || node->expanded) {
        return {};
    }

    std::vector<Move> valid_moves = getValidMoves(node);
    std::vector<MoveTreeNodePtr> new_children;

    for (Move move : valid_moves) {
        CubeState new_state = node->state;
        new_state.applyMove(move);

        size_t new_hash = new_state.hash();

        // In graph mode, check if this state already exists
        if (graph_mode_) {
            auto it = canonical_nodes_.find(new_hash);
            if (it != canonical_nodes_.end()) {
                // Link to existing node instead of creating new
                node->children.push_back(it->second);
                continue;
            }
        }

        std::vector<Move> new_path = node->path;
        new_path.push_back(move);

        auto child = std::make_shared<MoveTreeNode>(new_state, move, new_path, node->depth + 1);
        child->state_hash = new_hash;

        node->children.push_back(child);
        new_children.push_back(child);
        node_count_++;

        if (graph_mode_) {
            canonical_nodes_[new_hash] = child;
        }

        if (node->depth + 1 <= MAX_TREE_DEPTH) {
            nodes_by_depth_[node->depth + 1].push_back(child);
        }
    }

    node->expanded = true;
    return new_children;
}

bool MoveTreeExplorer::canExpand(MoveTreeNodePtr node) const {
    if (!node) return false;
    if (node->depth >= MAX_TREE_DEPTH) return false;
    if (node->expanded) return false;
    return true;
}

std::vector<MoveTreeNodePtr> MoveTreeExplorer::getNodesAtDepth(uint8_t depth) {
    if (depth > MAX_TREE_DEPTH) return {};
    if (depth >= nodes_by_depth_.size()) return {};
    return nodes_by_depth_[depth];
}

void MoveTreeExplorer::setGraphMode(bool enabled) {
    if (graph_mode_ == enabled) return;
    graph_mode_ = enabled;

    if (graph_mode_) {
        // Rebuild canonical map
        canonical_nodes_.clear();
        std::function<void(MoveTreeNodePtr)> traverse;
        traverse = [&](MoveTreeNodePtr node) {
            if (!node) return;
            canonical_nodes_[node->state_hash] = node;
            for (auto& child : node->children) {
                // In graph mode, children might point back to existing nodes
                // Only traverse if it's actually a tree child
                if (child->depth == node->depth + 1) {
                    traverse(child);
                }
            }
        };
        traverse(root_);
    } else {
        canonical_nodes_.clear();
    }
}

// ---------------------------------------------------------------------------
// Move pruning
// ---------------------------------------------------------------------------

std::vector<Move> MoveTreeExplorer::getValidMoves(MoveTreeNodePtr node) {
    static const Move ALL_MOVES[18] = {
        Move::U, Move::U2, Move::Up,
        Move::R, Move::R2, Move::Rp,
        Move::F, Move::F2, Move::Fp,
        Move::D, Move::D2, Move::Dp,
        Move::L, Move::L2, Move::Lp,
        Move::B, Move::B2, Move::Bp,
    };

    if (node->path.empty()) {
        // Root: all moves are valid
        return std::vector<Move>(ALL_MOVES, ALL_MOVES + 18);
    }

    Move last_move = node->path.back();

    std::vector<Move> valid;
    valid.reserve(15);

    for (Move move : ALL_MOVES) {
        // Prune: no same-face moves in a row (e.g., R followed by R')
        if (areSameFace(move, last_move)) {
            continue;
        }

        // Prune: no opposite-face commuting orderings
        // If last move was R, don't do L next (L R == R L, so just explore one ordering)
        if (areOppositeFaces(move, last_move)) {
            // Only allow one ordering: enforce face priority (U>D, R>L, F>B)
            uint8_t mi = static_cast<uint8_t>(move);
            uint8_t li = static_cast<uint8_t>(last_move);
            Face mf = moveFace(move);
            Face lf = moveFace(last_move);

            // Skip if the new move's face has lower priority than last move's face
            uint8_t mp = 0, lp = 0;
            switch (mf) {
                case Face::U: mp = 6; break;
                case Face::R: mp = 5; break;
                case Face::F: mp = 4; break;
                case Face::D: mp = 3; break;
                case Face::L: mp = 2; break;
                case Face::B: mp = 1; break;
                default: break;
            }
            switch (lf) {
                case Face::U: lp = 6; break;
                case Face::R: lp = 5; break;
                case Face::F: lp = 4; break;
                case Face::D: lp = 3; break;
                case Face::L: lp = 2; break;
                case Face::B: lp = 1; break;
                default: break;
            }

            if (mp < lp) {
                continue;
            }
        }

        valid.push_back(move);
    }

    return valid;
}

bool MoveTreeExplorer::areOppositeFaces(Move a, Move b) {
    Face fa = moveFace(a);
    Face fb = moveFace(b);

    return (fa == Face::U && fb == Face::D) ||
           (fa == Face::D && fb == Face::U) ||
           (fa == Face::R && fb == Face::L) ||
           (fa == Face::L && fb == Face::R) ||
           (fa == Face::F && fb == Face::B) ||
           (fa == Face::B && fb == Face::F);
}

bool MoveTreeExplorer::areSameFace(Move a, Move b) {
    return moveFace(a) == moveFace(b);
}

} // namespace rubiks
