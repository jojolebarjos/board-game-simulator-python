#ifndef MODULE_HELPER_HPP
#define MODULE_HELPER_HPP

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>

#include <game/hash.hpp>


template <typename T>
void bind_comparisons(nanobind::class_<T>& cls) {
    cls
        .def(nanobind::self == nanobind::self)
        .def(nanobind::self != nanobind::self)
        .def(nanobind::self < nanobind::self)
        .def(nanobind::self <= nanobind::self)
        .def(nanobind::self > nanobind::self)
        .def(nanobind::self >= nanobind::self)
        .def("__hash__", [](T const& obj) {
            Py_hash_t hash = game::hash_value(obj);
            if (hash == -1)
                hash = -2;
            return hash;
        });
}


#endif
