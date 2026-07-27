cmake_minimum_required(VERSION 3.16)
project(rubiks_solver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This file exists for CI (.github/workflows/deploy.yml runs emcmake cmake).
# For local builds cpp/build.ps1 does the same thing without needing CMake at
# all - keep the two in sync if you change flags.

# Explicit source list rather than a glob: src/explorer/ is a move-tree
# visualiser that nothing in the current bindings exposes, and globbing would
# silently pull it into the module.
set(SOURCES
    src/core/cube_state.cpp
    src/core/facelet_converter.cpp
    src/core/validation.cpp
    src/solver/cross.cpp
    src/solver/method_dispatcher.cpp
    src/solver/solve_api.cpp
    src/solver/f2l/f2l_advanced.cpp
    src/solver/f2l/f2l_beginner.cpp
    src/solver/oll/oll_advanced.cpp
    src/solver/oll/oll_beginner.cpp
    src/solver/pll/pll_advanced.cpp
    src/solver/pll/pll_beginner.cpp
    src/bindings/emscripten_bindings.cpp
)

add_executable(rubiks_solver ${SOURCES})
target_include_directories(rubiks_solver PRIVATE src)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    # Kept deliberately identical to cpp/build.ps1:
    #   ENVIRONMENT=web,worker - adding `node` emits imports of Node built-ins
    #     that a browser cannot resolve.
    #   GROWABLE_ARRAYBUFFERS=0 - otherwise the growable heap is a *resizable*
    #     ArrayBuffer and Chrome's TextDecoder refuses to decode from one, so
    #     every string returned over embind throws.
    #   ALLOW_MEMORY_GROWTH=1 - a deep F2L frontier outgrows the initial heap.
    target_link_options(rubiks_solver PRIVATE
        -lembind
        -O3
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sEXPORT_NAME=createRubiksSolver"
        "-sENVIRONMENT=web,worker"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sGROWABLE_ARRAYBUFFERS=0"
        "-sINITIAL_MEMORY=33554432"
        "-sMAXIMUM_MEMORY=1073741824"
        "-sNO_EXIT_RUNTIME=1"
        "-sASSERTIONS=0"
    )
    target_compile_options(rubiks_solver PRIVATE -O3)

    set_target_properties(rubiks_solver PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../public/wasm"
    )
endif()
