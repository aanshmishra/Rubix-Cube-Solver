// Cube face colors
export type FaceColor = 'W' | 'R' | 'G' | 'Y' | 'O' | 'B';

export const FACE_COLORS: Record<FaceColor, string> = {
  W: '#ffffff',
  R: '#b90000',
  G: '#009b48',
  Y: '#ffd500',
  O: '#ff5900',
  B: '#0045ad',
};

export const FACE_COLORS_DARK: Record<FaceColor, string> = {
  W: '#e8e8e8',
  R: '#dc2626',
  G: '#22c55e',
  Y: '#eab308',
  O: '#f97316',
  B: '#3b82f6',
};

// Face indices: U=0, R=1, F=2, D=3, L=4, B=5
export const FACE_NAMES = ['U', 'R', 'F', 'D', 'L', 'B'] as const;

// Centers define which color is on which face (solved: U=W, R=R, F=G, D=Y, L=O, B=B)
export const SOLVED_CENTERS: FaceColor[] = ['W', 'R', 'G', 'Y', 'O', 'B'];

// Method configuration
export interface MethodConfig {
  f2lAdvanced: boolean;
  ollAdvanced: boolean;
  pllAdvanced: boolean;
}

export function getMethodName(config: MethodConfig): string {
  const advancedCount = (config.f2lAdvanced ? 1 : 0) + (config.ollAdvanced ? 1 : 0) + (config.pllAdvanced ? 1 : 0);
  if (advancedCount === 3) return 'Full CFOP';
  if (advancedCount === 0) return 'Beginner Layer-by-Layer';
  const parts = [];
  parts.push(config.f2lAdvanced ? 'Adv-F2L' : 'Beg-F2L');
  parts.push(config.ollAdvanced ? 'Adv-OLL' : 'Beg-OLL');
  parts.push(config.pllAdvanced ? 'Adv-PLL' : 'Beg-PLL');
  return `Hybrid (${parts.join(' / ')})`;
}

// Solution stages
export interface SolutionResult {
  crossMoves: string[];
  f2lMoves: string[];
  ollMoves: string[];
  pllMoves: string[];
  config: MethodConfig;
  success: boolean;
  errorMessage: string;
  methodName: string;
  totalMoves: number;
}

// OLL/PLL case types
export interface AlgorithmCase {
  id: number;
  name: string;
  algorithm: string;
  difficulty: 'easy' | 'medium' | 'hard' | 'none';
  pattern?: string;
  cycle?: string;
  type?: string;
}

// History entry
export interface SolveHistoryEntry {
  id: string;
  timestamp: number;
  scramble: string;
  scrambleNotation: string;
  methodName: string;
  methodConfig: MethodConfig;
  totalMoves: number;
  crossMoves: number;
  f2lMoves: number;
  ollMoves: number;
  pllMoves: number;
  solutionNotation: string;
}

// Explorer node
export interface ExplorerNode {
  id: string;
  move: string;
  depth: number;
  moveCount: number;
  stateHash: number;
  expanded: boolean;
  children: string[];
  path: string[];
}

// Physical orientation
export type PhysicalOrientation = 'WG' | 'WB' | 'WR' | 'WO' | 'YG' | 'YB' | 'YR' | 'YO' |
  'GW' | 'GY' | 'GR' | 'GO' | 'BW' | 'BY' | 'BR' | 'BO' |
  'RW' | 'RY' | 'RG' | 'RB' | 'OW' | 'OY' | 'OG' | 'OB';

export interface OrientationOption {
  value: PhysicalOrientation;
  label: string;
  downColor: FaceColor;
  frontColor: FaceColor;
}

// Valid orientations: first char = down face color, second = front face color
export const ORIENTATION_OPTIONS: OrientationOption[] = [
  { value: 'WG', label: 'White Down, Green Front', downColor: 'W', frontColor: 'G' },
  { value: 'WB', label: 'White Down, Blue Front', downColor: 'W', frontColor: 'B' },
  { value: 'WR', label: 'White Down, Red Front', downColor: 'W', frontColor: 'R' },
  { value: 'WO', label: 'White Down, Orange Front', downColor: 'W', frontColor: 'O' },
  { value: 'YG', label: 'Yellow Down, Green Front', downColor: 'Y', frontColor: 'G' },
  { value: 'YB', label: 'Yellow Down, Blue Front', downColor: 'Y', frontColor: 'B' },
  { value: 'YR', label: 'Yellow Down, Red Front', downColor: 'Y', frontColor: 'R' },
  { value: 'YO', label: 'Yellow Down, Orange Front', downColor: 'Y', frontColor: 'O' },
  { value: 'GW', label: 'Green Down, White Front', downColor: 'G', frontColor: 'W' },
  { value: 'GY', label: 'Green Down, Yellow Front', downColor: 'G', frontColor: 'Y' },
  { value: 'RG', label: 'Red Down, Green Front', downColor: 'R', frontColor: 'G' },
  { value: 'RB', label: 'Red Down, Blue Front', downColor: 'R', frontColor: 'B' },
];
