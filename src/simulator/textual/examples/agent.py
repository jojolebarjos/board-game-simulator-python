from abc import ABC, abstractmethod
import asyncio
from concurrent.futures import Executor, ThreadPoolExecutor
import random
import time

from textual.app import App, ComposeResult

from simulator.game.connect import Config, State, Action
from simulator.textual.connect import ConnectBoard


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


class AgentApp(App):
    """Play against the agent."""

    def __init__(self, agent: Agent, executor: Executor) -> None:
        self.agent = agent
        self.executor = executor
        super().__init__()

    def compose(self) -> ComposeResult:
        board = ConnectBoard()
        board.state = ConnectBoard.DEFAULT_CONFIG.sample_initial_state()
        board.disabled = board.state.player != 0
        yield board

    async def on_connect_board_reset(self, event: ConnectBoard.Reset) -> None:
        await self._play(event.board, ConnectBoard.DEFAULT_CONFIG.sample_initial_state())

    async def on_connect_board_selected(self, event: ConnectBoard.Selected) -> None:
        await self._play(event.board, event.action.sample_next_state())

    async def _play(self, board: ConnectBoard, state: State) -> None:
        if state.player == 0:
            board.state = state
        else:
            board.state = state
            board.disabled = True
            _ = asyncio.create_task(self._play_agent(board, state))

    async def _play_agent(self, board: ConnectBoard, state: State) -> None:
        loop = asyncio.get_running_loop()
        while not state.has_ended and state.player != 0:
            policy = await loop.run_in_executor(self.executor, self.agent.predict, state)
            actions, weights = zip(*policy.items())
            [action] = random.choices(actions, weights)
            state = action.sample_next_state()
        board.state = state
        board.disabled = False
        board.focus()


if __name__ == "__main__":
    with ThreadPoolExecutor() as executor:
        agent = RandomAgent()
        app = AgentApp(agent, executor)
        app.run()
