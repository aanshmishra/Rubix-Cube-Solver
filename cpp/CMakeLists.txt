cmake_minimum_required(VERSION 3.16)
project(rubiks_solver)

set(CMAKE_CXX_STANDARD 17)

# Emscripten-specific settings
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    
    # embind + optimization flags
    set(EMSCRIPTEN_FLAGS
        "-sEXPORT_ES6=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createRubiksModule"
        "-sENVIRONMENT=web"
        "-sALLOW_MEMORY_GROWTH=0"
        "-sWASM=1"
        "-O3"
        "-sINITIAL_MEMORY=128MB"
        "--bind"
        "-sNO_EXIT_RUNTIME=1"
        "-sASSERTIONS=0"
        "-sTEXTDECODER=1"
        "-flto"
    )
    
    string(REPLACE ";" " " EMSCRIPTEN_FLAGS_STR "${EMSCRIPTEN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS_STR}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS_STR}")
endif()

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(rubiks_solver ${SOURCES})

# Output to specific directory for CI
set_target_properties(rubiks_solver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../public/wasm"
)
