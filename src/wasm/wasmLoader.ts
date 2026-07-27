/// <reference types="vite/client" />
// Loader for the Emscripten-built C++ solver (cpp/, built by cpp/build.ps1).
//
// The module is optional by design: if public/wasm/rubiks_solver.js is missing
// - a checkout that has never run the build, or a static deploy that did not
// ship the asset - loading resolves to null and the caller falls back to the
// TypeScript solver in src/lib/solver/. There is deliberately no mock module:
// a stub that returns plausible-looking nonsense is far worse than a null the
// caller must handle.

export interface SolveOutput {
  crossMoves: string;   // space-separated notation, possibly empty
  f2lMoves: string;
  ollMoves: string;
  pllMoves: string;
  methodName: string;
  success: boolean;
  errorMessage: string;
}

export interface WasmSolver {
  // `target` is a solved cube in the user's physical orientation; it is what
  // tells the solver the colour scheme.
  solveFacelets(
    scrambled: string,
    target: string,
    f2lAdvanced: boolean,
    ollAdvanced: boolean,
    pllAdvanced: boolean
  ): SolveOutput;
}

const WASM_MODULE_PATH = `${import.meta.env.BASE_URL}wasm/rubiks_solver.js`;

let solver: WasmSolver | null = null;
let attempted: Promise<WasmSolver | null> | null = null;

export async function loadWasmSolver(): Promise<WasmSolver | null> {
  if (solver) return solver;
  if (attempted) return attempted;

  attempted = (async () => {
    try {
      // Fetch the glue as text and import it from a Blob URL rather than
      // importing the path directly. Importing a file under public/ makes
      // Vite's dev server rewrite the request to `...js?import`, which it then
      // answers with a 500 - the module is not in its graph. A Blob URL is
      // invisible to Vite in dev and to the bundler in a build, so one code
      // path works in both.
      //
      // A missing asset also comes back as the SPA's index.html rather than a
      // 404, hence the content-type guard.
      const response = await fetch(WASM_MODULE_PATH);
      if (!response.ok) return null;
      if ((response.headers.get('content-type') ?? '').includes('text/html')) return null;

      const source = await response.text();
      const blobUrl = URL.createObjectURL(new Blob([source], { type: 'text/javascript' }));

      try {
        // Built with -s EXPORT_ES6=1 -s MODULARIZE=1: the default export is a
        // factory that resolves to the instantiated module.
        const factory = (await import(/* @vite-ignore */ blobUrl)).default;

        // Loading from a Blob leaves the runtime unable to work out where it
        // came from, so it cannot find the sibling .wasm on its own. Point it
        // back at the real directory explicitly.
        solver = (await factory({
          locateFile: (path: string) =>
            new URL(`${import.meta.env.BASE_URL}wasm/${path}`, window.location.href).href,
        })) as WasmSolver;
        return solver;
      } finally {
        URL.revokeObjectURL(blobUrl);
      }
    } catch (err) {
      console.warn('WASM solver unavailable, falling back to the TS solver:', err);
      return null;
    }
  })();

  return attempted;
}

// Splits the C++ side's space-separated notation into the array shape the UI
// and the history store already expect.
export function toMoveArray(notation: string): string[] {
  return notation.split(/\s+/).filter(Boolean);
}
