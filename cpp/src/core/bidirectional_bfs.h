#pragma once

#include "cube_state.h"
#include <queue>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <vector>

namespace rubiks {

// ---------------------------------------------------------------------------
// Bidirectional BFS (Meet-in-the-Middle) for Rubik's Cube sub-problems.
//
// Instead of searching from start to goal in one direction, we search from
// both ends simultaneously. At each level we expand the smaller frontier
// first. When the two frontiers collide (a projected hash appears in both
// visited sets) we reconstruct:
//     forward_moves + reverse(backward_moves)
//
// This dramatically reduces the search space: instead of O(b^d), we get
// O(2 * b^(d/2)), where b is the branching factor and d the depth.
// ---------------------------------------------------------------------------

struct BidiBFSConfig {
    const Move* moves;
    int num_moves;
    std::function<uint64_t(const CubeState&)> hash_fn;
    int max_depth = 12;
    int max_iterations = 200000;
};

// Node stored in the BFS visited maps
struct BFSNode {
    CubeState state;
    uint64_t parent;
    Move move;
    int depth;
};

// Helper to reconstruct path from start to meeting point
inline std::vector<Move> reconstructPath(uint64_t hash, uint64_t start_hash, const std::unordered_map<uint64_t, BFSNode>& visited) {
    std::vector<Move> path;
    uint64_t curr = hash;
    while (curr != start_hash) {
        auto it = visited.find(curr);
        if (it == visited.end()) break;
        path.push_back(it->second.move);
        curr = it->second.parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// Returns the solution move sequence, or empty vector if not found.
inline std::vector<Move> bidirectionalBFS(
    const CubeState& start,
    const CubeState& goal,
    const BidiBFSConfig& config
) {
    uint64_t start_hash = config.hash_fn(start);
    uint64_t goal_hash = config.hash_fn(goal);

    if (start_hash == goal_hash) {
        return {};
    }

    std::unordered_map<uint64_t, BFSNode> forward_visited;
    std::unordered_map<uint64_t, BFSNode> backward_visited;

    std::queue<uint64_t> forward_queue;
    std::queue<uint64_t> backward_queue;

    forward_visited[start_hash] = {start, start_hash, Move::NONE, 0};
    forward_queue.push(start_hash);

    backward_visited[goal_hash] = {goal, goal_hash, Move::NONE, 0};
    backward_queue.push(goal_hash);

    int iterations = 0;
    int total_depth = 0;

    while (!forward_queue.empty() && !backward_queue.empty() &&
           total_depth <= config.max_depth &&
           iterations < config.max_iterations) {

        bool expand_forward = (forward_queue.size() <= backward_queue.size());

        auto& active_queue = expand_forward ? forward_queue : backward_queue;
        auto& active_visited = expand_forward ? forward_visited : backward_visited;
        auto& other_visited = expand_forward ? backward_visited : forward_visited;
        uint64_t active_start = expand_forward ? start_hash : goal_hash;
        uint64_t other_start = expand_forward ? goal_hash : start_hash;

        int level_size = static_cast<int>(active_queue.size());

        for (int n = 0; n < level_size && iterations < config.max_iterations; ++n) {
            iterations++;

            uint64_t current_hash = active_queue.front();
            active_queue.pop();

            auto it = active_visited.find(current_hash);
            if (it == active_visited.end()) continue;

            const CubeState& current_state = it->second.state;
            int current_depth = it->second.depth;

            if (current_depth > config.max_depth) continue;

            for (int mi = 0; mi < config.num_moves; ++mi) {
                Move move = config.moves[mi];

                CubeState next = current_state;
                next.applyMove(move);

                uint64_t next_hash = config.hash_fn(next);

                auto collision_it = other_visited.find(next_hash);
                if (collision_it != other_visited.end()) {
                    std::vector<Move> active_path = reconstructPath(current_hash, active_start, active_visited);
                    active_path.push_back(move);
                    
                    std::vector<Move> other_path = reconstructPath(next_hash, other_start, other_visited);

                    if (expand_forward) {
                        std::vector<Move> result = active_path;
                        for (int i = static_cast<int>(other_path.size()) - 1; i >= 0; --i) {
                            result.push_back(inverseMove(other_path[i]));
                        }
                        return result;
                    } else {
                        std::vector<Move> result = other_path;
                        for (int i = static_cast<int>(active_path.size()) - 1; i >= 0; --i) {
                            result.push_back(inverseMove(active_path[i]));
                        }
                        return result;
                    }
                }

                if (active_visited.find(next_hash) == active_visited.end()) {
                    active_visited[next_hash] = {next, current_hash, move, current_depth + 1};
                    active_queue.push(next_hash);
                }
            }
        }

        total_depth++;
    }

    // No solution found within limits
    return {};
}

} // namespace rubiks
