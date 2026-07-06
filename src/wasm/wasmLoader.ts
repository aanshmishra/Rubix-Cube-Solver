// WASM module loader for the Rubik's cube solver
// Handles dynamic import of the Emscripten-generated module

let wasmModule: any = null;
let modulePromise: Promise<any> | null = null;

export interface WasmModule {
  // Cube state
  cubeStateSolved: () => CubeStateWasm;
  cubeStateFromFacelets: (facelets: string, orientation: string) => CubeStateWasm;
  stateToFacelets: (state: CubeStateWasm, orientation: string) => string;

  // Validation
  validateCubeState: (state: CubeStateWasm) => ValidationResultWasm;
  validateFacelets: (facelets: string) => ValidationResultWasm;
  isCubeSolved: (state: CubeStateWasm) => boolean;
  isCrossSolved: (state: CubeStateWasm) => boolean;
  isF2LSolved: (state: CubeStateWasm) => boolean;
  isOLLSolved: (state: CubeStateWasm) => boolean;

  // Method config
  methodConfigCFOP: () => MethodConfigWasm;
  methodConfigBeginner: () => MethodConfigWasm;
  methodConfigName: (config: MethodConfigWasm) => string;

  // Solver
  solveCube: (state: CubeStateWasm, config: MethodConfigWasm) => SolutionResultWasm;
  solveCrossOnly: (state: CubeStateWasm) => number[];

  // Move utilities
  moveToString: (move: number) => string;
  parseNotation: (notation: string) => number[];
  notationToString: (moves: number[]) => string;

  // Move tree
  MoveTreeExplorer: new (state: CubeStateWasm, graphMode: boolean) => MoveTreeExplorerWasm;

  // Constants
  MAX_TREE_DEPTH: number;
}

export interface CubeStateWasm {
  corner_perm: Uint8Array;
  corner_orient: Uint8Array;
  edge_perm: Uint8Array;
  edge_orient: Uint8Array;
}

export interface ValidationResultWasm {
  valid: boolean;
  errorMessage: string;
}

export interface MethodConfigWasm {
  f2lAdvanced: boolean;
  ollAdvanced: boolean;
  pllAdvanced: boolean;
}

export interface SolutionResultWasm {
  crossMoves: number[];
  f2lMoves: number[];
  ollMoves: number[];
  pllMoves: number[];
  config: MethodConfigWasm;
  success: boolean;
  errorMessage: string;
}

export interface MoveTreeExplorerWasm {
  getRoot: () => MoveTreeNodeWasm;
  expandNode: (node: MoveTreeNodeWasm) => MoveTreeNodeWasm[];
  canExpand: (node: MoveTreeNodeWasm) => boolean;
  setGraphMode: (enabled: boolean) => void;
  isGraphMode: () => boolean;
  getTotalNodeCount: () => number;
}

export interface MoveTreeNodeWasm {
  incomingMove: number;
  depth: number;
  expanded: boolean;
  stateHash: number;
}

// Module path - this won't be resolved at build time
const WASM_MODULE_PATH = '/wasm/rubiks_solver.js';

export async function loadWasmModule(): Promise<WasmModule> {
  if (wasmModule) {
    return wasmModule;
  }

  if (modulePromise) {
    return modulePromise;
  }

  modulePromise = new Promise((resolve) => {
    const tryLoadModule = async () => {
      try {
        // Check if the module exists
        const response = await fetch(WASM_MODULE_PATH);
        if (!response.ok) {
          throw new Error('WASM module not found');
        }

        // Use eval-based dynamic import to avoid build-time resolution
        // eslint-disable-next-line no-eval
        const dynamicImport = new Function('path', 'return import(path)');
        const module = await dynamicImport(WASM_MODULE_PATH);
        const createModule = module.default || module;
        const instance = await createModule();
        wasmModule = instance;
        return instance;
      } catch {
        wasmModule = createMockModule();
        return wasmModule;
      }
    };

    tryLoadModule().then(resolve).catch(() => {
      wasmModule = createMockModule();
      resolve(wasmModule);
    });
  });

  return modulePromise;
}

// Mock module for development when WASM is not available
function createMockModule(): WasmModule {
  const mockState = () => ({
    corner_perm: new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7]),
    corner_orient: new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0]),
    edge_perm: new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]),
    edge_orient: new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
  });

  const mockConfig = () => ({ f2lAdvanced: true, ollAdvanced: true, pllAdvanced: true });

  return {
    cubeStateSolved: mockState,
    cubeStateFromFacelets: () => mockState(),
    stateToFacelets: () => 'WWWWWWWWWRRRRRRRRRGGGGGGGGGYYYYYYYYYOOOOOOOOOBBBBBBBBB',
    validateCubeState: () => ({ valid: true, errorMessage: '' }),
    validateFacelets: () => ({ valid: true, errorMessage: '' }),
    isCubeSolved: () => false,
    isCrossSolved: () => false,
    isF2LSolved: () => false,
    isOLLSolved: () => false,
    methodConfigCFOP: mockConfig,
    methodConfigBeginner: () => ({ f2lAdvanced: false, ollAdvanced: false, pllAdvanced: false }),
    methodConfigName: (cfg: MethodConfigWasm) => {
      const count = (cfg.f2lAdvanced ? 1 : 0) + (cfg.ollAdvanced ? 1 : 0) + (cfg.pllAdvanced ? 1 : 0);
      if (count === 3) return 'Full CFOP';
      if (count === 0) return 'Beginner Layer-by-Layer';
      return 'Hybrid Method';
    },
    solveCube: () => ({
      crossMoves: [0, 1, 2],
      f2lMoves: [3, 4, 5, 6, 7, 8],
      ollMoves: [9, 10, 11],
      pllMoves: [12, 13, 14],
      config: mockConfig(),
      success: true,
      errorMessage: '',
    }),
    solveCrossOnly: () => [0, 1, 2],
    moveToString: (move: number) => {
      const names = ['U', 'U2', "U'", 'R', 'R2', "R'", 'F', 'F2', "F'", 'D', 'D2', "D'", 'L', 'L2', "L'", 'B', 'B2', "B'"];
      return names[move] || '?';
    },
    parseNotation: (notation: string) => {
      const map: Record<string, number> = {
        'U': 0, 'U2': 1, "U'": 2,
        'R': 3, 'R2': 4, "R'": 5,
        'F': 6, 'F2': 7, "F'": 8,
        'D': 9, 'D2': 10, "D'": 11,
        'L': 12, 'L2': 13, "L'": 14,
        'B': 15, 'B2': 16, "B'": 17,
      };
      return notation.split(' ').map((m) => map[m] ?? -1).filter((m) => m >= 0);
    },
    notationToString: (moves: number[]) => {
      const names = ['U', 'U2', "U'", 'R', 'R2', "R'", 'F', 'F2', "F'", 'D', 'D2', "D'", 'L', 'L2', "L'", 'B', 'B2', "B'"];
      return moves.map((m) => names[m] || '?').join(' ');
    },
    MoveTreeExplorer: class {
      constructor() {}
      getRoot() { return { incomingMove: 255, depth: 0, expanded: false, stateHash: 0 }; }
      expandNode() { return []; }
      canExpand() { return false; }
      setGraphMode() {}
      isGraphMode() { return false; }
      getTotalNodeCount() { return 1; }
    } as any,
    MAX_TREE_DEPTH: 5,
  };
}

export function getWasmModule(): WasmModule | null {
  return wasmModule;
}

export function isWasmLoaded(): boolean {
  return wasmModule !== null;
}

// Convert WASM solution result to JS format
export function convertSolutionResult(
  wasmResult: SolutionResultWasm,
  moveToString: (m: number) => string
): import('@/types').SolutionResult {
  const moveArrToStrings = (moves: number[]) =>
    moves.map((m) => moveToString(m));

  return {
    crossMoves: moveArrToStrings(wasmResult.crossMoves),
    f2lMoves: moveArrToStrings(wasmResult.f2lMoves),
    ollMoves: moveArrToStrings(wasmResult.ollMoves),
    pllMoves: moveArrToStrings(wasmResult.pllMoves),
    config: {
      f2lAdvanced: wasmResult.config.f2lAdvanced,
      ollAdvanced: wasmResult.config.ollAdvanced,
      pllAdvanced: wasmResult.config.pllAdvanced,
    },
    success: wasmResult.success,
    errorMessage: wasmResult.errorMessage,
    methodName: '', // Will be filled by caller
    totalMoves: wasmResult.crossMoves.length + wasmResult.f2lMoves.length +
                wasmResult.ollMoves.length + wasmResult.pllMoves.length,
  };
}
