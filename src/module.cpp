#include "./common.hpp"
#include "./object.hpp"
#include "./game.hpp"

#include <game/bounce.hpp>
#include <game/chinese_checkers.hpp>
#include <game/connect.hpp>


namespace {


bool define(PyObject* module) {
    return
        Game<game::bounce::Traits>::define(module, "Bounce") &&
        Game<game::chinese_checkers::Traits>::define(module, "ChineseCheckers") &&
        Game<game::connect::Traits<6, 7, 4>>::define(module, "Connect4");
}


int module_exec(PyObject* module) {
    if (define(module))
        return 0;
    Py_XDECREF(module);
    return -1;
}


PyModuleDef_Slot slots[] = {
    {Py_mod_exec, (void*)module_exec},
    {0, NULL},
};


PyMethodDef methods[] = {
    {NULL},
};


PyModuleDef module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "game._core",
    .m_doc = NULL,
    .m_size = 0,
    .m_methods = methods,
    .m_slots = slots,
};


}


PyMODINIT_FUNC PyInit__core() {

    import_array();
    if (PyErr_Occurred())
        return NULL;

    return PyModuleDef_Init(&::module);
}
