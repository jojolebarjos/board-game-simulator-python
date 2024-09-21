from abc import ABC, abstractmethod
import asyncio
from concurrent.futures import Executor, ThreadPoolExecutor
import random
import time

from textual.app import App, ComposeResult
from textual.containers import Grid

from simulator.game.bounce import Config, State, Action
from simulator.textual.bounce import BounceBoard


class Agent(ABC):
    """Abstract base class of AI player."""

    @abstractmethod
    def predict(self, state: State) -> dict[Action, float]:
        pass


class RandomAgent(Agent):
    """Uniform distribution."""

    def predict(self, state: State) -> dict[Action, float]:
        time.sleep(random.random())
        actions = state.actions
        return {action: 1 / len(actions) for action in actions}


class ArenaApp(App):
    """Many games played by AI."""

    DEFAULT_CSS = """

    Grid {
        grid-size: 4;
    }

    BounceBoard {
        border: round white;
    }

    """

    def __init__(self, agent: Agent, executor: Executor) -> None:
        self.agent = agent
        self.executor = executor
        super().__init__()

    def compose(self) -> ComposeResult:
        with Grid():
            for _ in range(8):
                board = BounceBoard(disabled=True)
                _ = asyncio.create_task(self._handle_board(board))
                yield board

    async def _handle_board(self, board: BounceBoard) -> None:
        loop = asyncio.get_running_loop()
        while True:
            state = BounceBoard.DEFAULT_CONFIG.sample_initial_state()
            board.state = state
            board.styles.border = ("round", "green")
            while not state.has_ended:
                policy = await loop.run_in_executor(self.executor, self.agent.predict, state)
                actions, weights = zip(*policy.items())
                [action] = random.choices(actions, weights)
                state = action.sample_next_state()
                board.state = state
            board.styles.border = ("round", "red")
            await asyncio.sleep(2.0)


if __name__ == "__main__":
    with ThreadPoolExecutor() as executor:
        agent = RandomAgent()
        app = ArenaApp(agent, executor)
        app.run()
