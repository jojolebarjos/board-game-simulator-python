from __future__ import annotations

import numpy as np

from rich.segment import Segment

from textual import events
from textual.app import App, ComposeResult
from textual.geometry import Offset
from textual.message import Message
from textual.reactive import reactive
from textual.strip import Strip
from textual.widget import Widget

from simulator.game.bounce import Config, State, Action


class BounceBoard(Widget, can_focus=True):
    """Render a Bounce board."""

    COMPONENT_CLASSES = {
        "bounceboard--empty",
        "bounceboard--piece",
        "bounceboard--cursor",
        "bounceboard--source",
        "bounceboard--target",
    }

    DEFAULT_CSS = """
    BounceBoard {
        padding: 1;

        &> .bounceboard--empty {
            color: gray;
        }

        &> .bounceboard--piece {
            color: green;
        }

        &> .bounceboard--cursor {
            background: white;
        }

        &> .bounceboard--source {
            background: yellow;
        }

        &> .bounceboard--target {
            background: gray;
        }
    }
    """

    BINDINGS = [
        ("backspace,delete,r", "reset", "Reset"),
        ("enter,space", "select", "Select"),
        ("left", "cursor_left", "Cursor Left"),
        ("right", "cursor_right", "Cursor Right"),
        ("up", "cursor_up", "Cursor Up"),
        ("down", "cursor_down", "Cursor Down"),
    ]

    DEFAULT_GRID = np.array([
        [0, 0, 0, 0, 0, 0],
        [1, 2, 3, 3, 2, 1],
        [0, 0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0, 0],
        [1, 2, 3, 3, 2, 1],
        [0, 0, 0, 0, 0, 0],
    ])
    DEFAULT_STATE = Config(DEFAULT_GRID).sample_initial_state()

    state: reactive[State] = reactive(DEFAULT_STATE)
    cursor_offset: reactive[Offset] = reactive(Offset(0, 0))
    source_offset: reactive[Offset | None] = reactive(None)

    class Reset(Message):
        """New game requested."""

        def __init__(self, board: BounceBoard) -> None:
            self.board = board
            super().__init__()

    class Selected(Message):
        """Action selected."""

        def __init__(self, board: BounceBoard, action: Action) -> None:
            self.board = board
            self.action = action
            super().__init__()

    def offset_at(self, offset: Offset) -> Offset:
        height, _ = self.state.grid.shape
        return Offset((offset.x + 1) // 2 - 1, height - offset.y)

    def action_at(self, source_offset: Offset, target_offset: Offset) -> Action | None:
        try:
            return self.state.action_at(np.array(source_offset), np.array(target_offset))
        except RuntimeError:
            return None

    def actions_at(self, source_offset: Offset) -> list[Action]:
        try:
            return self.state.actions_at(np.array(source_offset))
        except RuntimeError:
            return []

    def watch_state(self, old_state: State, new_state: State) -> None:
        height, width = new_state.grid.shape
        self.cursor_offset = Offset(
            min(self.cursor_offset.x, width - 1),
            min(self.cursor_offset.y, height - 1),
        )
        self.source_offset = None

    def reset(self) -> None:
        self.post_message(self.Reset(self))

    def select(self, offset: Offset | None = None) -> None:
        if offset is None:
            offset = self.cursor_offset
        if self.source_offset is None:
            actions = self.actions_at(offset)
            if len(actions) > 0:
                self.source_offset = offset
        else:
            action = self.action_at(self.source_offset, offset)
            if action is None:
                actions = self.actions_at(offset)
                if len(actions) > 0:
                    self.source_offset = offset
                else:
                    self.source_offset = None
            else:
                self.post_message(self.Selected(self, action))

    def action_reset(self) -> None:
        self.reset()
    
    def action_select(self) -> None:
        self.select()

    def action_cursor_left(self) -> None:
        _, width = self.state.grid.shape
        x = (self.cursor_offset.x - 1) % width
        self.cursor_offset = Offset(x, self.cursor_offset.y)

    def action_cursor_right(self) -> None:
        _, width = self.state.grid.shape
        x = (self.cursor_offset.x + 1) % width
        self.cursor_offset = Offset(x, self.cursor_offset.y)

    def action_cursor_up(self) -> None:
        height, _ = self.state.grid.shape
        y = (self.cursor_offset.y + 1) % height
        self.cursor_offset = Offset(self.cursor_offset.x, y)

    def action_cursor_down(self) -> None:
        height, _ = self.state.grid.shape
        y = (self.cursor_offset.y - 1) % height
        self.cursor_offset = Offset(self.cursor_offset.x, y)

    # TODO do we need to call event.stop()?

    def on_mouse_move(self, event: events.MouseMove) -> None:
        self.cursor_offset = self.offset_at(event.offset)

    def on_click(self, event: events.Click) -> None:
        self.cursor_offset = self.offset_at(event.offset)
        self.select()

    def render_line(self, y: int) -> Strip:
        grid = self.state.grid
        height, width = grid.shape
        y = height - y - 1

        if y < 0 or y >= height:
            return Strip.blank(self.size.width)

        source_offset = self.source_offset
        if source_offset is None:
            source_offset = self.cursor_offset
        actions = self.actions_at(source_offset)
        action_offsets = {Offset(*action.target) for action in actions}

        empty_style = self.get_component_rich_style("bounceboard--empty")
        # TODO maybe highlight in blue/red the bottom/top rows?
        piece_style = self.get_component_rich_style("bounceboard--piece")
        source_style = self.get_component_rich_style("bounceboard--source", partial=True)
        target_style = self.get_component_rich_style("bounceboard--target", partial=True)
        cursor_style = self.get_component_rich_style("bounceboard--cursor", partial=True)

        segments = []
        segments.append(Segment(" "))
        for x in range(width):
            offset = Offset(x, y)
            value = grid[y, x]
            if value > 0:
                assert value < 10
                marker = str(value)
                style = piece_style
            else:
                if y == 0 or y == height - 1:
                    marker = "-"
                else:
                    marker = "."
                style = empty_style
            if offset == self.source_offset:
                style = style + source_style
            if offset in action_offsets:
                style = style + target_style
            if offset == self.cursor_offset:
                style = style + cursor_style
            segments.append(Segment(marker, style))
            segments.append(Segment(" "))

        padded_width = len(segments)
        return Strip(segments, padded_width)


class ExampleApp(App):
    """Dummy example app."""

    def compose(self) -> ComposeResult:
        yield BounceBoard()
    
    def on_bounce_board_reset(self, event: BounceBoard.Reset) -> None:
        event.board.state = BounceBoard.DEFAULT_STATE
        
    def on_bounce_board_selected(self, event: BounceBoard.Selected) -> None:
        event.board.state = event.action.sample_next_state()


if __name__ == "__main__":
    app = ExampleApp()
    app.run()
