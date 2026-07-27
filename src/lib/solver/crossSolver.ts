import type { FaceColor, PhysicalOrientation } from '@/types';
import { bidirectionalBFS } from './bfsSolver';
import { getSolvedFacelets } from '../orientation';
import { makeCubieEncoder } from './cubieCoords';

// Cross pieces = the 4 D-layer edges: slot/piece ids 4 DR, 5 DF, 6 DL, 7 DB
const CROSS_EDGE_IDS = [4, 5, 6, 7];

export function solveCross(
  facelets: FaceColor[],
  orientation: PhysicalOrientation
): string[] {
  const targetFacelets = getSolvedFacelets(orientation);
  const encode = makeCubieEncoder(targetFacelets);

  if (!encode(facelets)) {
    console.warn('Cross solver: cube state has invalid stickers');
    return [];
  }

  // Piece-based congruent mask: where each cross edge sits and its flip.
  const mask = (f: FaceColor[]) => {
    const c = encode(f)!;
    let val = 0;
    for (const id of CROSS_EDGE_IDS) {
      const pos = c.ep.indexOf(id);
      val = val * 24 + pos * 2 + c.eo[pos];
    }
    return val;
  };

  // Cross can always be solved in 8 moves or fewer; 5 + 5 leaves margin.
  const maxDepth = 5;

  const solution = bidirectionalBFS({
    startFacelets: facelets,
    targetFacelets,
    mask,
    maxDepth,
  });

  return solution || [];
}
