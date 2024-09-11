#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <game/bounce.hpp>
#include "./tensor.hpp"


namespace nb = nanobind;
using namespace nb::literals;

using namespace game;
using namespace game::bounce;


nb::module_ create_bounce_module(nb::module_ parent) {
    nb::module_ m = parent.def_submodule("bounce");

    nb::class_<Config>(m, "Config")
        .def(nb::new_([](tensor<int8_t, -1, -1> const& grid){ return std::make_shared<Config>(grid); }))
        .def_ro_static("num_players", &Config::num_players)
        .def_ro("grid", &Config::grid)
        .def("sample_initial_state", &Config::sample_initial_state);

    nb::class_<State>(m, "State")
        .def_ro("config", &State::config)
        .def_prop_ro("has_ended", &State::has_ended)
        .def_prop_ro("player", &State::get_player)
        .def_prop_ro("reward", &State::get_reward)
        .def_ro("grid", &State::grid)
        .def_prop_ro("actions", &State::actions)
        .def("actions_at", &State::actions_at)
        .def("action_at", &State::action_at);

    nb::class_<Action>(m, "Action")
        .def_ro("state", &Action::state)
        .def_ro("source", &Action::source)
        .def_ro("target", &Action::target)
        .def("sample_next_state", &Action::sample_next_state);

    return m;
}
