#include <nanobind/nanobind.h>


namespace nb = nanobind;
using namespace nb::literals;


nb::module_ create_bounce_module(nb::module_ parent);
nb::module_ create_connect_module(nb::module_ parent);


NB_MODULE(game, m) {
    create_bounce_module(m);
    create_connect_module(m);
}
