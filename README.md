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

The following snippets provide a summary of the interface:

```py
import random

from game import *


# All games have the same API, choosing Connect4 here
State = ConnectState

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


## References

 * https://github.com/python/cpython
 * https://docs.python.org/3/c-api/index.html
 * https://cmake.org/cmake/help/latest/module/FindPython.html
 * https://scikit-build-core.readthedocs.io/en/latest/index.html
 * https://github.com/pybind/scikit_build_example
