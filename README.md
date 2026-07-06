# Rubik's Cube Solver

A browser-based Rubik's Cube solver and learning tool built around a React UI and a C++ solver core compiled to WebAssembly.

## Features

- Adaptive solver pipeline with per-stage F2L, OLL, and PLL method toggles
- Interactive 2D cube net with sticker painting and scramble application
- Move Tree Explorer with depth cap, pruning, and optional graph-style merge indicators
- 3D move playback through React Three Fiber
- OLL/PLL learning library with local progress tracking
- Solve history stored in `localStorage`
- Dark and light themes

## Architecture

```text
rubiks-cube-solver/
|-- cpp/                    C++17 solver core compiled with Emscripten
|   |-- src/core/           Cube state, move application, validation, facelet conversion
|   |-- src/solver/         Cross, F2L, OLL, PLL strategies and dispatcher
|   |-- src/explorer/       Lazy move tree/state graph explorer
|   `-- src/bindings/       Emscripten bindings
|-- src/                    React + TypeScript frontend
|   |-- components/         Cube net, move player, reusable UI
|   |-- pages/              Solver, explorer, library, history
|   |-- store/              Zustand app state
|   |-- wasm/               WASM loader and development fallback
|   |-- lib/                Shared frontend utilities
|   `-- storage/            localStorage persistence
|-- public/wasm/            Generated WASM artifacts
`-- .github/workflows/      GitHub Pages build and deploy workflow
```

The intended design follows a strategy pipeline:

```text
React controller
  -> WASM bridge
  -> MethodDispatcher(config)
     -> Cross stage
     -> F2L strategy: advanced or beginner
     -> OLL strategy: advanced or beginner
     -> PLL strategy: advanced or beginner
  -> React solution viewer and animator
```

The frontend includes a TypeScript facelet move engine for immediate UI feedback, scramble application, and local explorer behavior while the C++ engine remains the source of truth for solving.

## Development

### Prerequisites

- Node.js 20+
- Emscripten SDK for building the WASM solver
- CMake

### Setup

```bash
npm install
npm run wasm:build
npm run dev
```

### Checks

```bash
npm run lint
npm run test
npm run build
```

## Deployment

GitHub Actions builds the C++ solver to WebAssembly, builds the Vite frontend, and deploys the final `dist` folder to GitHub Pages.

Update `homepage` in `package.json` before publishing from your own repository.

## Current Implementation Notes

- `public/wasm` is generated output and is not committed by default.
- If WASM artifacts are missing during development, the app uses a mock module so the UI can still be explored.
- The C++ pipeline is scaffolded with strategy classes; complete production-grade OLL/PLL lookup tables can be added behind the existing strategy interfaces.

## License

MIT License.
