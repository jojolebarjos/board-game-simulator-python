#ifndef MODULE_TENSOR_HPP
#define MODULE_TENSOR_HPP


#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

#include <game/tensor.hpp>


namespace nanobind {
namespace detail {



// https://github.com/wjakob/nanobind/blob/41dfbc7342d7146415ff0d2bb2bf63c73bdc8ceb/include/nanobind/eigen/dense.h


template <typename T, game::dim_t... N>
struct type_caster<game::tensor<T, N...>> {

    using Tensor = game::tensor<T, N...>;

    using NDArray = ndarray<
        T,
        numpy,
        shape<N...>,
        c_contig
    >;

    using NDArrayCaster = type_caster<NDArray>;

    NB_TYPE_CASTER(Tensor, NDArrayCaster::Name)


    bool from_python(handle src, uint8_t flags, cleanup_list *cleanup) noexcept {

        NDArrayCaster caster;

        if (!caster.from_python(src, flags, cleanup))
            return false;

        NDArray& array = caster.value;

        std::memcpy(value.data(), array.data(), array.size() * sizeof(T));

        return true;
    }

    // TODO move semantics and ref const
    static handle from_cpp(game::tensor<T, N...> value, rv_policy, cleanup_list* cleanup) noexcept {

        T* ptr = value.data();

        auto s = value.shape().to_array();

        constexpr int ndim = sizeof...(N);
        size_t shape[ndim];
        //int64_t strides[ndim];
        for (int i = 0; i < ndim; ++i) {
            shape[i] = s[i];
            //strides[] =
        }

        object owner;

        object o = steal(NDArrayCaster::from_cpp(
            NDArray(ptr, ndim, shape, owner/*, strides*/),
            rv_policy::copy, cleanup));

        return o.release();

        //return Py_BuildValue("if", value.x, value.y);
    }

};


}
}


#endif
