import regex as re

import numpy as np

from simulator.game.connect import Config, State, Action


def parse(string):
    lines = [line for line in string.split("\n") if line.strip()]

    # Infer indentation
    indent = len(string)
    for line in lines:
        indent = min(indent, len(re.match(r"^ *", line).group()))

    # Check for column selector
    selected_column = None
    if "v" in lines[0]:
        selected_column = (
            len(re.fullmatch(r"( *)v\s*", lines[0]).group(1)) - indent
        ) // 2
        del lines[0]

    # Parse grid
    rows = []
    mapping = {".": -1, "O": 0, "X": 1}
    for line in lines[::-1]:
        symbols = re.sub(r"\s", "", line)
        row = [mapping[symbol] for symbol in symbols]
        rows.append(row)
        assert len(row) == len(rows[0])
    grid = np.array(rows)

    # Detect which player is supposed to play
    num_o = (grid == 0).sum()
    num_x = (grid == 1).sum()
    if num_o == num_x:
        next_player = 0
    elif num_o == num_x + 1:
        next_player = 1
    else:
        assert False

    return grid, selected_column, next_player


def assert_state(string, state=None, config=None):
    grid, selected_column, next_player = parse(string)

    # If state is provided, this should match
    if state is not None:
        np.testing.assert_array_equal(grid, state.grid)
        if state.has_ended:
            assert selected_column is None
        else:
            assert next_player == state.player

    # Otherwise, use the board as-is
    else:
        raise NotImplementedError

    # Fetch action object
    selected_action = None
    if selected_column is not None:
        selected_action = state.action_at(selected_column)

    return state, selected_action


def test_small():
    """Test small 2x3 board."""

    config = Config(2, 3, 2)
    state = config.sample_initial_state()

    # Turn 1
    _, action = assert_state(
        """
           v
         . . .
         . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 2
    _, action = assert_state(
        """
           v
         . . .
         . O .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 3
    _, action = assert_state(
        """
             v
         . X .
         . O .
        """,
        state,
    )
    state = action.sample_next_state()

    # End
    _, _ = assert_state(
        """
         . X .
         . O O
        """,
        state,
    )
    np.testing.assert_array_equal(state.reward, [1, -1])
