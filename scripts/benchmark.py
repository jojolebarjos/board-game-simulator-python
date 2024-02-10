import random
import time

from game import *

State = ConnectState

duration = 5.0

action_count = 0
game_count = 0

start = time.perf_counter()
end = start + duration

while time.perf_counter() < end:
    state = State.sample_initial_state()

    while not state.has_ended:
        actions = state.actions
        action = random.choice(actions)

        state = action.sample_next_state()

        action_count += 1

    game_count += 1

duration = time.perf_counter() - start

print(f"Actions: {action_count}")
print(f"Games: {game_count}")
print(f"Average actions per game: {action_count / game_count:.02f}")
print(f"Time per action: {1e6 * duration / action_count:.06f} us")
print(f"Time per game: {1e6 * duration / game_count:.06f} us")
# TODO also report rewards
