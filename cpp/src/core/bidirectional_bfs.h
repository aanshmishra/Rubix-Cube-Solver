#pragma once

#include "cube_state.h"
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace rubiks {

// ---------------------------------------------------------------------------
// Bidirectional BFS (meet in the middle) over cube sub-problems.
//
// Search runs from the scramble and from the goal at the same time, one ply
// per side, alternating. When the frontiers collide the answer is
//     forward_path + reverse(inverse(backward_path))
// Cost drops from O(b^d) to O(2 * b^(d/2)).
//
// Each stage supplies a `mask` functor projecting a CubeState onto the part it
// cares about. The mask MUST be congruent: if two states mask equal, they must
// still mask equal after any move. Piece-level coordinates (which piece sits
// in which slot, and how it is twisted) have that property; sampled sticker
// colours do not, and a non-congruent mask lets the two frontiers "meet" at a
// junction that does not actually connect, producing a wrong solution.
//
// Strict alternation is deliberate: it makes the first collision found the
// shortest sequence for that stage's subgoal.
// ---------------------------------------------------------------------------

using Key64 = uint64_t;

// A wide key for the stages whose projection needs more than 64 bits. Rather
// than depend on the __int128 compiler extension (MinGW has no such type), the
// projection is split across two independent accumulators, each of which is
// packed in mixed radix and stays comfortably inside 64 bits. Equality is
// exact, so distinct projections still cannot collide.
struct Key128 {
    uint64_t a = 0;
    uint64_t b = 0;
    bool operator==(const Key128& o) const noexcept { return a == o.a && b == o.b; }
    bool operator!=(const Key128& o) const noexcept { return !(*this == o); }
};

struct KeyHash {
    size_t operator()(uint64_t k) const noexcept {
        k ^= k >> 33; k *= 0xff51afd7ed558ccdULL;
        k ^= k >> 33; k *= 0xc4ceb9fe1a85ec53ULL;
        k ^= k >> 33;
        return static_cast<size_t>(k);
    }
    size_t operator()(const Key128& k) const noexcept {
        return (*this)(k.a ^ (k.b * 0x9e3779b97f4a7c15ULL));
    }
};

struct BFSResult {
    std::vector<Move> moves;
    bool found = false;
    size_t nodes = 0;
};

template <typename Key, typename MaskFn>
BFSResult bidirectionalBFS(const CubeState& start,
                           const CubeState& goal,
                           MaskFn mask,
                           int max_depth,
                           const Move* move_set,
                           int num_moves,
                           size_t max_nodes = 3000000) {
    BFSResult result;

    const Key start_key = mask(start);
    const Key goal_key = mask(goal);
    if (start_key == goal_key) {
        result.found = true;
        return result;
    }

    struct Trace { Key parent; Move move; };
    struct Frontier { CubeState state; Key key; Move last; };

    std::unordered_map<Key, Trace, KeyHash> fwd_seen, bwd_seen;
    std::vector<Frontier> fwd{{start, start_key, Move::NONE}};
    std::vector<Frontier> bwd{{goal, goal_key, Move::NONE}};

    fwd_seen[start_key] = {start_key, Move::NONE};
    bwd_seen[goal_key] = {goal_key, Move::NONE};

    // Walks parent pointers back to the search root.
    auto trace_back = [](Key from, Key root,
                         const std::unordered_map<Key, Trace, KeyHash>& seen) {
        std::vector<Move> path;
        Key cur = from;
        while (cur != root) {
            auto it = seen.find(cur);
            if (it == seen.end() || it->second.move == Move::NONE) break;
            path.push_back(it->second.move);
            cur = it->second.parent;
        }
        std::reverse(path.begin(), path.end());
        return path;
    };

    // Joins the two half-paths. The backward half was built by applying moves
    // away from the goal, so it is inverted and reversed to run back into it.
    auto join = [&](Key meet) {
        std::vector<Move> out = trace_back(meet, start_key, fwd_seen);
        std::vector<Move> back = trace_back(meet, goal_key, bwd_seen);
        for (auto it = back.rbegin(); it != back.rend(); ++it) {
            out.push_back(inverseMove(*it));
        }
        return out;
    };

    for (int depth = 1; depth <= max_depth; ++depth) {
        for (int side = 0; side < 2; ++side) {
            const bool forward = (side == 0);
            std::vector<Frontier>& cur = forward ? fwd : bwd;
            auto& own = forward ? fwd_seen : bwd_seen;
            auto& other = forward ? bwd_seen : fwd_seen;

            std::vector<Frontier> next;
            next.reserve(cur.size() * 4);

            for (const Frontier& node : cur) {
                for (int i = 0; i < num_moves; ++i) {
                    const Move mv = move_set[i];
                    if (isRedundantAfter(mv, node.last)) continue;

                    if (++result.nodes > max_nodes) return result;

                    CubeState child = node.state;
                    child.applyMove(mv);
                    const Key child_key = mask(child);

                    if (own.count(child_key)) continue;
                    own[child_key] = {node.key, mv};

                    if (other.count(child_key)) {
                        result.moves = join(child_key);
                        result.found = true;
                        return result;
                    }

                    next.push_back({child, child_key, mv});
                }
            }
            cur.swap(next);
            if (cur.empty()) return result;  // subgoal unreachable
        }
    }

    return result;
}

} // namespace rubiks
