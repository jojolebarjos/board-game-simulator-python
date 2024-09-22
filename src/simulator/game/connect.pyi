from typing import Any

import numpy as np

class Config:
    num_players: int
    height: int
    width: int
    count: int

    def __init__(self, height: int, width: int, count: int) -> None: ...
    def sample_initial_state(self) -> State: ...
    def to_json(self) -> dict[str, Any]: ...
    @staticmethod
    def from_json(value: dict[str, Any]) -> Config: ...

class State:
    config: Config
    has_ended: bool
    player: int
    reward: np.ndarray
    grid: np.ndarray
    actions: list[Action]

    # TODO should have __init__?
    def action_at(self, column: int) -> Action: ...
    def to_json(self) -> dict[str, Any]: ...
    @staticmethod
    def from_json(value: dict[str, Any]) -> State: ...

class Action:
    state: State
    column: int

    # TODO should have __init__?
    def sample_next_state(self) -> State: ...
    def to_json(self) -> dict[str, Any]: ...
    @staticmethod
    def from_json(value: dict[str, Any]) -> Action: ...
