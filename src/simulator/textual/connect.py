from __future__ import annotations

from rich.segment import Segment

from textual import events
from textual.app import App, ComposeResult
from textual.geometry import Size
from textual.message import Message
from textual.reactive import reactive
from textual.strip import Strip
from textual.widget import Widget

from simulator.game.connect import Config, State, Action


class ConnectBoard(Widget, can_focus=True):
    """Render a Connect board."""

    COMPONENT_CLASSES = {
        "connectboard--empty",
        "connectboard--o",
        "connectboard--x",
        "connectboard--cursor",
    }

    # TODO is there a way to allow combinatorial style (e.g. .connectboard--empty:focus), without making children widgets?
    DEFAULT_CSS = """
    ConnectBoard {
        width: auto;
        height: auto;
        padding: 1;

        &> .connectboard--empty {
            color: gray;
        }

        &> .connectboard--o {
            color: blue;
        }

        &> .connectboard--x {
            color: red;
        }

        &> .connectboard--cursor {
            background: white;
        }
    }
    """

    BINDINGS = [
        ("backspace,delete,r", "reset", "Reset"),
        ("enter,space", "select", "Select"),
        ("left", "cursor_left", "Cursor Left"),
        ("right", "cursor_right", "Cursor Right"),
    ]

    MARKERS = {
        -1: ".",
        0: "O",
        1: "X",
    }

    DEFAULT_CONFIG = Config(6, 7, 4)

    state: reactive[State | None] = reactive(None)
    cursor_column: reactive[int] = reactive(0)

    class Reset(Message):
        """New game requested."""

        def __init__(self, board: ConnectBoard) -> None:
            self.board = board
            super().__init__()

    # TODO add event for highlighted action

    class Selected(Message):
        """Action selected."""

        def __init__(self, board: ConnectBoard, action: Action) -> None:
            self.board = board
            self.action = action
            super().__init__()

    def __init__(
        self,
        state: State | None = None,
        *,
        name: str | None = None,
        id: str | None = None,
        classes: str | None = None,
        disabled: bool = False,
    ) -> None:
        super().__init__(name=name, id=id, classes=classes, disabled=disabled)
        self.state = state

    def column_at(self, x: int) -> int:
        return (x + 1) // 2 - 1

    def watch_state(self, old_state: State | None, new_state: State | None) -> None:
        if new_state is not None:
            self.cursor_column = min(self.cursor_column, new_state.config.width - 1)
        else:
            self.cursor_column = 0

    def reset(self) -> None:
        if not self.disabled:
            self.post_message(self.Reset(self))

    def select(self, column: int | None = None) -> None:
        if not self.disabled and self.state is not None:
            if column is None:
                column = self.cursor_column
            try:
                action = self.state.action_at(column)
            except RuntimeError:
                return
            self.post_message(self.Selected(self, action))

    def action_reset(self) -> None:
        self.reset()

    def action_select(self) -> None:
        self.select()

    def action_cursor_left(self) -> None:
        if self.state is not None:
            self.cursor_column = (self.cursor_column - 1) % self.state.config.width

    def action_cursor_right(self) -> None:
        if self.state is not None:
            self.cursor_column = (self.cursor_column + 1) % self.state.config.width

    # TODO do we need to call event.stop()?

    def on_mouse_move(self, event: events.MouseMove) -> None:
        self.cursor_column = self.column_at(event.offset.x)

    def on_click(self, event: events.Click) -> None:
        self.cursor_column = self.column_at(event.offset.x)
        self.select()

    def get_content_width(self, container: Size, viewport: Size) -> int:
        if self.state is None:
            return 0
        return 2 * self.state.config.width + 1

    def get_content_height(self, container: Size, viewport: Size, width: int) -> int:
        if self.state is None:
            return 0
        return self.state.config.height

    def render_line(self, y: int) -> Strip:
        if self.state is None:
            return Strip.blank(self.size.width)

        grid = self.state.grid
        height, width = grid.shape

        if y >= height:
            return Strip.blank(self.size.width)

        styles = {
            -1: self.get_component_rich_style("connectboard--empty"),
            0: self.get_component_rich_style("connectboard--o"),
            1: self.get_component_rich_style("connectboard--x"),
        }

        cursor_style = self.get_component_rich_style("connectboard--cursor", partial=True)

        segments = []
        segments.append(Segment(" "))
        for x in range(width):
            player = grid[height - y - 1, x]
            style = styles[player]
            if not self.disabled and x == self.cursor_column:
                style = style + cursor_style
            segments.append(Segment(self.MARKERS[player], style))
            segments.append(Segment(" "))

        padded_width = len(segments)
        return Strip(segments, padded_width)


class ExampleApp(App):
    """Dummy example app."""

    def compose(self) -> ComposeResult:
        state = ConnectBoard.DEFAULT_CONFIG.sample_initial_state()
        yield ConnectBoard(state)

    def on_connect_board_reset(self, event: ConnectBoard.Reset) -> None:
        event.board.state = ConnectBoard.DEFAULT_CONFIG.sample_initial_state()

    def on_connect_board_selected(self, event: ConnectBoard.Selected) -> None:
        event.board.state = event.action.sample_next_state()


if __name__ == "__main__":
    app = ExampleApp()
    app.run()
