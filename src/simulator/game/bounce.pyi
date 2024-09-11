import numpy as np

class Config:
    num_players: int
    grid: np.ndarray

    def sample_initial_state(self) -> State: ...

class State:
    config: Config
    has_ended: bool
    player: int
    reward: np.ndarray
    grid: np.ndarray
    actions: list[Action]

    def actions_at(self, source: np.ndarray) -> Action: ...
    def action_at(self, source: np.ndarray, target: np.ndarray) -> Action: ...

class Action:
    state: State
    source: np.ndarray
    target: np.ndarray

    def sample_next_state(self) -> State: ...
