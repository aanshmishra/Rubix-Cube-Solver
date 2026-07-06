import { create } from 'zustand';
import type {
  FaceColor,
  MethodConfig,
  SolutionResult,
  PhysicalOrientation,
} from '@/types';
import { applyNotationToFacelets, solvedFacelets } from '@/lib/cubeEngine';

// Default facelet state (solved cube)
// Face order: U, R, F, D, L, B - each face is 9 stickers
const SOLVED_FACELETS: FaceColor[] = solvedFacelets();

// Generate a random scramble (sequence of moves)
export function generateScramble(length: number = 25): string {
  const faces = ['U', 'D', 'L', 'R', 'F', 'B'];
  const modifiers = ['', "'", '2'];
  const moves: string[] = [];
  let lastFace = '';

  for (let i = 0; i < length; i++) {
    let face: string;
    do {
      face = faces[Math.floor(Math.random() * faces.length)];
    } while (face === lastFace);
    lastFace = face;
    const mod = modifiers[Math.floor(Math.random() * modifiers.length)];
    moves.push(face + mod);
  }

  return moves.join(' ');
}

// Apply scramble to facelets
export function applyScrambleToFacelets(
  facelets: FaceColor[],
  scramble: string,
  _orientation: PhysicalOrientation
): FaceColor[] {
  return applyNotationToFacelets(facelets, scramble);
}

const INITIAL_ORIENTATION: PhysicalOrientation = 'WG';
const INITIAL_SCRAMBLE = generateScramble();

export interface CubeStore {
  // Facelet state (54 stickers)
  facelets: FaceColor[];
  setFacelets: (facelets: FaceColor[]) => void;

  // Scramble notation
  scrambleNotation: string;
  setScrambleNotation: (notation: string) => void;
  generateNewScramble: () => void;

  // Physical orientation
  orientation: PhysicalOrientation;
  setOrientation: (orientation: PhysicalOrientation) => void;

  // Method configuration
  methodConfig: MethodConfig;
  setMethodConfig: (config: MethodConfig) => void;
  toggleF2L: () => void;
  toggleOLL: () => void;
  togglePLL: () => void;

  // Solution
  solution: SolutionResult | null;
  setSolution: (solution: SolutionResult | null) => void;

  // Solving state
  isSolving: boolean;
  setIsSolving: (solving: boolean) => void;

  // Active face for editing
  editingFace: number | null;
  setEditingFace: (face: number | null) => void;

  // Selected color for painting
  selectedColor: FaceColor;
  setSelectedColor: (color: FaceColor) => void;

  // Reset to solved
  resetSolved: () => void;

  // Apply scramble
  applyScramble: () => void;
}

export const useCubeStore = create<CubeStore>((set, get) => ({
  facelets: applyScrambleToFacelets(SOLVED_FACELETS, INITIAL_SCRAMBLE, INITIAL_ORIENTATION),
  setFacelets: (facelets) => set({ facelets }),

  scrambleNotation: INITIAL_SCRAMBLE,
  setScrambleNotation: (scrambleNotation) => set({ scrambleNotation }),
  generateNewScramble: () => {
    const scrambleNotation = generateScramble();
    set({
      scrambleNotation,
      facelets: applyScrambleToFacelets(SOLVED_FACELETS, scrambleNotation, get().orientation),
      solution: null,
    });
  },

  orientation: INITIAL_ORIENTATION,
  setOrientation: (orientation) => set({ orientation }),

  methodConfig: { f2lAdvanced: true, ollAdvanced: true, pllAdvanced: true },
  setMethodConfig: (methodConfig) => set({ methodConfig }),
  toggleF2L: () => set((s) => ({
    methodConfig: { ...s.methodConfig, f2lAdvanced: !s.methodConfig.f2lAdvanced },
  })),
  toggleOLL: () => set((s) => ({
    methodConfig: { ...s.methodConfig, ollAdvanced: !s.methodConfig.ollAdvanced },
  })),
  togglePLL: () => set((s) => ({
    methodConfig: { ...s.methodConfig, pllAdvanced: !s.methodConfig.pllAdvanced },
  })),

  solution: null,
  setSolution: (solution) => set({ solution }),

  isSolving: false,
  setIsSolving: (isSolving) => set({ isSolving }),

  editingFace: null,
  setEditingFace: (editingFace) => set({ editingFace }),

  selectedColor: 'W',
  setSelectedColor: (selectedColor) => set({ selectedColor }),

  resetSolved: () => set({ facelets: [...SOLVED_FACELETS], scrambleNotation: '', solution: null }),

  applyScramble: () => {
    const { scrambleNotation, orientation } = get();
    set({
      facelets: applyScrambleToFacelets(SOLVED_FACELETS, scrambleNotation, orientation),
      solution: null,
    });
  },
}));
