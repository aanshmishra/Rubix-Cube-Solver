import { applyNotationToFacelets } from '../cubeEngine';
import type { FaceColor, PhysicalOrientation } from '@/types';
import { bidirectionalBFS } from './bfsSolver';
import { getSolvedFacelets } from '../orientation';
import { makeCubieEncoder } from './cubieCoords';

// Cross pieces = the 4 D-layer edges (piece ids 4..7); they must stay solved
// throughout F2L, so they are always part of the mask.
const CROSS_EDGE_IDS = [4, 5, 6, 7];

// F2L pairs: [corner piece id, edge piece id]
// corners: 4 DFR, 5 DLF, 6 DBL, 7 DRB; edges: 8 FR, 9 FL, 10 BL, 11 BR
const F2L_PAIRS: ReadonlyArray<readonly [number, number]> = [
  [4, 8],
  [5, 9],
  [6, 10],
  [7, 11],
];

export function solveF2L(
  facelets: FaceColor[],
  orientation: PhysicalOrientation
): string[] {
  let currentFacelets = [...facelets];
  const targetFacelets = getSolvedFacelets(orientation);
  const encode = makeCubieEncoder(targetFacelets);
  const solutionMoves: string[] = [];

  if (!encode(facelets)) {
    console.warn('F2L solver: cube state has invalid stickers');
    return [];
  }

  // Piece ids tracked so far (cross + already-inserted pairs + current pair)
  const trackedCorners: number[] = [];
  const trackedEdges: number[] = [...CROSS_EDGE_IDS];

  for (let i = 0; i < F2L_PAIRS.length; i++) {
    trackedCorners.push(F2L_PAIRS[i][0]);
    trackedEdges.push(F2L_PAIRS[i][1]);

    // Piece-based congruent mask: position + orientation of every tracked piece.
    const mask = (f: FaceColor[]) => {
      const c = encode(f)!;
      const parts: number[] = [];
      for (const id of trackedCorners) {
        const pos = c.cp.indexOf(id);
        parts.push(pos * 3 + c.co[pos]);
      }
      for (const id of trackedEdges) {
        const pos = c.ep.indexOf(id);
        parts.push(pos * 2 + c.eo[pos]);
      }
      return parts.join(',');
    };

    // A single F2L pair fits in 11 moves; bidirectional 7 + 7 = 14 has margin.
    const maxDepth = 7;

    const pairSolution = bidirectionalBFS({
      startFacelets: currentFacelets,
      targetFacelets,
      mask,
      maxDepth,
    });

    if (pairSolution) {
      solutionMoves.push(...pairSolution);
      currentFacelets = applyNotationToFacelets(currentFacelets, pairSolution.join(' '));
    } else {
      console.warn(`F2L Pair ${i + 1} not solved within depth ${maxDepth * 2}`);
    }
  }

  return solutionMoves;
}
