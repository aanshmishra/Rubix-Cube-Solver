import { applyMoveToFacelets, getPrunedMoves, getPrunedMovesSubset, MoveName } from '../cubeEngine';
import type { FaceColor } from '@/types';

export type StateHash = bigint | number | string;

export interface BFSOptions {
  startFacelets: FaceColor[];
  targetFacelets: FaceColor[];
  mask: (facelets: FaceColor[]) => StateHash;
  maxDepth: number;
  allowedFaces?: string[];
  maxNodes?: number;
}

// Visited states store only a parent pointer and the incoming move; full move
// sequences are reconstructed once at the meeting point instead of being
// copied on every node expansion.
interface VisitedNode {
  parentHash: StateHash;
  move: MoveName | null;
}

interface QueueEntry {
  facelets: FaceColor[];
  hash: StateHash;
  lastMove?: MoveName;
}

function reconstructPath(
  hash: StateHash,
  startHash: StateHash,
  visited: Map<StateHash, VisitedNode>
): MoveName[] {
  const path: MoveName[] = [];
  let curr = hash;
  while (curr !== startHash) {
    const node = visited.get(curr);
    if (!node || node.move === null) break;
    path.push(node.move);
    curr = node.parentHash;
  }
  return path.reverse();
}

export function bidirectionalBFS({ startFacelets, targetFacelets, mask, maxDepth, allowedFaces, maxNodes = 2_000_000 }: BFSOptions): string[] | null {
  const startHash = mask(startFacelets);
  const targetHash = mask(targetFacelets);

  if (startHash === targetHash) {
    return [];
  }

  let generated = 0;

  const getMoves = (lastMove?: MoveName) =>
    allowedFaces ? getPrunedMovesSubset(allowedFaces, lastMove) : getPrunedMoves(lastMove);

  const forwardVisited = new Map<StateHash, VisitedNode>();
  const backwardVisited = new Map<StateHash, VisitedNode>();

  forwardVisited.set(startHash, { parentHash: startHash, move: null });
  backwardVisited.set(targetHash, { parentHash: targetHash, move: null });

  let forwardQueue: QueueEntry[] = [{ facelets: startFacelets, hash: startHash }];
  let backwardQueue: QueueEntry[] = [{ facelets: targetFacelets, hash: targetHash }];

  for (let depth = 1; depth <= maxDepth; depth++) {
    // Forward step
    const nextForwardQueue: QueueEntry[] = [];
    for (const { facelets, hash, lastMove } of forwardQueue) {
      for (const move of getMoves(lastMove)) {
        if (++generated > maxNodes) return null;
        const nextFacelets = applyMoveToFacelets(facelets, move);
        const nextHash = mask(nextFacelets);

        if (!forwardVisited.has(nextHash)) {
          forwardVisited.set(nextHash, { parentHash: hash, move });
          nextForwardQueue.push({ facelets: nextFacelets, hash: nextHash, lastMove: move });

          // Check intersection
          if (backwardVisited.has(nextHash)) {
            const fwdMoves = reconstructPath(nextHash, startHash, forwardVisited);
            const backMoves = reconstructPath(nextHash, targetHash, backwardVisited);
            return [...fwdMoves, ...inverseSequence(backMoves)];
          }
        }
      }
    }
    forwardQueue = nextForwardQueue;

    // Backward step
    const nextBackwardQueue: QueueEntry[] = [];
    for (const { facelets, hash, lastMove } of backwardQueue) {
      for (const move of getMoves(lastMove)) {
        if (++generated > maxNodes) return null;
        // Since we apply moves backwards, applying a normal move in backward search
        // implies the inverse move was applied in forward search.
        // We just apply the move normally to state, and record it.
        const nextFacelets = applyMoveToFacelets(facelets, move);
        const nextHash = mask(nextFacelets);

        if (!backwardVisited.has(nextHash)) {
          backwardVisited.set(nextHash, { parentHash: hash, move });
          nextBackwardQueue.push({ facelets: nextFacelets, hash: nextHash, lastMove: move });

          // Check intersection
          if (forwardVisited.has(nextHash)) {
            const fwdMoves = reconstructPath(nextHash, startHash, forwardVisited);
            const backMoves = reconstructPath(nextHash, targetHash, backwardVisited);
            return [...fwdMoves, ...inverseSequence(backMoves)];
          }
        }
      }
    }
    backwardQueue = nextBackwardQueue;
  }

  return null;
}

export function inverseMove(move: MoveName): MoveName {
  if (move.endsWith("'")) return move[0] as MoveName;
  if (move.endsWith("2")) return move;
  return (move + "'") as MoveName;
}

export function inverseSequence(moves: MoveName[]): MoveName[] {
  return [...moves].reverse().map(inverseMove);
}
