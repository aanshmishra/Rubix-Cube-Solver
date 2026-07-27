import { applyNotationToFacelets } from '../cubeEngine';
import type { FaceColor, PhysicalOrientation } from '@/types';
import { bidirectionalBFS } from './bfsSolver';
import { getSolvedFacelets } from '../orientation';
import { makeCubieEncoder } from './cubieCoords';

// F2L edges (ids 4 DR..7 DB, 8 FR..11 BR) must stay home during PLL;
// U-layer edges (ids 0..3) are free during the corner stage.
const F2L_EDGE_IDS = [4, 5, 6, 7, 8, 9, 10, 11];

// PLL as two bidirectional BFS stages. A single exact search needs >14 moves
// in <U,R,F> for some cases (e.g. G-perm-like states), which blows up the
// frontier. Splitting it keeps both searches shallow:
//   1. permute corners, leaving the U-edge order free (much shorter goals)
//   2. permute the remaining U edges - an EPLL state, always solvable inside
//      the small <R,U> subgroup, so the search can afford depth 9 + 9.
export async function solvePLL(
  facelets: FaceColor[],
  orientation: PhysicalOrientation
): Promise<string[]> {
  const targetFacelets = getSolvedFacelets(orientation);
  const encode = makeCubieEncoder(targetFacelets);

  if (!encode(facelets)) {
    console.warn('PLL solver: cube state has invalid stickers');
    return [];
  }

  // Stage 1 - corners home, orientations intact, F2L edges home; U-edge
  // permutation left out of the mask so the search may scramble it freely.
  const cornerMask = (f: FaceColor[]) => {
    const c = encode(f)!;
    const f2lEdgePos = F2L_EDGE_IDS.map((id) => c.ep.indexOf(id));
    return c.cp.join('') + c.co.join('') + c.eo.join('') + '|' + f2lEdgePos.join(',');
  };

  const cornerMoves = bidirectionalBFS({
    startFacelets: facelets,
    targetFacelets,
    mask: cornerMask,
    maxDepth: 7,
    allowedFaces: ['U', 'R', 'F'],
  });

  if (!cornerMoves) {
    console.warn('PLL corner stage exhausted up to depth 14');
    return [];
  }

  const afterCorners = applyNotationToFacelets(facelets, cornerMoves.join(' '));

  // Stage 2 - exact solve of the remaining edge permutation.
  const edgeMoves = bidirectionalBFS({
    startFacelets: afterCorners,
    targetFacelets,
    mask: (f: FaceColor[]) => f.join(''),
    maxDepth: 9,
    allowedFaces: ['U', 'R'],
  });

  if (!edgeMoves) {
    console.warn('PLL edge stage exhausted up to depth 18');
    return cornerMoves;
  }

  return [...cornerMoves, ...edgeMoves];
}
