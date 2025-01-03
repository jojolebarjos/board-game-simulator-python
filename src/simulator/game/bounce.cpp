#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <game/bounce.hpp>

#include "./helper.hpp"
#include "./json.hpp"
#include "./tensor.hpp"


namespace nb = nanobind;
using namespace nb::literals;

using namespace game;
using namespace game::bounce;


NB_MODULE(bounce, m) {

    nb::class_<Config> config(m, "Config");
    config
        .def(nb::new_([](tensor<int8_t, -1, -1> const& grid){ return std::make_shared<Config>(grid); }))
        .def_ro_static("num_players", &Config::num_players)
        .def_prop_ro("grid", &Config::get_grid)
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
        .def("actions_at", &State::get_actions_at)
        .def("action_at", &State::get_action_at)
        .def("to_json", &State::to_json)
        .def_static("from_json", &State::from_json);

    nb::class_<Action> action(m, "Action");
    action
        .def_ro("state", &Action::state)
        .def_prop_ro("source", &Action::get_source)
        .def_prop_ro("target", &Action::get_target)
        .def("sample_next_state", &Action::sample_next_state)
        .def("to_json", &Action::to_json)
        .def_static("from_json", &Action::from_json);

    bind_comparisons(config);
    bind_comparisons(state);
    bind_comparisons(action);

    config.def_prop_ro_static("State", [=](nb::handle){ return state; });
    state.def_prop_ro_static("Action", [=](nb::handle){ return action; });
}
