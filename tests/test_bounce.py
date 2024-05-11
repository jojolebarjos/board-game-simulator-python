import regex as re

import numpy as np

from game import BounceState as State


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
    assert len(rows) == 9
    assert len(rows[0]) == 1
    assert len(rows[-1]) == 1
    assert all(len(row) == 6 for row in rows[1:-1])

    # Parse rows
    grid = np.zeros((7, 6), dtype=np.int8)
    selections = []
    targets = []
    for y, steps in enumerate(rows[::-1], -1):
        for x, (piece, marker) in enumerate(steps):
            if piece.isdigit():
                assert 0 <= y < 7
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
        state_dict = {
            "grid": grid.tolist(),
            "player": player,
            "winner": -1,
        }
        state = State.from_json(state_dict)

    # Recover game state
    state_dict = state.to_json()

    # First, check that all pieces are at the expected location
    assert state_dict["grid"] == grid.tolist()

    # Check current player
    assert state_dict["player"] == player

    # Then, if a piece is selected
    selected_action = None
    if len(selections) > 0:
        piece_selection = None
        target_selection = None
        for selection in selections:
            if selection in targets:
                assert not target_selection
                target_selection = selection
            else:
                assert grid[selection[1], selection[0]] > 0
                assert not piece_selection
                piece_selection = selection

        # A piece single must be selected
        assert piece_selection is not None

        # Get associated actions
        actions = {}
        for action in state.actions:
            action_dict = action.to_json()
            from_coord = action_dict["from"]["x"], action_dict["from"]["y"]
            to_coord = action_dict["to"]["x"], action_dict["to"]["y"]
            if from_coord == piece_selection:
                actions[to_coord] = action

        # Actions must be exhaustively highlighted
        assert set(targets) == set(actions.keys())

        # If an action is selected, return that
        if target_selection is not None:
            selected_action = actions[target_selection]

    return state, selected_action


def test_sequence():
    state = State.sample_initial_state()

    # Turn 1
    _, action = assert_state(
        """
              .
         1 2 3 3 2 1
         . . . . . .
         . . . . . .
         . . . . . .
         .[*]. . . .
         * . * . . .
        [1]2 3 3 2 1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 2
    _, action = assert_state(
        """
              .
         1 2[3]3 2 1
         * . . . * .
         . * . * . .
         . .[*]. . .
         . 1 . . . .
         . . . . . .
         . 2 3 3 2 1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 3
    _, action = assert_state(
        """
              .
         1 2 . 3 2 1
         . . . . . .
         . . . . . .
         .[*]3 . . .
         * 1 * . . .
         * . * . . .
         .[2]3 3 2 1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 4
    _, action = assert_state(
        """
              .
        [1]2 . 3 2 1
         * * * * . *
         . * * . * .
         . 2 3[*]. .
         . 1 . . . .
         . . . . . .
         . . 3 3 2 1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 5
    _, action = assert_state(
        """
              .
         . 2 * 3 2 1
         . * . * . .
        [*]. * . * .
         . 2 3 1 . .
         * 1 * * . .
         * . * . * .
         . .[3]3 2 1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 6
    _, action = assert_state(
        """
              .
         . 2 .[3]2 1
         . * . * . *
         3 . * . * .
         . 2 3 1 * .
         . 1 . * * .
         . * .[*]. .
         . . * 3 2 1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    # Turn 7
    _, action = assert_state(
        """
             [*]
         . 2 * * 2 1
         * * * * . .
         3 . * * * .
         . 2 3 1 * .
         * 1 * * * *
         * . . 3 . *
         . . . 3[2]1
              .
        """,
        state,
    )
    state = action.sample_next_state()

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.winner == 0
    assert state.reward.tolist() == [1, -1]
