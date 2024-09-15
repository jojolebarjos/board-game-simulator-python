#ifndef MODULE_HELPER_HPP
#define MODULE_HELPER_HPP

#include <nanobind/nanobind.h>


template <typename T>
void bind_comparisons(nanobind::class_<T>& cls) {
    cls
        .def("__eq__", [](T const& left, T const& right) { return (left <=> right) == 0; })
        .def("__ne__", [](T const& left, T const& right) { return (left <=> right) != 0; })
        .def("__lt__", [](T const& left, T const& right) { return (left <=> right) < 0; })
        .def("__le__", [](T const& left, T const& right) { return (left <=> right) <= 0; })
        .def("__gt__", [](T const& left, T const& right) { return (left <=> right) > 0; })
        .def("__ge__", [](T const& left, T const& right) { return (left <=> right) >= 0; });
    // TODO hash
}


#endif
