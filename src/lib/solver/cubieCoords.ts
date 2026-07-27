import type { FaceColor } from '@/types';

// ---------------------------------------------------------------------------
// Facelet -> cubie (piece-level) encoding.
//
// BFS masks must be "congruent": if two states hash equally, they must keep
// hashing equally after any move. Colors sampled at fixed sticker positions
// do NOT have this property (different pieces can show the same colors at the
// masked positions and then diverge), which allows bidirectional BFS to merge
// forward/backward paths that don't actually connect. Tracking *pieces* -
// where each physical corner/edge sits and how it is twisted - is congruent,
// because a piece's trajectory under a move depends only on its own location.
// ---------------------------------------------------------------------------

// Facelet indices of each corner slot, ordered [U/D-face sticker, then
// clockwise around the corner]. Standard Kociemba ordering, consistent with
// cubeEngine's facelet geometry.
// Slot ids: 0 URF, 1 UFL, 2 ULB, 3 UBR, 4 DFR, 5 DLF, 6 DBL, 7 DRB
export const CORNER_SLOTS: ReadonlyArray<readonly [number, number, number]> = [
  [8, 9, 20],   // URF
  [6, 18, 38],  // UFL
  [0, 36, 47],  // ULB
  [2, 45, 11],  // UBR
  [29, 26, 15], // DFR
  [27, 44, 24], // DLF
  [33, 53, 42], // DBL
  [35, 17, 51], // DRB
];

// Facelet indices of each edge slot, ordered [primary sticker, secondary].
// Slot ids: 0 UR, 1 UF, 2 UL, 3 UB, 4 DR, 5 DF, 6 DL, 7 DB,
//           8 FR, 9 FL, 10 BL, 11 BR
export const EDGE_SLOTS: ReadonlyArray<readonly [number, number]> = [
  [5, 10],  // UR
  [7, 19],  // UF
  [3, 37],  // UL
  [1, 46],  // UB
  [32, 16], // DR
  [28, 25], // DF
  [30, 43], // DL
  [34, 52], // DB
  [23, 12], // FR
  [21, 41], // FL
  [50, 39], // BL
  [48, 14], // BR
];

export interface CubieState {
  // cp[slot] = which corner piece sits at this slot (piece id = its home slot)
  cp: number[];
  // co[slot] = clockwise twist of the piece at this slot (0..2)
  co: number[];
  ep: number[];
  eo: number[];
}

// Builds an encoder bound to a color scheme (the solved facelets for the
// user's physical orientation). Returns null for states whose stickers don't
// form a legal set of pieces (e.g. hand-painted impossible cubes).
export function makeCubieEncoder(
  target: FaceColor[]
): (facelets: FaceColor[]) => CubieState | null {
  // A corner twisted clockwise r times shows its home colors (c0,c1,c2)
  // cyclically shifted so that c0 appears at tuple index r.
  const cornerLookup = new Map<string, { id: number; ori: number }>();
  CORNER_SLOTS.forEach((slot, id) => {
    const [c0, c1, c2] = slot.map((i) => target[i]);
    cornerLookup.set(c0 + c1 + c2, { id, ori: 0 });
    cornerLookup.set(c2 + c0 + c1, { id, ori: 1 });
    cornerLookup.set(c1 + c2 + c0, { id, ori: 2 });
  });

  const edgeLookup = new Map<string, { id: number; ori: number }>();
  EDGE_SLOTS.forEach((slot, id) => {
    const [c0, c1] = slot.map((i) => target[i]);
    edgeLookup.set(c0 + c1, { id, ori: 0 });
    edgeLookup.set(c1 + c0, { id, ori: 1 });
  });

  return (facelets: FaceColor[]): CubieState | null => {
    const cp = new Array<number>(8);
    const co = new Array<number>(8);
    const ep = new Array<number>(12);
    const eo = new Array<number>(12);

    for (let s = 0; s < 8; s++) {
      const [a, b, c] = CORNER_SLOTS[s];
      const piece = cornerLookup.get(facelets[a] + facelets[b] + facelets[c]);
      if (!piece) return null;
      cp[s] = piece.id;
      co[s] = piece.ori;
    }

    for (let s = 0; s < 12; s++) {
      const [a, b] = EDGE_SLOTS[s];
      const piece = edgeLookup.get(facelets[a] + facelets[b]);
      if (!piece) return null;
      ep[s] = piece.id;
      eo[s] = piece.ori;
    }

    return { cp, co, ep, eo };
  };
}
