#include <nanobind/nanobind.h>

#include "./tensor.hpp"


// https://github.com/wjakob/nanobind/blob/master/include/nanobind/stl/string.h
// https://github.com/wjakob/nanobind/blob/41dfbc7342d7146415ff0d2bb2bf63c73bdc8ceb/include/nanobind/eigen/dense.h
// https://github.com/wjakob/nanobind/blob/41dfbc7342d7146415ff0d2bb2bf63c73bdc8ceb/include/nanobind/ndarray.h#L68
// https://github.com/wjakob/nanobind/blob/master/include/nanobind/nb_cast.h#L358


namespace nb = nanobind;
using namespace nb::literals;


int add(int a, int b) { return a + b; }


game::tensor<float, 2> foo(game::tensor<float, 2> x) {
    return x;
}


nb::module_ create_connect_module(nb::module_ parent);


NB_MODULE(game, m) {
    m.def("add", &add);
    m.def("foo", &foo);

    create_connect_module(m);
}
