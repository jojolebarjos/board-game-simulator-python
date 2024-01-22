#include <vector>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include <game/connect.hpp>


// TODO use unnamed namespace?
// TODO __hash__
// TODO __repr__ / __str__
// TODO release GIL when calling some operations
// TODO add noexcept in many locations, to ensure that Traits are also no except


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

    using State = typename Traits::State;
	using Action = typename Traits::Action;

    struct StateObject {
        PyObject_HEAD
        State value;
    };

    struct ActionObject {
        PyObject_HEAD
        StateObject* state;
        Action value;
    };

    inline static PyTypeObject* State_Type = NULL;
    inline static PyTypeObject* Action_Type = NULL;

    // TODO add noexcept everywhere?

    static PyObject* State_sample_initial_state(PyObject* cls, PyObject*) {
        StateObject* state = PyObject_New(StateObject, State_Type);
        if (state) {
            new (&state->value) State();
            state->value.initialize();
        }
        return (PyObject*)state;
    }

    static void State_dealloc(StateObject* self) {
        self->value.~State();
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static PyObject* State_player(StateObject* self, void*) {
        unsigned player = self->value.get_player();
        return PyLong_FromUnsignedLong(player);
    }

    static PyObject* State_has_ended(StateObject* self, void*) {
        long has_ended = self->value.has_ended();
        return PyBool_FromLong(has_ended);
    }

    // TODO winner (return None if still in game)
    // TODO reward (list or numpy array?)

    // TODO tensor representation (as tuple of numpy arrays / scalars)

    static PyObject* State_actions(StateObject* self, void*) {
        // TODO ideally, should reuse the same vector, to avoid allocation
        std::vector<Action> actions;
        self->value.get_actions(actions);
        size_t count = actions.size();
        PyObject* tuple = PyTuple_New(count);
        if (tuple) {
            for (size_t i = 0; i < count; ++i) {
                ActionObject* action = PyObject_New(ActionObject, Action_Type);
                // TODO check NULL
                action->state = self;
                new (&action->value) Action(actions[i]);
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
        // TODO can only compare states that share the same context?
        bool value = _richcompare(left <=> right, op);
        return PyBool_FromLong(value);
    }

    static Py_hash_t State_hash(StateObject* self) {
        // TODO
        return 0;
    }

    static void Action_dealloc(ActionObject* self) {
        self->value.~Action();
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
            new (&self->value) State(self->state->value);
            state->value.apply(self->value);
        }
        return (PyObject*)state;
    }

    static PyObject* Action_richcompare(PyObject* self, PyObject* other, int op) {
        if (Py_TYPE(other) != Action_Type)
            Py_RETURN_NOTIMPLEMENTED;
        // TODO
        return PyBool_FromLong(0);
    }

    static Py_hash_t Action_hash(ActionObject* self) {
        // TODO
        return 0;
    }

    static bool define(PyObject* module) {

        // TODO properly name classes, so that there is no name clash when having more than one game
        // TODO maybe make Action inner classes?

        static PyGetSetDef State_getset[] = {
            {"player", (getter)State_player, NULL, NULL, NULL},
            {"has_ended", (getter)State_has_ended, NULL, NULL, NULL},
            {"actions", (getter)State_actions, NULL, NULL, NULL},
            {NULL}
        };

        static PyMethodDef State_methods[] = {
            {"sample_initial_state", (PyCFunction)State_sample_initial_state, METH_NOARGS | METH_CLASS, NULL},
            {NULL}
        };

        static PyType_Slot State_slots[] = {
            {Py_tp_new, PyType_GenericNew},
            {Py_tp_dealloc, (destructor)State_dealloc},
            {Py_tp_getset, State_getset},
            {Py_tp_methods, State_methods},
            {Py_tp_richcompare, State_richcompare},
            {Py_tp_hash, (hashfunc)State_hash},
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
            {Py_tp_richcompare, Action_richcompare},
            {Py_tp_hash, (hashfunc)Action_hash},
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
