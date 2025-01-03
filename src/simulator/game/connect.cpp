#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <game/connect.hpp>

#include "./helper.hpp"
#include "./json.hpp"
#include "./tensor.hpp"


namespace nb = nanobind;
using namespace nb::literals;

using namespace game;
using namespace game::connect;


NB_MODULE(connect, m) {

    nb::class_<Config> config(m, "Config");
    config
        .def(nb::new_([](int height, int width, int count){ return std::make_shared<Config>(height, width, count); }))
        // TODO should work since https://github.com/wjakob/nanobind/pull/676: "height"_a, "width"_a, "count"_a
        .def_ro_static("num_players", &Config::num_players)
        .def_ro("height", &Config::height)
        .def_ro("width", &Config::width)
        .def_ro("count", &Config::count)
        .def("sample_initial_state", &Config::sample_initial_state)
        .def("to_json", &Config::to_json)
        .def_static("from_json", &Config::from_json);

    nb::class_<State> state(m, "State");
    state
        .def_ro("config", &State::config)
        .def_prop_ro("has_ended", &State::has_ended)
        .def_prop_ro("player", &State::get_player)
        .def_prop_ro("reward", &State::get_reward)
        .def_prop_ro("grid", &State::get_grid)
        .def_prop_ro("actions", &State::get_actions)
        .def("action_at", &State::get_action_at)
        .def("to_json", &State::to_json)
        .def_static("from_json", &State::from_json);

    nb::class_<Action> action(m, "Action");
    action
        .def_ro("state", &Action::state)
        .def_ro("column", &Action::column)
        .def("sample_next_state", &Action::sample_next_state)
        .def("to_json", &Action::to_json)
        .def_static("from_json", &Action::from_json);

    bind_comparisons(config);
    bind_comparisons(state);
    bind_comparisons(action);

    config.def_prop_ro_static("State", [=](nb::handle){ return state; });
    state.def_prop_ro_static("Action", [=](nb::handle){ return action; });
}
