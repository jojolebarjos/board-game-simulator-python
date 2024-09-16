#ifndef MODULE_HELPER_HPP
#define MODULE_HELPER_HPP

#include <nanobind/nanobind.h>
#include <game/hash.hpp>


template <typename T>
void bind_comparisons(nanobind::class_<T>& cls) {
    cls
        .def("__eq__", [](T const& left, T const& right) { return left == right; })
        .def("__ne__", [](T const& left, T const& right) { return left != right; })
        .def("__lt__", [](T const& left, T const& right) { return left < right; })
        .def("__le__", [](T const& left, T const& right) { return left <= right; })
        .def("__gt__", [](T const& left, T const& right) { return left > right; })
        .def("__ge__", [](T const& left, T const& right) { return left >= right; })
        .def("__hash__", [](T const& obj) {
            Py_hash_t hash = game::hash_value(obj);
            if (hash == -1)
                hash = -2;
            return hash;
        });
}


#endif
