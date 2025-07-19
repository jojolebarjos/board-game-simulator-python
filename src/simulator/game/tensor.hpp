#ifndef MODULE_TENSOR_HPP
#define MODULE_TENSOR_HPP


#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

#include <game/tensor.hpp>


namespace nanobind {
namespace detail {


// TODO finalize this type caster
// https://github.com/wjakob/nanobind/blob/master/include/nanobind/stl/string.h
// https://github.com/wjakob/nanobind/blob/41dfbc7342d7146415ff0d2bb2bf63c73bdc8ceb/include/nanobind/eigen/dense.h
// https://github.com/wjakob/nanobind/blob/41dfbc7342d7146415ff0d2bb2bf63c73bdc8ceb/include/nanobind/ndarray.h#L68
// https://github.com/wjakob/nanobind/blob/master/include/nanobind/nb_cast.h#L358


template <typename T, game::dim_t... N>
struct type_caster<game::tensor<T, N...>> {

    using Shape = game::shape_t<N...>;

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

        if (array.ndim() != Shape::ndim)
            return false;

        std::array<game::dim_t, Shape::ndim> dims = {};
        for (size_t i = 0; i < Shape::ndim; ++i)
            dims[i] = array.shape(i);

        Shape shape = {};
        if (!shape.from_array(dims))
            return false;

        value.reshape(shape);

        std::memcpy(value.data(), array.data(), array.size() * sizeof(T));

        return true;
    }

    // TODO move semantics and ref const
    static handle from_cpp(game::tensor<T, N...> value, rv_policy, cleanup_list* cleanup) {

        T* ptr = value.data();
        Shape shape = value.shape();

        size_t dims[Shape::ndim];
        for (int i = 0; i < Shape::ndim; ++i)
            dims[i] = shape[i];

        object owner;

        object o = steal(NDArrayCaster::from_cpp(
            NDArray(ptr, Shape::ndim, dims, owner),
            rv_policy::copy,
            cleanup
        ));

        return o.release();
    }

};


}
}


#endif
