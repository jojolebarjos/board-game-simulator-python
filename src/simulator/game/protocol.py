from __future__ import annotations

from typing import ClassVar, Protocol

import numpy as np


class ConfigLike(Protocol):
    State: ClassVar[type]

    num_players: int

    def sample_initial_state(self) -> StateLike: ...


class StateLike(Protocol):
    Action: ClassVar[type]

    config: ConfigLike
    has_ended: bool
    player: int
    reward: np.ndarray
    actions: list[ActionLike]


class ActionLike(Protocol):
    state: StateLike

    def sample_next_state(self) -> StateLike: ...
