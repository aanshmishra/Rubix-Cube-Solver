# Builds the solver to WebAssembly, straight from em++ - no CMake needed.
#
#   pwsh cpp/build.ps1
#
# Requires emsdk; the script activates it if emcc is not already on PATH.
# For correctness testing use the native harness instead - it exercises the
# same C++ through solve_api and needs nothing but g++:
#   g++ -std=c++17 -O2 -Icpp/src -o cpp/solver_tests.exe <sources> cpp/tests/*.cpp

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    $emsdk = Join-Path $env:USERPROFILE 'emsdk\emsdk_env.ps1'
    if (-not (Test-Path $emsdk)) {
        throw "emcc not on PATH and no emsdk at $emsdk - install from https://emscripten.org"
    }
    $env:EMSDK_QUIET = 1
    & $emsdk | Out-Null
}

$sources = @(
    'cpp/src/core/cube_state.cpp'
    'cpp/src/core/facelet_converter.cpp'
    'cpp/src/core/validation.cpp'
    'cpp/src/solver/cross.cpp'
    'cpp/src/solver/method_dispatcher.cpp'
    'cpp/src/solver/solve_api.cpp'
    'cpp/src/solver/f2l/f2l_advanced.cpp'
    'cpp/src/solver/f2l/f2l_beginner.cpp'
    'cpp/src/solver/oll/oll_advanced.cpp'
    'cpp/src/solver/oll/oll_beginner.cpp'
    'cpp/src/solver/pll/pll_advanced.cpp'
    'cpp/src/solver/pll/pll_beginner.cpp'
    'cpp/src/bindings/emscripten_bindings.cpp'
)

New-Item -ItemType Directory -Force 'public/wasm' | Out-Null

# MODULARIZE + ES6 so the output is an ES module the app can import, and
# ALLOW_MEMORY_GROWTH because a deep F2L frontier can outgrow the initial heap.
#
# ENVIRONMENT stays web,worker on purpose. Adding `node` makes Emscripten emit
# imports of Node built-ins, which a browser cannot resolve - the module then
# fails with "Failed to fetch dynamically imported module" and the app silently
# drops to the TypeScript fallback.
$common = @(
    '-std=c++17'
    '-O3'
    '-lembind'
    '-s', 'MODULARIZE=1'
    '-s', 'EXPORT_ES6=1'
    '-s', 'ALLOW_MEMORY_GROWTH=1'
    # Emscripten otherwise backs the growable heap with a *resizable*
    # ArrayBuffer, and Chrome's TextDecoder refuses to decode from one - every
    # string coming back over embind then throws
    # "The provided ArrayBuffer value must not be resizable".
    '-s', 'GROWABLE_ARRAYBUFFERS=0'
    '-s', 'INITIAL_MEMORY=33554432'
    '-s', 'MAXIMUM_MEMORY=1073741824'
    '-s', 'EXPORT_NAME=createRubiksSolver'
    '-Icpp/src'
)

Write-Host 'Building WASM for the browser...' -ForegroundColor Cyan
& em++ @common '-s' 'ENVIRONMENT=web,worker' '-o' 'public/wasm/rubiks_solver.js' @sources
if ($LASTEXITCODE -ne 0) { throw "em++ failed with exit code $LASTEXITCODE" }
Get-ChildItem 'public/wasm' -Filter 'rubiks_solver.*' | Select-Object Name, Length | Format-Table -AutoSize

Write-Host 'Done.' -ForegroundColor Green
