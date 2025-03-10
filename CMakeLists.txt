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
    GIT_TAG 804ceeb8dfbe2bbb95bac7ae3a2670b6a9db3888
)
FetchContent_MakeAvailable(game-cpp)

find_package(Python 3.8
    REQUIRED COMPONENTS Interpreter Development.Module NumPy
    OPTIONAL_COMPONENTS Development.SABIModule
)

find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(bounce STABLE_ABI
    src/simulator/game/bounce.cpp
)
target_link_libraries(bounce PRIVATE game-cpp)

nanobind_add_module(connect STABLE_ABI
    src/simulator/game/connect.cpp
)
target_link_libraries(connect PRIVATE game-cpp)

install(TARGETS bounce connect LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME}/game)
