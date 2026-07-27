import { FaceColor, PhysicalOrientation } from '@/types';

// Standard opposites
const opposite: Record<FaceColor, FaceColor> = {
  W: 'Y', Y: 'W',
  G: 'B', B: 'G',
  R: 'O', O: 'R',
};

// Standard right face lookup: rightFace[Up][Front] = Right
// Only valid combinations are needed.
const rightFaceLUT: Record<FaceColor, Record<FaceColor, FaceColor>> = {
  W: { G: 'R', R: 'B', B: 'O', O: 'G' } as Record<FaceColor, FaceColor>,
  Y: { G: 'O', O: 'B', B: 'R', R: 'G' } as Record<FaceColor, FaceColor>,
  G: { W: 'O', O: 'Y', Y: 'R', R: 'W' } as Record<FaceColor, FaceColor>,
  B: { W: 'R', R: 'Y', Y: 'O', O: 'W' } as Record<FaceColor, FaceColor>,
  R: { W: 'G', G: 'Y', Y: 'B', B: 'W' } as Record<FaceColor, FaceColor>,
  O: { W: 'B', B: 'Y', Y: 'G', G: 'W' } as Record<FaceColor, FaceColor>,
};

/**
 * Returns the 6 face colors [U, R, F, D, L, B] for a given orientation.
 * The orientation string is "DF" (Down, Front).
 */
export function getOrientationColors(orientation: PhysicalOrientation): FaceColor[] {
  const D = orientation[0] as FaceColor;
  const F = orientation[1] as FaceColor;
  
  const U = opposite[D];
  const B = opposite[F];
  
  const R = rightFaceLUT[U][F];
  const L = opposite[R];
  
  return [U, R, F, D, L, B];
}

/**
 * Returns the 54-element facelets array for a perfectly solved cube in the given orientation.
 */
export function getSolvedFacelets(orientation: PhysicalOrientation): FaceColor[] {
  const colors = getOrientationColors(orientation);
  const facelets: FaceColor[] = [];
  for (const c of colors) {
    for (let i = 0; i < 9; i++) {
      facelets.push(c);
    }
  }
  return facelets;
}
