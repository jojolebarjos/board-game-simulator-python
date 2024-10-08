cmake_minimum_required(VERSION 3.18...3.27)

project(
    ${SKBUILD_PROJECT_NAME}-py
    VERSION ${SKBUILD_PROJECT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
    game-cpp
    GIT_REPOSITORY https://github.com/jojolebarjos/board-game-simulator.git
    GIT_TAG e9b2074b3e9a7e7159af8b57875bf17865685281
    GIT_SHALLOW 1
)
FetchContent_MakeAvailable(game-cpp)

find_package(Python 3.8
    REQUIRED COMPONENTS Interpreter Development.Module NumPy
    OPTIONAL_COMPONENTS Development.SABIModule
)

find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(game STABLE_ABI
    src/simulator/game/module.cpp
    src/simulator/game/bounce.cpp
    src/simulator/game/connect.cpp
)
target_link_libraries(game PRIVATE game-cpp)

install(TARGETS game LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME})
