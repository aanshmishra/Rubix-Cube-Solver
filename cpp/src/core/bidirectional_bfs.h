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
    std::vector<Move> path;
};

// Returns the solution move sequence, or empty vector if not found.
inline std::vector<Move> bidirectionalBFS(
    const CubeState& start,
    const CubeState& goal,
    const BidiBFSConfig& config
) {
    // Quick check: is start already at goal (under the projected hash)?
    uint64_t start_hash = config.hash_fn(start);
    uint64_t goal_hash = config.hash_fn(goal);

    if (start_hash == goal_hash) {
        return {};
    }

    // Forward visited: hash -> BFSNode (state + path from start)
    std::unordered_map<uint64_t, BFSNode> forward_visited;
    // Backward visited: hash -> BFSNode (state + path from goal)
    std::unordered_map<uint64_t, BFSNode> backward_visited;

    // BFS frontiers (queues of hashes to expand from)
    std::queue<uint64_t> forward_queue;
    std::queue<uint64_t> backward_queue;

    // Seed the forward search
    forward_visited[start_hash] = {start, {}};
    forward_queue.push(start_hash);

    // Seed the backward search
    backward_visited[goal_hash] = {goal, {}};
    backward_queue.push(goal_hash);

    int iterations = 0;
    int total_depth = 0;

    while (!forward_queue.empty() && !backward_queue.empty() &&
           total_depth <= config.max_depth &&
           iterations < config.max_iterations) {

        // Expand one full level from the smaller frontier
        bool expand_forward = (forward_queue.size() <= backward_queue.size());

        auto& active_queue = expand_forward ? forward_queue : backward_queue;
        auto& active_visited = expand_forward ? forward_visited : backward_visited;
        auto& other_visited = expand_forward ? backward_visited : forward_visited;

        // Process all nodes at the current level
        int level_size = static_cast<int>(active_queue.size());

        for (int n = 0; n < level_size && iterations < config.max_iterations; ++n) {
            iterations++;

            uint64_t current_hash = active_queue.front();
            active_queue.pop();

            auto it = active_visited.find(current_hash);
            if (it == active_visited.end()) continue;

            const CubeState& current_state = it->second.state;
            const std::vector<Move>& current_path = it->second.path;

            // Skip if this path is already too deep for one direction
            if (static_cast<int>(current_path.size()) > config.max_depth) continue;

            for (int mi = 0; mi < config.num_moves; ++mi) {
                Move move = config.moves[mi];

                CubeState next = current_state;
                next.applyMove(move);

                uint64_t next_hash = config.hash_fn(next);

                // Check for collision with the other frontier
                auto collision_it = other_visited.find(next_hash);
                if (collision_it != other_visited.end()) {
                    // Found a meeting point! Reconstruct the full path.
                    std::vector<Move> new_path = current_path;
                    new_path.push_back(move);

                    const std::vector<Move>& other_path = collision_it->second.path;

                    if (expand_forward) {
                        // forward_path + reversed(backward_path)
                        std::vector<Move> result = new_path;
                        for (int i = static_cast<int>(other_path.size()) - 1; i >= 0; --i) {
                            result.push_back(inverseMove(other_path[i]));
                        }
                        return result;
                    } else {
                        // We expanded backward, so:
                        // other_path (forward) + reversed(new_path)
                        const std::vector<Move>& fwd_path = collision_it->second.path;
                        std::vector<Move> result = fwd_path;
                        // new_path is the backward path including the current move
                        for (int i = static_cast<int>(new_path.size()) - 1; i >= 0; --i) {
                            result.push_back(inverseMove(new_path[i]));
                        }
                        return result;
                    }
                }

                // If not visited in this direction, add to frontier
                if (active_visited.find(next_hash) == active_visited.end()) {
                    std::vector<Move> new_path = current_path;
                    new_path.push_back(move);
                    active_visited[next_hash] = {next, new_path};
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
