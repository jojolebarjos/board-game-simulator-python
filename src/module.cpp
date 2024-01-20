#include <type_traits>
#include <vector>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include <game/connect.hpp>


// TODO __hash__, __eq__
// TODO __lt__
// TODO __repr__ / __str__


template <typename T>
constexpr bool _richcompare(T comparison, int op) {
    switch (op) {
    case Py_LT:
        return comparison < 0;
    case Py_LE:
        return comparison <= 0;
    case Py_EQ:
        return comparison == 0;
    case Py_NE:
        return comparison != 0;
    case Py_GT:
        return comparison > 0;
    case Py_GE:
        return comparison >= 0;
    }
    return false;
}


template <typename Traits>
struct Definition {

	using Context = typename Traits::Context;
    using State = typename Traits::State;
	using Action = typename Traits::Action;

    // TODO probably need to remove this restriction
    static_assert(std::is_trivial_v<State>);
    static_assert(std::is_trivial_v<Action>);

    struct ContextObject {
        PyObject_HEAD
        Context value;
    };

    struct StateObject {
        PyObject_HEAD
        ContextObject* context;
        State value;
    };

    struct ActionObject {
        PyObject_HEAD
        StateObject* state;
        Action value;
    };

    inline static PyTypeObject* Context_Type = NULL;
    inline static PyTypeObject* State_Type = NULL;
    inline static PyTypeObject* Action_Type = NULL;

    static int Context_init(ContextObject* self, PyObject* args, PyObject* kwargs) {
        new (&self->value) Context();
        return 0;
    }

    static void Context_dealloc(ContextObject* self) {
        self->value.~Context();
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static PyObject* Context_sample_initial_state(ContextObject* self, PyObject*) {
        StateObject* state = PyObject_New(StateObject, State_Type);
        if (state) {
            state->context = self;
            state->value = self->value.sample_initial_state();
            Py_INCREF(self);
        }
        return (PyObject*)state;
    }

    static void State_dealloc(StateObject* self) {
        Py_DECREF(self->context);
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static int State_traverse(StateObject* self, visitproc visit, void* arg) {
        Py_VISIT(self->context);
        return 0;
    }

    static PyObject* State_player(StateObject* self, void*) {
        unsigned player = self->context->value.get_player(self->value);
        return PyLong_FromUnsignedLong(player);
    }

    static PyObject* State_has_ended(StateObject* self, void*) {
        bool has_ended = self->context->value.has_ended(self->value);
        return PyBool_FromLong(has_ended);
    }

    // TODO winner (return None if still in game)
    // TODO reward (list or numpy array?)

    // TODO tensor representation (as tuple of numpy arrays / scalars)

    static PyObject* State_actions(StateObject* self, void*) {
        // TODO ideally, should reuse the same vector, to avoid allocation
        std::vector<Action> actions;
        self->context->value.get_actions(actions, self->value);
        size_t count = actions.size();
        PyObject* tuple = PyTuple_New(count);
        if (tuple) {
            for (size_t i = 0; i < count; ++i) {
                ActionObject* action = PyObject_New(ActionObject, Action_Type);
                // TODO check NULL
                action->state = self;
                action->value = actions[i];
                Py_INCREF(self);
                PyTuple_SET_ITEM(tuple, i, action);
            }
        }
        return tuple;
    }

    static PyObject* State_richcompare(PyObject* self, PyObject* other, int op) {
        if (Py_TYPE(other) != State_Type)
            Py_RETURN_NOTIMPLEMENTED;
        State& left = ((StateObject*)self)->value;
        State& right = ((StateObject*)other)->value;
        bool value = _richcompare(left <=> right, op);
        return PyBool_FromLong(value);
    }

    static void Action_dealloc(ActionObject* self) {
        Py_DECREF(self->state);
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static int Action_traverse(ActionObject* self, visitproc visit, void* arg) {
        Py_VISIT(self->state);
        return 0;
    }

    static PyObject* Action_sample_next_state(ActionObject* self, PyObject*) {
        StateObject* state = PyObject_New(StateObject, State_Type);
        if (state) {
            ContextObject* context = self->state->context;
            state->context = context;
            state->value = context->value.sample_next_state(self->state->value, self->value);
            Py_INCREF(context);
        }
        return (PyObject*)state;
    }

    static bool define(PyObject* module) {

        static PyMethodDef Context_methods[] = {
            {"sample_initial_state", (PyCFunction)Context_sample_initial_state, METH_NOARGS, NULL},
            {NULL}
        };

        static PyType_Slot Context_slots[] = {
            {Py_tp_new, PyType_GenericNew},
            {Py_tp_init, Context_init},
            {Py_tp_dealloc, (destructor)Context_dealloc},
            {Py_tp_methods, Context_methods},
            {0, NULL}
        };

        static PyType_Spec Context_spec = {
            "game._core.Context",
            sizeof(ContextObject),
            0,
            Py_TPFLAGS_DEFAULT,
            Context_slots
        };

        Context_Type = (PyTypeObject*)PyType_FromSpec(&Context_spec);
        if (PyModule_AddObjectRef(module, "Context", (PyObject*)Context_Type) < 0)
            return false;

        static PyGetSetDef State_getset[] = {
            {"player", (getter)State_player, NULL, NULL, NULL},
            {"has_ended", (getter)State_has_ended, NULL, NULL, NULL},
            {"actions", (getter)State_actions, NULL, NULL, NULL},
            {NULL}
        };

        static PyType_Slot State_slots[] = {
            {Py_tp_new, PyType_GenericNew},
            {Py_tp_dealloc, (destructor)State_dealloc},
            {Py_tp_traverse, State_traverse},
            {Py_tp_getset, State_getset},
            {Py_tp_richcompare, State_richcompare},
            {0, NULL}
        };

        static PyType_Spec State_spec = {
            "game._core.State",
            sizeof(StateObject),
            0,
            Py_TPFLAGS_DEFAULT,
            State_slots
        };

        State_Type = (PyTypeObject*)PyType_FromSpec(&State_spec);
        if (PyModule_AddObjectRef(module, "State", (PyObject*)State_Type) < 0)
            return false;

        static PyMethodDef Action_methods[] = {
            {"sample_next_state", (PyCFunction)Action_sample_next_state, METH_NOARGS, NULL},
            {NULL}
        };

        static PyType_Slot Action_slots[] = {
            {Py_tp_new, PyType_GenericNew},
            {Py_tp_dealloc, (destructor)Action_dealloc},
            {Py_tp_traverse, Action_traverse},
            {Py_tp_methods, Action_methods},
            // TODO {Py_tp_richcompare, Action_richcompare},
            {0, NULL}
        };

        static PyType_Spec Action_spec = {
            "game._core.Action",
            sizeof(ActionObject),
            0,
            Py_TPFLAGS_DEFAULT,
            Action_slots
        };

        Action_Type = (PyTypeObject*)PyType_FromSpec(&Action_spec);
        if (PyModule_AddObjectRef(module, "Action", (PyObject*)Action_Type) < 0)
            return false;

        return true;
    }
};


bool define(PyObject* module) {
    return Definition<game::connect::Traits<6, 7, 4>>::define(module);
}


static int module_exec(PyObject* module) {

    PyModule_AddIntConstant(module, "X", 42);

    if (define(module))
        return 0;
    Py_XDECREF(module);
    return -1;
}


PyModuleDef_Slot slots[] = {
    {Py_mod_exec, (void*)module_exec},
    {0, NULL},
};


PyModuleDef module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "game._core",
    .m_doc = NULL,
    .m_size = 0,
    .m_slots = slots,
};


PyMODINIT_FUNC PyInit__core() {
    return PyModuleDef_Init(&module);
}
