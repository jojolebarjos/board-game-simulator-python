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
    GIT_TAG 8b872d3d27b85a7f136afe56b9f569092bd7f1fd
    GIT_SHALLOW 1
)
FetchContent_MakeAvailable(game)

find_package(Python COMPONENTS Interpreter Development.Module NumPy REQUIRED)

python_add_library(_core MODULE src/module.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE game Python::Module Python::NumPy)

install(TARGETS _core DESTINATION ${SKBUILD_PROJECT_NAME})
