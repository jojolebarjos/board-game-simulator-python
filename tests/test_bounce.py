import regex as re

import numpy as np

from simulator.game.bounce import Config, State, Action


def parse(string):
    # Extract rows
    PATTERN = r"\[?([\.\d\*][ \]\[])+\s*"
    rows = []
    for line in string.split("\n"):
        line = line.strip()
        if line:
            match = re.fullmatch(PATTERN, line + " ")
            assert match
            steps = match.captures(1)
            rows.append(steps)
    height = len(rows)
    width = len(rows[0])
    assert all(len(row) == width for row in rows)

    # Parse rows
    grid = np.zeros((height, width), dtype=np.int8)
    selections = []
    targets = []
    for y, steps in enumerate(rows[::-1]):
        for x, (piece, marker) in enumerate(steps):
            if piece.isdigit():
                assert 0 < y < height - 1
                grid[y, x] = int(piece)
            if piece == "*":
                targets.append((x, y))
            if marker == "]":
                selections.append((x, y))

    return grid, selections, targets


def assert_state(string, state=None):
    grid, selections, targets = parse(string)

    # Which player is meant to play (if unknown, assume 0)
    player = 0
    if selections:
        [(_, y)] = set(selections) - set(targets)
        if grid[:y].sum() > 0:
            player = 1

    # If state is not provided, use string as-is
    if state is None:
        assert player == 0
        config = Config(grid)
        state = config.sample_initial_state()

    # First, check that all pieces are at the expected location
    np.testing.assert_array_equal(grid, state.grid)

    # Check current player
    assert player == state.player

    # Then, if a piece is selected
    selected_action = None
    if len(selections) > 0:
        piece_selection = None
        target_selection = None
        for selection in selections:
            if selection in targets:
                assert target_selection is None
                target_selection = selection
            else:
                assert grid[selection[1], selection[0]] > 0
                assert piece_selection is None
                piece_selection = selection

        # A piece single must be selected
        assert piece_selection is not None

        # Get associated actions
        actions = {tuple(action.target): action for action in state.actions_at(np.array(piece_selection))}

        # Actions must be exhaustively highlighted
        assert set(targets) == set(actions.keys())

        # If an action is selected, return that
        if target_selection is not None:
            selected_action = actions[target_selection]

    return state, selected_action


def test_small():
    """..."""

    # Turn 1
    _, action = assert_state(
        """
         . . .
         1 2 3
         . * .
        [*]. *
         1[2]3
         . . .
        """,
    )
    state = action.sample_next_state()

    # Turn 2
    _, action = assert_state(
        """
         . . .
         1 2[3]
        [*]. .
         2 * .
         1 . 3
         . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 3
    _, action = assert_state(
        """
         . . .
         1 2 *
         3 * .
         2 .[*]
         1 .[3]
         . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 4
    _, action = assert_state(
        """
         . . .
        [1]2 .
         3 . *
         2 * 3
         1 * *
        [*]* .
        """,
        state,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.reward.tolist() == [-1, 1]


def test_normal():
    """..."""

    # Turn 1
    _, action = assert_state(
        """
         . . . . . .
         1 2 3 3 2 1
         . . . . . .
         . . . . . .
         . . . . . .
         .[*]. . . .
         * . * . . .
        [1]2 3 3 2 1
         . . . . . .
        """,
    )
    state = action.sample_next_state()

    # Turn 2
    _, action = assert_state(
        """
         . . . . . .
         1 2[3]3 2 1
         * . . . * .
         . * . * . .
         . .[*]. . .
         . 1 . . . .
         . . . . . .
         . 2 3 3 2 1
         . . . . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 3
    _, action = assert_state(
        """
         . . . . . .
         1 2 . 3 2 1
         . . . . . .
         . . . . . .
         .[*]3 . . .
         * 1 * . . .
         * . * . . .
         .[2]3 3 2 1
         . . . . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 4
    _, action = assert_state(
        """
         . . . . . .
        [1]2 . 3 2 1
         * * * * . *
         . * * . * .
         . 2 3[*]. .
         . 1 . . . .
         . . . . . .
         . . 3 3 2 1
         . . . . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 5
    _, action = assert_state(
        """
         . . . . . .
         . 2 * 3 2 1
         . * . * . .
        [*]. * . * .
         . 2 3 1 . .
         * 1 * * . .
         * . * . * .
         . .[3]3 2 1
         . . . . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 6
    _, action = assert_state(
        """
         . . . . . .
         . 2 .[3]2 1
         . * . * . *
         3 . * . * .
         . 2 3 1 * .
         . 1 . * * .
         . * .[*]. .
         . . * 3 2 1
         . . . . . .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 7
    _, action = assert_state(
        """
         * .[*]. . .
         . 2 * * 2 1
         * * * * . .
         3 . * * * .
         . 2 3 1 * .
         * 1 * * * *
         * . . 3 . *
         . . . 3[2]1
         . . . . . .
        """,
        state,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.reward.tolist() == [1, -1]


def test_last_row():
    _, action = assert_state(
        """
         . .[*]. . .
         . * 1 3 2 1
         . * . * . .
         * 2 * . * .
         . * 3 . . *
         2 * * * . .
         1 * * . * 1
         . .[3]3 2 .
         . . . . . .
        """,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.reward.tolist() == [1, -1]


def test_larger_values():
    _, action = assert_state(
        """
         . . . .[*].
         1 . . * . *
         . . * . * .
         . * . * . *
         * . * . * .
         . * . * . *
         * . * . * .
         . . . .[7].
         . . . . . .
        """,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.reward.tolist() == [1, -1]


def test_block_victory():
    _, action = assert_state(
        """
         . . . . . .
         . . . . . .
         . . . . . .
         . . . . . .
         3 . 3 . 3 .
         2 2 2 2[*]2
         . . . * . *
         1 . * .[2].
         . . . . . .
        """,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.reward.tolist() == [1, -1]


def test_draw():
    _, action = assert_state(
        """
         . . . . . .
         . . . . . .
         2 2 2 2 2 2
         3 3 3 3 3 3
         3 .[*]. 3 .
         . * . * . .
         * . * . * .
         . .[3]. . *
         . . . . . .
        """,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.reward.tolist() == [0, 0]


def test_json():
    """Test JSON conversion."""

    state, action = assert_state(
        """
         . . .
         1 2 3
         . * .
        [*]. *
         1[2]3
         . . .
        """,
    )
    config = state.config

    assert config.to_json() == {
        "grid": [
            [0, 0, 0],
            [1, 2, 3],
            [0, 0, 0],
            [0, 0, 0],
            [1, 2, 3],
            [0, 0, 0],
        ],
    }
    assert Config.from_json(config.to_json()) == config

    assert state.to_json() == {
        "grid": [
            [0, 0, 0],
            [1, 2, 3],
            [0, 0, 0],
            [0, 0, 0],
            [1, 2, 3],
            [0, 0, 0],
        ],
        "player": 0,
    }
    assert State.from_json(state.to_json(), config) == state

    assert action.to_json() == {
        "source": [1, 1],
        "target": [0, 2],
    }
    assert Action.from_json(action.to_json(), state) == action
