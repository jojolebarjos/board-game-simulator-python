#include <pybind11/pybind11.h>

#include <game/connect.hpp>

int add(int i, int j) {
    game::connect::Action k = 3;
    return i + j + k;
}

PYBIND11_MODULE(_core, m) {
    m.doc() = "pybind11 example plugin";
    m.def("add", &add, "A function that adds two numbers");
}
