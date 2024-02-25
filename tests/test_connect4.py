import re

import numpy as np

from game import Connect4State as State


def to_grid(literal):
    literal = re.sub(r"\s+", "", literal)
    assert len(literal) == 6 * 7
    mapping = {".": -1, "O": 0, "X": 1}
    array = np.array([mapping[c] for c in literal]).reshape([6, 7])
    grid = array[::-1].tolist()
    return grid


def test_simple():
    state = State.sample_initial_state()

    state = state.actions[3].sample_next_state()
    state = state.actions[3].sample_next_state()
    state = state.actions[2].sample_next_state()
    state = state.actions[4].sample_next_state()
    state = state.actions[3].sample_next_state()
    state = state.actions[4].sample_next_state()
    state = state.actions[1].sample_next_state()
    state = state.actions[0].sample_next_state()
    state = state.actions[2].sample_next_state()
    state = state.actions[3].sample_next_state()
    state = state.actions[3].sample_next_state()
    state = state.actions[3].sample_next_state()

    expected = """
        . . . X . . .
        . . . O . . .
        . . . X . . .
        . . . O . . .
        . . O X X . .
        X O O O X . .
    """
    assert state.to_json()["grid"] == to_grid(expected)

    assert len(state.actions) == 6

    state = state.actions[2].sample_next_state()
    state = state.actions[3].sample_next_state()
    state = state.actions[2].sample_next_state()

    expected = """
        . . . X . . .
        . . . O . . .
        . . O X . . .
        . . O O X . .
        . . O X X . .
        X O O O X . .
    """
    assert state.to_json()["grid"] == to_grid(expected)

    assert state.has_ended
    assert len(state.actions) == 0
    assert state.winner == 0
    assert state.reward.tolist() == [1, -1]
