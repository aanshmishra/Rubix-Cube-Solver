import { useRef, useEffect, useMemo } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import * as THREE from 'three';

interface Scene3DProps {
  moves?: string[];
  scramble?: string;
}

// Standard Rubik's cube sticker colors
const STICKER_COLORS: Record<string, string> = {
  U: '#ffffff',  // Pure white
  D: '#ffd500',  // Bright yellow
  F: '#009b48',  // Official green
  B: '#0045ad',  // Official blue
  R: '#b90000',  // Official red
  L: '#ff5900',  // Official orange
};

// Apply a move to the cube state (represented as 27 cubies with their face colors)
function applyMoveToState(state: Map<string, { x: number; y: number; z: number; faces: Record<string, string> }>, move: string): void {
  // Parse move
  const face = move[0];
  const modifier = move.slice(1); // '', "'", or '2'

  // Determine rotation axis and layer
  let axis: 'x' | 'y' | 'z';
  let layer: number;
  let clockwise: boolean;

  switch (face) {
    case 'U': axis = 'y'; layer = 1; clockwise = true; break;
    case 'D': axis = 'y'; layer = -1; clockwise = false; break;
    case 'F': axis = 'z'; layer = 1; clockwise = true; break;
    case 'B': axis = 'z'; layer = -1; clockwise = false; break;
    case 'R': axis = 'x'; layer = 1; clockwise = true; break;
    case 'L': axis = 'x'; layer = -1; clockwise = false; break;
    default: return;
  }

  if (modifier === "'") clockwise = !clockwise;
  const turns = modifier === '2' ? 2 : 1;

  for (let t = 0; t < turns; t++) {
    // Find cubies in the rotating layer
    const layerCubies: { key: string; x: number; y: number; z: number; faces: Record<string, string> }[] = [];

    state.forEach((cubie, key) => {
      if (Math.abs(cubie[axis] - layer) < 0.1) {
        layerCubies.push({ key, ...cubie });
      }
    });

    // Rotate positions
    layerCubies.forEach((cubie) => {
      let newX = cubie.x;
      let newY = cubie.y;
      let newZ = cubie.z;
      let newFaces: Record<string, string>;

      if (axis === 'y') {
        // U/D rotation: rotate in XZ plane
        if (clockwise) {
          [newX, newZ] = [-newZ, newX];
        } else {
          [newX, newZ] = [newZ, -newX];
        }
        // Rotate face colors
        const f = { ...cubie.faces }; // snapshot of originals
        newFaces = {}; // start fresh
        // Copy non-rotating faces
        if (f.U) newFaces.U = f.U;
        if (f.D) newFaces.D = f.D;
        // Rotate the lateral faces
        if (clockwise) {
          if (f.F) newFaces.L = f.F;
          if (f.L) newFaces.B = f.L;
          if (f.B) newFaces.R = f.B;
          if (f.R) newFaces.F = f.R;
        } else {
          if (f.F) newFaces.R = f.F;
          if (f.R) newFaces.B = f.R;
          if (f.B) newFaces.L = f.B;
          if (f.L) newFaces.F = f.L;
        }
      } else if (axis === 'z') {
        // F/B rotation: rotate in XY plane
        if (clockwise) {
          [newX, newY] = [newY, -newX];
        } else {
          [newX, newY] = [-newY, newX];
        }
        const f = { ...cubie.faces }; // snapshot of originals
        newFaces = {}; // start fresh
        // Copy non-rotating faces
        if (f.F) newFaces.F = f.F;
        if (f.B) newFaces.B = f.B;
        // Rotate the lateral faces
        if (clockwise) {
          if (f.U) newFaces.R = f.U;
          if (f.R) newFaces.D = f.R;
          if (f.D) newFaces.L = f.D;
          if (f.L) newFaces.U = f.L;
        } else {
          if (f.U) newFaces.L = f.U;
          if (f.L) newFaces.D = f.L;
          if (f.D) newFaces.R = f.D;
          if (f.R) newFaces.U = f.R;
        }
      } else {
        // R/L rotation: rotate in YZ plane
        if (clockwise) {
          [newY, newZ] = [newZ, -newY];
        } else {
          [newY, newZ] = [-newZ, newY];
        }
        const f = { ...cubie.faces }; // snapshot of originals
        newFaces = {}; // start fresh
        // Copy non-rotating faces
        if (f.R) newFaces.R = f.R;
        if (f.L) newFaces.L = f.L;
        // Rotate the lateral faces
        if (clockwise) {
          if (f.U) newFaces.F = f.U;
          if (f.F) newFaces.D = f.F;
          if (f.D) newFaces.B = f.D;
          if (f.B) newFaces.U = f.B;
        } else {
          if (f.U) newFaces.B = f.U;
          if (f.B) newFaces.D = f.B;
          if (f.D) newFaces.F = f.D;
          if (f.F) newFaces.U = f.F;
        }
      }

      state.set(cubie.key, { x: newX, y: newY, z: newZ, faces: newFaces });
    });
  }
}

// Build initial solved cube state
function buildSolvedState(): Map<string, { x: number; y: number; z: number; faces: Record<string, string> }> {
  const state = new Map();
  const positions = [-1, 0, 1];

  for (const x of positions) {
    for (const y of positions) {
      for (const z of positions) {
        // Skip center cubie (invisible)
        if (x === 0 && y === 0 && z === 0) continue;

        const faces: Record<string, string> = {};
        if (y === 1) faces.U = 'U';
        if (y === -1) faces.D = 'D';
        if (z === 1) faces.F = 'F';
        if (z === -1) faces.B = 'B';
        if (x === 1) faces.R = 'R';
        if (x === -1) faces.L = 'L';

        const key = `${x},${y},${z}`;
        state.set(key, { x, y, z, faces });
      }
    }
  }

  return state;
}

// Parse a scramble notation string into individual moves
function parseScrambleToMoves(scramble: string): string[] {
  return scramble.trim().split(/\s+/).filter(m => m.length > 0);
}

interface CubieProps {
  position: [number, number, number];
  faces: Record<string, string>;
}

function Cubie({ position, faces }: CubieProps) {
  const meshRef = useRef<THREE.Mesh>(null);

  // Create materials for each face
  const materials = useMemo(() => {
    const faceOrder = ['R', 'L', 'U', 'D', 'F', 'B'];
    return faceOrder.map((face) => {
      const color = faces[face] ? STICKER_COLORS[faces[face]] : '#1a1a1a';
      return new THREE.MeshStandardMaterial({
        color,
        roughness: 0.3,
        metalness: 0.1,
      });
    });
  }, [faces]);

  return (
    <mesh ref={meshRef} position={position} material={materials}>
      <boxGeometry args={[0.95, 0.95, 0.95]} />
    </mesh>
  );
}

function Cube({ moves = [], scramble }: { moves: string[]; scramble?: string }) {
  const groupRef = useRef<THREE.Group>(null);

  // Build and apply moves to state
  const cubieState = useMemo(() => {
    const state = buildSolvedState();

    // First apply scramble moves (so the cube starts scrambled)
    if (scramble) {
      const scrambleMoves = parseScrambleToMoves(scramble);
      for (const move of scrambleMoves) {
        if (move) applyMoveToState(state, move);
      }
    }

    // Then apply solution moves up to the current index (progressively solving)
    for (const move of moves) {
      if (move) applyMoveToState(state, move);
    }
    return state;
  }, [moves, scramble]);

  // Convert state to cubie array
  const cubies = useMemo(() => {
    return Array.from(cubieState.values());
  }, [cubieState]);

  // Auto-rotate
  useFrame((_, delta) => {
    if (groupRef.current) {
      groupRef.current.rotation.y += delta * 0.2;
    }
  });

  return (
    <group ref={groupRef}>
      {cubies.map((cubie, i) => (
        <Cubie
          key={i}
          position={[cubie.x, cubie.y, cubie.z]}
          faces={cubie.faces}
        />
      ))}
    </group>
  );
}

export default function Scene3D({ moves = [], scramble }: Scene3DProps) {
  return (
    <Canvas camera={{ position: [4, 4, 4], fov: 35 }}>
      <ambientLight intensity={0.8} />
      <directionalLight position={[5, 5, 5]} intensity={1} />
      <directionalLight position={[-3, -3, -3]} intensity={0.3} />
      <Cube moves={moves} scramble={scramble} />
      <OrbitControls enablePan={false} />
    </Canvas>
  );
}
