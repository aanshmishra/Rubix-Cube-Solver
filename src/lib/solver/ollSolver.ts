import type { FaceColor, PhysicalOrientation } from '@/types';
import { bidirectionalBFS } from './bfsSolver';
import { getSolvedFacelets } from '../orientation';
import { makeCubieEncoder } from './cubieCoords';

// First-two-layer pieces that must be back home when OLL completes:
// corners 4 DFR, 5 DLF, 6 DBL, 7 DRB; edges 4 DR..7 DB and 8 FR..11 BR
const F2L_CORNER_IDS = [4, 5, 6, 7];
const F2L_EDGE_IDS = [4, 5, 6, 7, 8, 9, 10, 11];

export async function solveOLL(
  facelets: FaceColor[],
  orientation: PhysicalOrientation
): Promise<string[]> {
  const targetFacelets = getSolvedFacelets(orientation);
  const encode = makeCubieEncoder(targetFacelets);

  if (!encode(facelets)) {
    console.warn('OLL solver: cube state has invalid stickers');
    return [];
  }

  // Congruent mask with the last-layer permutation left free:
  // - the per-slot orientation vector (twist/flip deltas depend only on the
  //   move and the slot, never on which piece sits there), and
  // - the positions of the F2L pieces, which must return home.
  // Goal hash = all orientations 0 + every F2L piece home, reached by the
  // backward search from the solved cube, matched by ANY U-layer permutation.
  const mask = (f: FaceColor[]) => {
    const c = encode(f)!;
    const cornerPos = F2L_CORNER_IDS.map((id) => c.cp.indexOf(id));
    const edgePos = F2L_EDGE_IDS.map((id) => c.ep.indexOf(id));
    return c.co.join('') + c.eo.join('') + '|' + cornerPos.join(',') + '|' + edgePos.join(',');
  };

  // OLL fits within 14 moves; bidirectional depth 7 + 7 = 14.
  const maxDepth = 7;

  const solution = bidirectionalBFS({
    startFacelets: facelets,
    targetFacelets,
    mask,
    maxDepth,
    allowedFaces: ['U', 'R', 'F'],
  });

  if (!solution) {
    console.warn(`OLL solver exhausted up to depth ${maxDepth * 2}`);
    return [];
  }

  return solution;
}
