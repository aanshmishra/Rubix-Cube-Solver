# 🧩 Rubik's Cube Solver

A browser-based Rubik's Cube solver and learning tool, pairing a React UI with a C++17 solver core compiled to WebAssembly for fast, deterministic solves.

<p align="left">
  <img alt="React" src="https://img.shields.io/badge/React-18.3-61DAFB?logo=react&logoColor=white&labelColor=20232a">
  <img alt="TypeScript" src="https://img.shields.io/badge/TypeScript-5.6-3178C6?logo=typescript&logoColor=white">
  <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white">
  <img alt="WebAssembly" src="https://img.shields.io/badge/WebAssembly-Emscripten-654FF0?logo=webassembly&logoColor=white">
  <img alt="Vite" src="https://img.shields.io/badge/Vite-6-646CFF?logo=vite&logoColor=white">
  <img alt="License" src="https://img.shields.io/badge/License-MIT-green.svg">
</p>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Tech Stack](#tech-stack)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Setup](#setup)
  - [Available Scripts](#available-scripts)
- [Deployment](#deployment)
- [Implementation Notes](#implementation-notes)
- [License](#license)

---

## Overview

This project lets you scramble, solve, and study the Rubik's Cube directly in the browser. A TypeScript facelet engine drives instant UI feedback, while a compiled C++ solver — running as WebAssembly — provides the authoritative solve pipeline, working stage by stage through Cross, F2L, OLL, and PLL with switchable beginner/advanced strategies per stage.

Beyond solving, it doubles as a study tool: an interactive move-tree explorer, a 3D animated playback of the solution, and an OLL/PLL algorithm library with locally tracked progress.

## Features

| Category | Capability |
|---|---|
| 🧠 Solver | Adaptive pipeline with per-stage F2L, OLL, and PLL method toggles (beginner ↔ advanced) |
| 🎨 Cube Net | Interactive 2D net with sticker painting and scramble application |
| 🌳 Explorer | Move Tree Explorer with depth cap, pruning, and optional graph-style merge indicators |
| 🎥 3D Playback | Animated move-by-move solution playback via React Three Fiber |
| 📚 Learning Library | OLL/PLL algorithm library with local progress tracking |
| 🕘 History | Solve history persisted in `localStorage` |
| 🌗 Theming | Dark and light themes |

## Architecture

The solving pipeline flows from the React controller, through a WASM bridge, into a configurable method dispatcher, and back into the React solution viewer:

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

The frontend also ships a lightweight TypeScript facelet move engine used for immediate UI feedback, scramble application, and local explorer behavior — while the compiled C++ engine remains the single source of truth for actual solving.

## Tech Stack

**Frontend**
- React 18 + TypeScript, built with Vite
- Zustand for app state
- Tailwind CSS + shadcn/ui components, Radix UI primitives
- React Three Fiber / drei for 3D rendering
- Framer Motion for animation

**Solver Core**
- C++17 compiled to WebAssembly via Emscripten
- CMake build orchestration

**Tooling**
- ESLint, Vitest + Testing Library
- GitHub Actions → GitHub Pages deployment

## Project Structure

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

## Getting Started

### Prerequisites

- [Node.js](https://nodejs.org/) 20+
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) for building the WASM solver
- [CMake](https://cmake.org/)

### Setup

```bash
npm install
npm run wasm:build
npm run dev
```

The app will be available locally through Vite's dev server (default `http://localhost:5173`).

> If WASM artifacts are missing, the app automatically falls back to a mock solver module so the UI remains explorable without a native build step.

### Available Scripts

| Script | Description |
|---|---|
| `npm run dev` | Start the Vite development server |
| `npm run build` | Type-check and build the production bundle |
| `npm run preview` | Preview the production build locally |
| `npm run lint` | Run ESLint over the project |
| `npm run test` | Run the Vitest test suite |
| `npm run wasm:build` | Build the C++ solver to WASM (PowerShell script) |
| `npm run wasm:build:cmake` | Build the C++ solver to WASM directly via CMake |
| `npm run deploy` | Build and publish `dist` to GitHub Pages |

## Deployment

GitHub Actions builds the C++ solver to WebAssembly, builds the Vite frontend, and deploys the resulting `dist` folder to GitHub Pages.

Before publishing from your own fork, update `homepage` in [package.json](package.json) to match your repository.

## Implementation Notes

- `public/wasm` is generated output and is not committed by default — run `npm run wasm:build` to produce it locally.
- If WASM artifacts are missing during development, the app transparently uses a mock module so the UI can still be explored.
- The C++ pipeline is scaffolded with strategy classes; production-grade OLL/PLL lookup tables can be added behind the existing strategy interfaces.

## License

Released under the [MIT License](LICENSE).
