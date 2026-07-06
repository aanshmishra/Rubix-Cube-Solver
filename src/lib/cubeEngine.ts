import type { FaceColor } from '@/types';

export const MOVE_NAMES = [
  'U', 'U2', "U'",
  'R', 'R2', "R'",
  'F', 'F2', "F'",
  'D', 'D2', "D'",
  'L', 'L2', "L'",
  'B', 'B2', "B'",
] as const;

export type MoveName = (typeof MOVE_NAMES)[number];

const SOLVED_FACELETS = 'WWWWWWWWWRRRRRRRRRGGGGGGGGGYYYYYYYYYOOOOOOOOOBBBBBBBBB';

type Vec3 = readonly [number, number, number];

interface Sticker {
  pos: Vec3;
  normal: Vec3;
}

function faceletToSticker(index: number): Sticker {
  const face = Math.floor(index / 9);
  const offset = index % 9;
  const row = Math.floor(offset / 3);
  const col = offset % 3;

  switch (face) {
    case 0: return { pos: [col - 1, 1, row - 1], normal: [0, 1, 0] }; // U
    case 1: return { pos: [1, 1 - row, 1 - col], normal: [1, 0, 0] }; // R
    case 2: return { pos: [col - 1, 1 - row, 1], normal: [0, 0, 1] }; // F
    case 3: return { pos: [col - 1, -1, 1 - row], normal: [0, -1, 0] }; // D
    case 4: return { pos: [-1, 1 - row, col - 1], normal: [-1, 0, 0] }; // L
    case 5: return { pos: [1 - col, 1 - row, -1], normal: [0, 0, -1] }; // B
    default: throw new Error(`Invalid facelet index: ${index}`);
  }
}

function stickerToFacelet({ pos, normal }: Sticker): number {
  const [x, y, z] = pos;
  const [nx, ny, nz] = normal;

  if (ny === 1) return (z + 1) * 3 + (x + 1);
  if (nx === 1) return 9 + (1 - y) * 3 + (1 - z);
  if (nz === 1) return 18 + (1 - y) * 3 + (x + 1);
  if (ny === -1) return 27 + (1 - z) * 3 + (x + 1);
  if (nx === -1) return 36 + (1 - y) * 3 + (z + 1);
  if (nz === -1) return 45 + (1 - y) * 3 + (1 - x);

  throw new Error(`Invalid sticker normal: ${normal.join(',')}`);
}

function rotateVec([x, y, z]: Vec3, axis: 'x' | 'y' | 'z', turns: number): Vec3 {
  const normalizedTurns = ((turns % 4) + 4) % 4;
  let next: Vec3 = [x, y, z];

  for (let i = 0; i < normalizedTurns; i++) {
    const [cx, cy, cz] = next;
    if (axis === 'x') next = [cx, -cz, cy];
    if (axis === 'y') next = [cz, cy, -cx];
    if (axis === 'z') next = [-cy, cx, cz];
  }

  return next;
}

const MOVE_SPECS: Record<string, { axis: 'x' | 'y' | 'z'; layer: number; turns: number }> = {
  U: { axis: 'y', layer: 1, turns: -1 },
  R: { axis: 'x', layer: 1, turns: -1 },
  F: { axis: 'z', layer: 1, turns: -1 },
  D: { axis: 'y', layer: -1, turns: 1 },
  L: { axis: 'x', layer: -1, turns: 1 },
  B: { axis: 'z', layer: -1, turns: 1 },
};

const FACE_PRIORITIES: Record<string, number> = {
  U: 6,
  R: 5,
  F: 4,
  D: 3,
  L: 2,
  B: 1,
};

const FACELET_PERMUTATIONS = MOVE_NAMES.reduce<Record<MoveName, number[]>>((perms, move) => {
  const face = move[0];
  const suffix = move.slice(1);
  const base = MOVE_SPECS[face];
  const amount = suffix === '2' ? 2 : suffix === "'" ? -1 : 1;
  const turns = base.turns * amount;
  const permutation = Array.from({ length: 54 }, (_, sourceIndex) => {
    const sticker = faceletToSticker(sourceIndex);
    const axisCoord = base.axis === 'x' ? sticker.pos[0] : base.axis === 'y' ? sticker.pos[1] : sticker.pos[2];

    if (axisCoord !== base.layer) {
      return sourceIndex;
    }

    return stickerToFacelet({
      pos: rotateVec(sticker.pos, base.axis, turns),
      normal: rotateVec(sticker.normal, base.axis, turns),
    });
  });

  perms[move] = permutation;
  return perms;
}, {} as Record<MoveName, number[]>);

export function parseNotation(notation: string): MoveName[] {
  return notation
    .trim()
    .split(/\s+/)
    .filter(Boolean)
    .filter((token): token is MoveName => MOVE_NAMES.includes(token as MoveName));
}

export function applyMoveToFacelets(facelets: FaceColor[], move: MoveName): FaceColor[] {
  const next = [...facelets];
  FACELET_PERMUTATIONS[move].forEach((targetIndex, sourceIndex) => {
    next[targetIndex] = facelets[sourceIndex];
  });
  return next;
}

export function applyNotationToFacelets(facelets: FaceColor[], notation: string): FaceColor[] {
  return parseNotation(notation).reduce(
    (current, move) => applyMoveToFacelets(current, move),
    [...facelets],
  );
}

export function solvedFacelets(): FaceColor[] {
  return SOLVED_FACELETS.split('') as FaceColor[];
}

export function hashFacelets(facelets: FaceColor[]): number {
  let hash = 2166136261;
  for (const color of facelets) {
    hash ^= color.charCodeAt(0);
    hash = Math.imul(hash, 16777619);
  }
  return hash >>> 0;
}

export function isSameFace(a: string, b: string): boolean {
  return a[0] === b[0];
}

export function isOppositeFace(a: string, b: string): boolean {
  const pair = `${a[0]}${b[0]}`;
  return pair === 'UD' || pair === 'DU' ||
    pair === 'RL' || pair === 'LR' ||
    pair === 'FB' || pair === 'BF';
}

function facePriority(move: string): number {
  return FACE_PRIORITIES[move[0]] ?? 0;
}

export function getPrunedMoves(previousMove?: string): MoveName[] {
  if (!previousMove) return [...MOVE_NAMES];

  return MOVE_NAMES.filter((move) => {
    if (isSameFace(move, previousMove)) return false;
    if (isOppositeFace(move, previousMove) && facePriority(move) < facePriority(previousMove)) {
      return false;
    }
    return true;
  });
}
