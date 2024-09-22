#ifndef MODULE_JSON_HPP
#define MODULE_JSON_HPP


#include <string>
#include <stdexcept>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>


/*
 * Based on:
 *   https://github.com/pybind/pybind11_json
 */


namespace game {


inline nanobind::object from_json(nlohmann::json const& j) {

    if (j.is_null())
        return nanobind::none();

    if (j.is_boolean())
        return nanobind::bool_(j.get<bool>());

    if (j.is_number_unsigned())
        return nanobind::int_(j.get<nlohmann::json::number_unsigned_t>());

    if (j.is_number_integer())
        return nanobind::int_(j.get<nlohmann::json::number_integer_t>());

    if (j.is_number_float())
        return nanobind::float_(j.get<double>());

    if (j.is_string())
        return nanobind::str(j.get<std::string>().c_str());

    if (j.is_array()) {
        nanobind::list obj;
        for (std::size_t i = 0; i < j.size(); i++)
            obj.append(from_json(j[i]));
        return obj;
    }

    {
        nanobind::dict obj;
        for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it)
            obj[nanobind::str(it.key().c_str())] = from_json(it.value());
        return obj;
    }
}


inline nlohmann::json to_json(nanobind::handle const& obj) {

    if (obj.ptr() == nullptr || obj.is_none())
        return nullptr;

    if (nanobind::isinstance<nanobind::bool_>(obj))
        return nanobind::cast<bool>(obj);

    if (nanobind::isinstance<nanobind::int_>(obj))
        return nanobind::cast<nlohmann::json::number_integer_t>(obj);

    if (nanobind::isinstance<nanobind::float_>(obj))
        return nanobind::cast<double>(obj);

    if (nanobind::isinstance<nanobind::str>(obj))
        return nanobind::cast<std::string>(obj);

    if (nanobind::isinstance<nanobind::tuple>(obj) || nanobind::isinstance<nanobind::list>(obj)) {
        auto out = nlohmann::json::array();
        for (nanobind::handle const value : obj)
            out.push_back(to_json(value));
        return out;
    }

    if (nanobind::isinstance<nanobind::dict>(obj)) {
        auto out = nlohmann::json::object();
        for (const nanobind::handle key : obj)
            out[nanobind::cast<std::string>(nanobind::str(key))] = to_json(obj[key]);
        return out;
    }

    throw std::runtime_error("to_json not implemented for this type of object: " + nanobind::cast<std::string>(nanobind::repr(obj)));
}


}


namespace nanobind {
namespace detail {


template <>
struct type_caster<nlohmann::json> {

    NB_TYPE_CASTER(nlohmann::json, const_name("json"))

    bool from_python(handle src, uint8_t flags, cleanup_list *cleanup) noexcept {
        try {
            value = game::to_json(src);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    static handle from_cpp(nlohmann::json const& value, rv_policy, cleanup_list* cleanup) noexcept {
        return game::from_json(value).release();
    }
};


}
}


#endif
