import { describe, expect, it } from 'vitest';
import {
  MOVE_NAMES,
  applyMoveToFacelets,
  applyNotationToFacelets,
  solvedFacelets,
} from './cubeEngine';

const INVERSES: Record<string, string> = {
  U: "U'",
  U2: 'U2',
  "U'": 'U',
  R: "R'",
  R2: 'R2',
  "R'": 'R',
  F: "F'",
  F2: 'F2',
  "F'": 'F',
  D: "D'",
  D2: 'D2',
  "D'": 'D',
  L: "L'",
  L2: 'L2',
  "L'": 'L',
  B: "B'",
  B2: 'B2',
  "B'": 'B',
};

describe('cubeEngine', () => {
  it('returns to solved after four applications of any face turn', () => {
    for (const move of MOVE_NAMES) {
      let facelets = solvedFacelets();

      for (let i = 0; i < 4; i++) {
        facelets = applyMoveToFacelets(facelets, move);
      }

      expect(facelets, move).toEqual(solvedFacelets());
    }
  });

  it('returns to solved after a move followed by its inverse', () => {
    for (const move of MOVE_NAMES) {
      const result = applyNotationToFacelets(solvedFacelets(), `${move} ${INVERSES[move]}`);
      expect(result, move).toEqual(solvedFacelets());
    }
  });

  it('applies a scramble and inverse sequence losslessly', () => {
    const scramble = "R U R' U' F2 L D B' R2";
    const inverse = "R2 B D' L' F2 U R U' R'";

    expect(applyNotationToFacelets(solvedFacelets(), `${scramble} ${inverse}`))
      .toEqual(solvedFacelets());
  });
});
