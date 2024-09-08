#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <game/connect.hpp>
#include "./tensor.hpp"


namespace nb = nanobind;
using namespace nb::literals;

using namespace game::connect;


nb::module_ create_connect_module(nb::module_ parent) {
    nb::module_ m = parent.def_submodule("connect");

    nb::class_<Config>(m, "Config")
        .def(nb::new_([](int height, int width, int count){ return std::make_shared<Config>(height, width, count); }))
        // TODO should work since https://github.com/wjakob/nanobind/pull/676: "height"_a, "width"_a, "count"_a
        .def_ro_static("num_players", &Config::num_players)
        .def_ro("height", &Config::height)
        .def_ro("width", &Config::width)
        .def_ro("count", &Config::count)
        .def("sample_initial_state", &Config::sample_initial_state);

    nb::class_<State>(m, "State")
        .def_ro("config", &State::config)
        .def_prop_ro("has_ended", &State::has_ended)
        .def_prop_ro("player", &State::get_player)
        .def_prop_ro("reward", &State::get_reward)
        .def_ro("grid", &State::grid)
        .def_prop_ro("actions", &State::actions)
        .def("action_at", &State::action_at);

    nb::class_<Action>(m, "Action")
        .def_ro("state", &Action::state)
        .def_ro("column", &Action::column)
        .def("sample_next_state", &Action::sample_next_state);

    return m;
}
