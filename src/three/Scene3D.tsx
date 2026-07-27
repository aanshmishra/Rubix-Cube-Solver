import { useRef, useEffect, useMemo, useState } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import * as THREE from 'three';

interface Scene3DProps {
  moves?: string[];
  scramble?: string;
  animationDuration?: number; // ms per move animation
  onAnimationComplete?: () => void;
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

type CubieData = { x: number; y: number; z: number; faces: Record<string, string> };
type CubeState = Map<string, CubieData>;

// Apply a move to the cube state (represented as 26 cubies with their face colors)
function applyMoveToState(state: CubeState, move: string): void {
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
          if (f.U) newFaces.B = f.U;
          if (f.B) newFaces.D = f.B;
          if (f.D) newFaces.F = f.D;
          if (f.F) newFaces.U = f.F;
        } else {
          if (f.U) newFaces.F = f.U;
          if (f.F) newFaces.D = f.F;
          if (f.D) newFaces.B = f.D;
          if (f.B) newFaces.U = f.B;
        }
      }

      state.set(cubie.key, { x: newX, y: newY, z: newZ, faces: newFaces });
    });
  }
}

// Clone a cube state Map
function cloneState(state: CubeState): CubeState {
  const cloned = new Map<string, CubieData>();
  state.forEach((v, k) => {
    cloned.set(k, { x: v.x, y: v.y, z: v.z, faces: { ...v.faces } });
  });
  return cloned;
}

// Build initial solved cube state
function buildSolvedState(): CubeState {
  const state: CubeState = new Map();
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

// Get the rotation axis vector and target angle for a move
function getMoveRotation(move: string): { axis: THREE.Vector3; angle: number } {
  const face = move[0];
  const modifier = move.slice(1);

  let axisVec: THREE.Vector3;
  let baseAngle: number;

  // The rotation direction: positive angle = counter-clockwise around axis (right-hand rule)
  // For standard Rubik's notation, clockwise when looking at the face means:
  switch (face) {
    case 'U': axisVec = new THREE.Vector3(0, 1, 0); baseAngle = -Math.PI / 2; break;
    case 'D': axisVec = new THREE.Vector3(0, 1, 0); baseAngle = Math.PI / 2; break;
    case 'R': axisVec = new THREE.Vector3(1, 0, 0); baseAngle = -Math.PI / 2; break;
    case 'L': axisVec = new THREE.Vector3(1, 0, 0); baseAngle = Math.PI / 2; break;
    case 'F': axisVec = new THREE.Vector3(0, 0, 1); baseAngle = -Math.PI / 2; break;
    case 'B': axisVec = new THREE.Vector3(0, 0, 1); baseAngle = Math.PI / 2; break;
    default: axisVec = new THREE.Vector3(0, 1, 0); baseAngle = 0; break;
  }

  if (modifier === "'") baseAngle = -baseAngle;
  if (modifier === '2') baseAngle *= 2;

  return { axis: axisVec, angle: baseAngle };
}

// Get which cubies are in the rotating layer for a move
function getLayerFilter(move: string): { axis: 'x' | 'y' | 'z'; layer: number } {
  const face = move[0];
  switch (face) {
    case 'U': return { axis: 'y', layer: 1 };
    case 'D': return { axis: 'y', layer: -1 };
    case 'R': return { axis: 'x', layer: 1 };
    case 'L': return { axis: 'x', layer: -1 };
    case 'F': return { axis: 'z', layer: 1 };
    case 'B': return { axis: 'z', layer: -1 };
    default: return { axis: 'y', layer: 0 };
  }
}

// Easing function for smooth animation (ease-in-out cubic)
function easeInOutCubic(t: number): number {
  return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
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

interface AnimationState {
  move: string;
  axis: THREE.Vector3;
  targetAngle: number;
  layerAxis: 'x' | 'y' | 'z';
  layerValue: number;
  progress: number; // 0 to 1
}

function Cube({
  moves = [],
  scramble,
  animationDuration = 350,
  onAnimationComplete,
}: {
  moves: string[];
  scramble?: string;
  animationDuration?: number;
  onAnimationComplete?: () => void;
}) {
  const groupRef = useRef<THREE.Group>(null);
  const rotatingGroupRef = useRef<THREE.Group>(null);

  // Track the last committed move count — how many moves have been "baked" into baseState
  const committedCountRef = useRef(0);
  const [baseState, setBaseState] = useState<CubeState>(() => buildSolvedState());
  const [animation, setAnimation] = useState<AnimationState | null>(null);
  const animationRef = useRef<AnimationState | null>(null);
  const onCompleteRef = useRef(onAnimationComplete);
  onCompleteRef.current = onAnimationComplete;

  // Build the initial scrambled state whenever scramble changes
  useEffect(() => {
    const state = buildSolvedState();
    if (scramble) {
      const scrambleMoves = parseScrambleToMoves(scramble);
      for (const m of scrambleMoves) {
        if (m) applyMoveToState(state, m);
      }
    }
    setBaseState(state);
    committedCountRef.current = 0;
    setAnimation(null);
    animationRef.current = null;
  }, [scramble]);

  // When moves array changes, figure out what to animate
  useEffect(() => {
    const committed = committedCountRef.current;
    const target = moves.length;

    if (target === committed) {
      // No change needed
      return;
    }

    if (target < committed) {
      // Stepping backwards — rebuild state from scratch up to target
      const state = buildSolvedState();
      if (scramble) {
        const scrambleMoves = parseScrambleToMoves(scramble);
        for (const m of scrambleMoves) {
          if (m) applyMoveToState(state, m);
        }
      }
      for (let i = 0; i < target; i++) {
        if (moves[i]) applyMoveToState(state, moves[i]);
      }
      setBaseState(state);
      committedCountRef.current = target;
      setAnimation(null);
      animationRef.current = null;
      return;
    }

    // Forward: animate the next move (target > committed)
    // First, if we skipped moves (e.g. jumped forward), fast-apply intermediate ones
    if (target > committed + 1) {
      const state = cloneState(baseState);
      for (let i = committed; i < target - 1; i++) {
        if (moves[i]) applyMoveToState(state, moves[i]);
      }
      setBaseState(state);
      committedCountRef.current = target - 1;
    }

    // Now animate the last move
    const moveToAnimate = moves[target - 1];
    if (!moveToAnimate) {
      committedCountRef.current = target;
      return;
    }

    const { axis, angle } = getMoveRotation(moveToAnimate);
    const layerInfo = getLayerFilter(moveToAnimate);

    const anim: AnimationState = {
      move: moveToAnimate,
      axis,
      targetAngle: angle,
      layerAxis: layerInfo.axis,
      layerValue: layerInfo.layer,
      progress: 0,
    };
    setAnimation(anim);
    animationRef.current = anim;
  }, [moves, scramble]); // eslint-disable-line react-hooks/exhaustive-deps

  // Animate the rotation in useFrame
  useFrame((_, delta) => {
    const anim = animationRef.current;
    if (!anim || !rotatingGroupRef.current) return;

    const durationSec = animationDuration / 1000;
    const newProgress = Math.min(anim.progress + delta / durationSec, 1);
    anim.progress = newProgress;

    // Apply eased rotation
    const easedProgress = easeInOutCubic(newProgress);
    const currentAngle = anim.targetAngle * easedProgress;

    rotatingGroupRef.current.rotation.set(0, 0, 0);
    rotatingGroupRef.current.rotateOnAxis(anim.axis, currentAngle);

    if (newProgress >= 1) {
      // Animation complete — commit the move to baseState
      const newState = cloneState(baseState);
      applyMoveToState(newState, anim.move);
      committedCountRef.current += 1;

      setBaseState(newState);
      setAnimation(null);
      animationRef.current = null;

      // Reset rotating group
      rotatingGroupRef.current.rotation.set(0, 0, 0);

      // Notify parent
      if (onCompleteRef.current) {
        onCompleteRef.current();
      }
    }
  });

  // Split cubies into static and rotating based on current animation
  const { staticCubies, rotatingCubies } = useMemo(() => {
    const allCubies = Array.from(baseState.entries()).map(([key, data]) => ({ key, ...data }));

    if (!animation) {
      return { staticCubies: allCubies, rotatingCubies: [] };
    }

    const statics: typeof allCubies = [];
    const rotating: typeof allCubies = [];

    for (const cubie of allCubies) {
      if (Math.abs(cubie[animation.layerAxis] - animation.layerValue) < 0.1) {
        rotating.push(cubie);
      } else {
        statics.push(cubie);
      }
    }

    return { staticCubies: statics, rotatingCubies: rotating };
  }, [baseState, animation]);

  return (
    <group ref={groupRef}>
      {/* Static cubies (not in the rotating layer) */}
      {staticCubies.map((cubie) => (
        <Cubie
          key={cubie.key}
          position={[cubie.x, cubie.y, cubie.z]}
          faces={cubie.faces}
        />
      ))}

      {/* Rotating layer group — this group gets animated */}
      <group ref={rotatingGroupRef}>
        {rotatingCubies.map((cubie) => (
          <Cubie
            key={cubie.key}
            position={[cubie.x, cubie.y, cubie.z]}
            faces={cubie.faces}
          />
        ))}
      </group>
    </group>
  );
}

export default function Scene3D({ moves = [], scramble, animationDuration = 350, onAnimationComplete }: Scene3DProps) {
  return (
    <Canvas camera={{ position: [4, 4, 4], fov: 35 }}>
      <ambientLight intensity={0.8} />
      <directionalLight position={[5, 5, 5]} intensity={1} />
      <directionalLight position={[-3, -3, -3]} intensity={0.3} />
      <Cube
        moves={moves}
        scramble={scramble}
        animationDuration={animationDuration}
        onAnimationComplete={onAnimationComplete}
      />
      <OrbitControls enablePan={false} />
    </Canvas>
  );
}
