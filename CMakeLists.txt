cmake_minimum_required(VERSION 3.23)

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
    GIT_REPOSITORY https://github.com/jojolebarjos/board-game-simulator.git
    GIT_TAG 7ca0f1867fff695a2a666526f577cd1dd9fd20f4
    GIT_SHALLOW 1
)
FetchContent_MakeAvailable(game)

find_package(Python COMPONENTS Interpreter Development.Module NumPy REQUIRED)

python_add_library(_core MODULE src/module.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE game Python::Module Python::NumPy)

install(TARGETS _core DESTINATION ${SKBUILD_PROJECT_NAME})
