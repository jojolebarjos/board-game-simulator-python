#ifndef MODULE_OBJECT_HPP
#define MODULE_OBJECT_HPP


#include "./common.hpp"

#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <tuple>

#include <nlohmann/json.hpp>

#include <game/tensor.hpp>


// TODO should use the templated base json, instead of the default `json` alias


struct python_exception {};


template<typename T> constexpr int as_dtype = NPY_NOTYPE;
template<> constexpr int as_dtype<int8_t> = NPY_INT8;
template<> constexpr int as_dtype<int16_t> = NPY_INT16;
template<> constexpr int as_dtype<int32_t> = NPY_INT32;
template<> constexpr int as_dtype<int64_t> = NPY_INT64;
template<> constexpr int as_dtype<uint8_t> = NPY_UINT8;
template<> constexpr int as_dtype<uint16_t> = NPY_UINT16;
template<> constexpr int as_dtype<uint32_t> = NPY_UINT32;
template<> constexpr int as_dtype<uint64_t> = NPY_UINT64;
template<> constexpr int as_dtype<float> = NPY_FLOAT;
template<> constexpr int as_dtype<double> = NPY_DOUBLE;


struct Object {
    PyObject* value;

    constexpr Object(PyObject* value = nullptr) : value(value) {}

    Object(Object const& other) : value(other.value) {
        Py_XINCREF(value);
    }

    constexpr Object(Object&& other) : value(other.value) {
        other.value = nullptr;
    }

    ~Object() {
        Py_XDECREF(value);
    }

    Object& operator=(Object const& right) {
        Py_XINCREF(right.value);
        Py_XDECREF(value);
        value = right.value;
        return *this;
    }

    Object& operator=(Object&& right) {
        if (this != &right) {
            Py_XDECREF(value);
            value = right.value;
            right.value = nullptr;
        }
        return *this;
    }

    constexpr explicit operator bool() const {
        return value;
    }

    constexpr PyObject* release() noexcept {
        PyObject* result = value;
        value = nullptr;
        return result;
    }
};


template <typename T>
Object to_object(T value) noexcept = delete;


Object to_object(bool value) noexcept {
    return PyBool_FromLong(value);
}


Object to_object(long value) noexcept {
    return PyLong_FromLong(value);
}


Object to_object(long long value) noexcept {
    return PyLong_FromLongLong(value);
}


Object to_object(unsigned long value) noexcept {
    return PyLong_FromUnsignedLong(value);
}


Object to_object(unsigned long long value) noexcept {
    return PyLong_FromUnsignedLongLong(value);
}


Object to_object(float value) noexcept {
    return PyFloat_FromDouble(value);
}


Object to_object(double value) noexcept {
    return PyFloat_FromDouble(value);
}


Object to_object(std::string const& value) noexcept {
    return PyUnicode_FromString(value.c_str());
}


template<typename T, size_t... Shape>
Object to_object(game::tensor<T, Shape...> const& value) noexcept {
    static_assert(as_dtype<T> != NPY_NOTYPE);
    Object array = PyArray_SimpleNew(value.ndim, (npy_intp const*)value.shape, as_dtype<T>);
    if (array) {
        void const* src = (void const*)&value;
        void* dst = PyArray_DATA((PyArrayObject*)array.value);
        memcpy(dst, src, sizeof(value));
    }
    return array;
}


Object to_object(nlohmann::json const& value) noexcept;


template<typename... T>
Object to_object(std::tuple<T...> const& value) noexcept {
    size_t size = sizeof...(T);
    Object tuple = PyTuple_New(size);
    if (tuple) {
        size_t i = 0;
        auto set = [&](auto v) {
            PyObject* o = to_object(v).release();
            PyTuple_SET_ITEM(tuple.value, i++, o);
            return o;
        };
        if (!std::apply(set, value))
            return nullptr;
    }
    return tuple;
}


template<typename T, typename Allocator>
Object to_object(std::vector<T, Allocator> const& value) noexcept {
    size_t size = value.size();
    Object list = PyList_New(size);
    if (list) {
        for (size_t i = 0; i < size; ++i) {
            Object item = to_object(value[i]);
            if (!item)
                return nullptr;
            PyList_SET_ITEM(list.value, i, item.release());
        }
    }
    return list;
}


template<typename K, typename V, typename Compare, typename Allocator>
Object to_object(std::map<K, V, Compare, Allocator> const& value) noexcept {
    Object dict = PyDict_New();
    if (dict) {
        for (const auto& [k, v] : value) {
            Object ko = to_object(k);
            if (!ko)
                return nullptr;
            Object vo = to_object(v);
            if (!vo)
                return nullptr;
            if (PyDict_SetItem(dict.value, ko.value, vo.value))
                return nullptr;
        }
    }
    return dict;
}


Object to_object(nlohmann::json const& value) noexcept {
    switch (value.type()) {

    case nlohmann::json::value_t::null:
        Py_RETURN_NONE;

    case nlohmann::json::value_t::object:
        return to_object(value.get_ref<nlohmann::json::object_t const&>());

    case nlohmann::json::value_t::array:
        return to_object(value.get_ref<nlohmann::json::array_t const&>());

    case nlohmann::json::value_t::string:
        return to_object(value.get_ref<nlohmann::json::string_t const&>());

    case nlohmann::json::value_t::boolean:
        return to_object(value.get_ref<nlohmann::json::boolean_t const&>());

    case nlohmann::json::value_t::number_integer:
        return to_object(value.get_ref<nlohmann::json::number_integer_t const&>());

    case nlohmann::json::value_t::number_unsigned:
        return to_object(value.get_ref<nlohmann::json::number_unsigned_t const&>());

    case nlohmann::json::value_t::number_float:
        return to_object(value.get_ref<nlohmann::json::number_float_t const&>());

    default:
        return PyErr_Format(PyExc_NotImplementedError, "JSON");
    }
}


template <typename T>
T from_object(Object const& object) = delete;


template <>
bool from_object<bool>(Object const& object) {
    int value = PyObject_IsTrue(object.value);
    if (value == -1)
        throw python_exception();
    return value;
}


template <>
long long from_object<long long>(Object const& object) {
    long long value = PyLong_AsLongLong(object.value);
    if (value == -1 && PyErr_Occurred())
        throw python_exception();
    return value;
}


template <>
double from_object<double>(Object const& object) {
    double value = PyFloat_AsDouble(object.value);
    if (value == -1 && PyErr_Occurred())
        throw python_exception();
    return value;
}


template <>
std::string from_object<std::string>(Object const& object) {
    char const* value = PyUnicode_AsUTF8(object.value);
    if (!value)
        throw python_exception();
    return value;
}


// TODO tensor
// TODO std::tuple
// TODO std::vector
// TODO std::map


template <>
nlohmann::json from_object<nlohmann::json>(Object const& object) {

    if (object.value == Py_None)
        return nullptr;

    if (object.value == Py_True)
        return true;

    if (object.value == Py_False)
        return false;

    if (PyDict_Check(object.value)) {
        nlohmann::json j(nlohmann::json::value_t::object);
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(object.value, &pos, &key, &value)) {
            char const* k = PyUnicode_AsUTF8(key);
            if (!k)
                throw python_exception();
            j[k] = from_object<nlohmann::json>(value);
        }
        return j;
    }

    if (PyList_Check(object.value)) {
        nlohmann::json j(nlohmann::json::value_t::array);
        Py_ssize_t size = PyList_GET_SIZE(object.value);
        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject* value = PyList_GET_ITEM(object.value, i);
            j.emplace_back(from_object<nlohmann::json>(value));
        }
        return j;
    }

    // TODO tuple?

    if (PyUnicode_Check(object.value))
        return from_object<std::string>(object.value);

    if (PyLong_Check(object.value))
        return from_object<long long>(object.value);

    if (PyFloat_Check(object.value))
        return from_object<double>(object.value);

    PyErr_Format(PyExc_ValueError, "not JSON: %R", object.value);
    throw python_exception();
}


#endif
