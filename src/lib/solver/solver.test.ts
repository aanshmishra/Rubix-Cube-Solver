import { describe, expect, it } from 'vitest';
import { applyNotationToFacelets, solvedFacelets } from '../cubeEngine';
import { getSolvedFacelets } from '../orientation';
import { solveCross } from './crossSolver';
import { solveF2L } from './f2lSolver';
import { solveOLL } from './ollSolver';
import { solvePLL } from './pllSolver';
import type { PhysicalOrientation } from '@/types';

// solvedFacelets() has U=W, R=R, F=G, i.e. Yellow down / Green front
const ORIENTATION: PhysicalOrientation = 'YG';

const FIXED_SCRAMBLES = [
  "R U R' U' F2 L D B' R2",
  "D2 F' L2 B R U2 L' D R' F U B2",
  "U F2 R' D' L B U2 R F' D2 L2 U'",
  "B' R2 U L' F D' B2 U2 R L F2 D",
  "L U' B2 D R' F L2 B U' R2 D' F'",
  "F R U' L' D2 B R2 U F2 L D' B'",
  "R2 D L' B' U F2 R D2 L U' B F",
  "U' L2 B R D' F' U2 L B2 R' D F2",
  // Superflip: every edge in place but flipped - a classic worst-case state
  "U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2",
];

// Deterministic PRNG (mulberry32) so failures are reproducible
function mulberry32(seed: number) {
  return () => {
    seed |= 0;
    seed = (seed + 0x6d2b79f5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

function randomScramble(rand: () => number, length: number): string {
  const faces = ['U', 'R', 'F', 'D', 'L', 'B'];
  const suffixes = ['', "'", '2'];
  const moves: string[] = [];
  let prevFace = '';
  while (moves.length < length) {
    const face = faces[Math.floor(rand() * 6)];
    if (face === prevFace) continue;
    moves.push(face + suffixes[Math.floor(rand() * 3)]);
    prevFace = face;
  }
  return moves.join(' ');
}

const rand = mulberry32(20260707);
const RANDOM_SCRAMBLES = Array.from({ length: 20 }, () => randomScramble(rand, 25));

const SCRAMBLES = [...FIXED_SCRAMBLES, ...RANDOM_SCRAMBLES];

describe('full solver pipeline', () => {
  it('sanity: orientation YG matches the engine solved facelets', () => {
    expect(getSolvedFacelets(ORIENTATION)).toEqual(solvedFacelets());
  });

  for (const scramble of SCRAMBLES) {
    it(`solves scramble "${scramble}"`, { timeout: 30_000 }, async () => {
      let facelets = applyNotationToFacelets(solvedFacelets(), scramble);

      const crossMoves = solveCross(facelets, ORIENTATION);
      facelets = applyNotationToFacelets(facelets, crossMoves.join(' '));

      const f2lMoves = solveF2L(facelets, ORIENTATION);
      facelets = applyNotationToFacelets(facelets, f2lMoves.join(' '));

      const ollMoves = await solveOLL(facelets, ORIENTATION);
      facelets = applyNotationToFacelets(facelets, ollMoves.join(' '));

      const pllMoves = await solvePLL(facelets, ORIENTATION);
      facelets = applyNotationToFacelets(facelets, pllMoves.join(' '));

      expect(facelets).toEqual(getSolvedFacelets(ORIENTATION));
    });
  }
});
