# Board game simulator (Python bindings)

This Python module offers a framework for simulating a variety of board games, including Connect4 and Chinese Checkers.
It is based on [this sister project](https://github.com/jojolebarjos/board-game-simulator), providing an efficient implementation tailored for reinforcement learning applications.

The library introduces a unified representation of games via states and actions.
In each state, the current player chooses an action from a set of legal moves, resulting in a transition to a new state.
Upon reaching a terminal state, a reward is assigned based on the outcome of the playthrough.


## Getting started

The library can be installed directly from GitHub:

```
pip install git+https://github.com/jojolebarjos/board-game-simulator-python.git
```

Alternatively, the repository can be downloaded locally:

```
git clone https://github.com/jojolebarjos/board-game-simulator-python.git
cd board-game-simulator-python
pip install .
```

This module relies on [`scikit-build-core`](https://github.com/scikit-build/scikit-build-core) to use CMake as a C++ build tool.
While it should work as-is in most cases, additional configuration may be provided to CMake, as documented [here](https://scikit-build-core.readthedocs.io/en/latest/configuration.html#configuring-cmake-arguments-and-defines).
For instance, a specific compiler may be specified:

```
export SKBUILD_CMAKE_DEFINE="CMAKE_CXX_COMPILER=/opt/homebrew/Cellar/llvm/17.0.6/bin/clang++"
pip install git+https://github.com/jojolebarjos/board-game-simulator-python.git
```

The following snippets provide a summary of the interface:

```py
import random

from game import *


# All games have the same API, choosing Connect4 here
State = Connect4State

# A class method is provided to create a new game state
# Note: some games do feature randomness, hence the `sample` name
state = State.sample_initial_state()

# Usual run until a terminal state is reached
while not state.has_ended:

    # The current player has to take an action
    player = state.player

    # For debugging and serialization, a game-dependent JSON object is provided
    json_like = state.to_json()

    # For modelling features, a game-dependent tuple of NumPy arrays is provided
    arrays = state.get_tensors()

    # At least one action is available
    actions = state.actions
    action = random.choice(actions)

    # States and actions are immutable, the next is a new object
    # Note: similar to the initial state, the next state may be non-deterministic
    state = action.sample_next_state()

# When the game has ended, the reward is provided as a NumPy array
reward = state.reward
```

Additionally, some example [`scripts`](./scripts/) and [`tests`](./tests/) are provided:

```
python ./scripts/benchmark.py
pytest -v
```

Available games:

 * "Bounce", a homebrew game I heard of. I was unable to find anything online, including the actual name...
 * [Connect 4](https://en.wikipedia.org/wiki/Connect_Four)
 * [Chinese checkers](https://en.wikipedia.org/wiki/Chinese_checkers), 2 players only


## References

 * https://github.com/python/cpython
 * https://docs.python.org/3/c-api/index.html
 * https://cmake.org/cmake/help/latest/module/FindPython.html
 * https://scikit-build-core.readthedocs.io/en/latest/index.html
 * https://github.com/pybind/scikit_build_example
