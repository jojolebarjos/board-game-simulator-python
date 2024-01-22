cmake_minimum_required(VERSION 3.15...3.27)

project(
    ${SKBUILD_PROJECT_NAME}-py
    VERSION ${SKBUILD_PROJECT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
    game
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../board-game-simulator/"
)
FetchContent_MakeAvailable(game)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

python_add_library(_core MODULE src/module.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE game Python::Module)

install(TARGETS _core DESTINATION ${SKBUILD_PROJECT_NAME})
