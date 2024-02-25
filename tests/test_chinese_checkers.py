import regex as re

import numpy as np

from game import ChineseCheckersState as State


def parse(string):
    # Extract rows
    lines = []
    min_indent = len(string)
    for line in string.split("\n"):
        line = line.rstrip("\r ")
        if line:
            match = re.search(r"[\.OX\*]", line)
            assert match
            indent = match.start()
            min_indent = min(min_indent, indent)
            lines.append(line + " ")

    # Parse rows
    assert len(lines) == 17
    PATTERN = r"( *\[?)([\.OX\*][ \]\[])+"
    pieces = [[], []]
    selections = []
    targets = []
    for y, line in enumerate(lines):
        match = re.fullmatch(PATTERN, line)
        assert match
        x0 = len(match.group(1)) - min_indent
        steps = match.captures(2)
        for i, step in enumerate(steps):
            x = x0 + i * 2

            # Convert to grid coordinates
            assert (x + y) % 2 == 0
            x_grid = 6 + (x - y) // 2
            y_grid = 18 - (x + y) // 2
            coordinate = [x_grid, y_grid]

            if step[0] == "O":
                pieces[0].append(coordinate)
            if step[0] == "X":
                pieces[1].append(coordinate)
            if step[0] == "*":
                targets.append(coordinate)
            if step[1] == "]":
                selections.append(coordinate)

    # Sort pieces, so that it is consistent with internal representation
    pieces[0].sort()
    pieces[1].sort()
    selections.sort()
    targets.sort

    return pieces, selections, targets


def encode(state_or_action):
    if isinstance(state_or_action, State):
        state = state_or_action
        action = None
    elif isinstance(state_or_action, Action):
        action = state_or_action
        state = action.state
    else:
        raise TypeError

    # TODO render state as string, for easier debugging
    raise NotImplementedError


def assert_state(state, string):
    state_dict = state.to_json()
    pieces, selections, targets = parse(string)

    # First, check that all pieces are at the expected location
    assert state_dict["pieces"] == pieces

    # Then, if some pieces are highlighted, check actions
    selected_action = None
    if len(selections) > 0:
        piece_selection = None
        target_selection = None
        for selection in selections:
            if selection in targets:
                assert not target_selection
                target_selection = selection
            else:
                assert not piece_selection
                piece_selection = selection

        # Get current player
        player = 0 if piece_selection in pieces[0] else 1
        assert player == state_dict["player"]

        # Search for related actions
        destinations = []
        for action in state.actions:
            action_dict = action.to_json()
            index = action_dict["index"]
            source = state_dict["pieces"][player][index]
            destination = [action_dict["x"], action_dict["y"]]
            if piece_selection == source:
                destinations.append(destination)
                if target_selection == destination:
                    assert not selected_action
                    selected_action = action

        # This should be an exact match
        targets.sort()
        destinations.sort()
        assert targets == destinations

        if target_selection:
            assert selected_action

    return selected_action


def assert_string(string):
    pieces, selections, targets = parse(string)
    [selection] = selections

    assert len(pieces[0]) == 10
    assert len(pieces[1]) == 10

    player = 0 if selection in pieces[0] else 1

    state_dict = {
        "pieces": pieces,
        "player": player,
        "winner": -1,
    }
    state = State.from_json(state_dict)

    return assert_state(state, string)


def test_individual():
    assert_string(
        """
                 X
                X X
               X X X
              X . . .
     . . . . . . . . . . . . .
      . . . . . . . . . . . .
       . . . . . . . . . . .
        . * . . . . . . . .
         . X . O . . . . .
        . . * . . . . . . .
       . . . X . * * . . . .
      . . . . * X[O]* . . . .
     . . . . . . O * . . . . .
              O * . O
               O . O
                O O
                 O
    """
    )


def test_sequence():
    state = State.sample_initial_state()

    # Turn 1
    string = """
                 X
                X X
               X X X
              X X X X
     . . . . . . . . . . . . .
      . . . . . . . . . . . .
       . . . . . . . . . . .
        . . . . . . . . . .
         . . . . . . . . .
        . . . . . . . . . .
       . . . . . . . . . . .
      . . . . . . . . . . . .
     . . . . . *[*]. . . . . .
              O[O]O O
               O O O
                O O
                 O
    """
    action = assert_state(state, string)
    state = action.sample_next_state()

    # Turn 2
    string = """
                 X
                X X
              [X]X X
              X X X X
     . . . . * .[*]. . . . . .
      . . . . . . . . . . . .
       . . . . . . . . . . .
        . . . . . . . . . .
         . . . . . . . . .
        . . . . . . . . . .
       . . . . . . . . . . .
      . . . . . . . . . . . .
     . . . . . . O . . . . . .
              O . O O
               O O O
                O O
                 O
    """
    action = assert_state(state, string)
    state = action.sample_next_state()

    # TODO more turns
