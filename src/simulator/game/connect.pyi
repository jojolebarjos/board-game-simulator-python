import numpy as np

class Config:
    num_players: int
    height: int
    width: int
    count: int

    def sample_initial_state(self) -> State: ...

class State:
    config: Config
    has_ended: bool
    player: int
    reward: np.ndarray
    grid: np.ndarray
    actions: list[Action]

    def action_at(self, column: int) -> Action: ...

class Action:
    state: State
    column: int

    def sample_next_state(self) -> State: ...
