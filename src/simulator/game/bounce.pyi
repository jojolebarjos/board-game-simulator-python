from typing import Any

import numpy as np

class Config:
    num_players: int
    grid: np.ndarray

    def __init__(self, grid: np.ndarray) -> None: ...
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
    def actions_at(self, source: np.ndarray) -> list[Action]: ...
    def action_at(self, source: np.ndarray, target: np.ndarray) -> Action: ...
    def to_json(self) -> dict[str, Any]: ...
    @staticmethod
    def from_json(value: dict[str, Any]) -> State: ...

class Action:
    state: State
    source: np.ndarray
    target: np.ndarray

    # TODO should have __init__?
    def sample_next_state(self) -> State: ...
    def to_json(self) -> dict[str, Any]: ...
    @staticmethod
    def from_json(value: dict[str, Any]) -> Action: ...
