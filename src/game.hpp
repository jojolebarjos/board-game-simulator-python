#ifndef MODULE_GAME_HPP
#define MODULE_GAME_HPP


#include "./common.hpp"
#include "./object.hpp"


// TODO __repr__ / __str__
// TODO release GIL when calling some operations


template <typename T>
constexpr bool richcompare(T comparison, int op) {
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




template <typename _Traits>
struct Game {

    using Traits = typename _Traits;
    using State = Traits::State;
	using Action = Traits::Action;

    // TODO maybe allow for throwable constructors (i.e. careful with destructor)?
    static_assert(std::is_nothrow_default_constructible_v<State>);
    static_assert(std::is_nothrow_copy_constructible_v<State>);
    static_assert(std::is_nothrow_default_constructible_v<Action>);
    static_assert(std::is_nothrow_copy_constructible_v<Action>);

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

    static void check_state(StateObject* self) {
        if (self->value.player < -1 || self->value.player > 1)
            throw std::runtime_error("player");
    }

    static void State_dealloc(StateObject* self) noexcept {
        self->value.~State();
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static void Action_dealloc(ActionObject* self) noexcept {
        self->value.~Action();
        Py_XDECREF(self->state);
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static int Action_traverse(ActionObject* self, visitproc visit, void* arg) noexcept {
        Py_VISIT(self->state);
        return 0;
    }

    static PyObject* State_sample_initial_state(PyObject* cls, PyObject*) noexcept {
        StateObject* state = PyObject_New(StateObject, State_Type);
        if (state) {
            new (&state->value) State();
            try {
                Traits::initialize(state->value);
            }
            catch (const std::exception& e) {
                Py_DECREF(state);
                return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
            }
        }
        return (PyObject*)state;
    }

    static PyObject* State_has_ended(StateObject* self, void*) noexcept {
        bool has_ended = Traits::has_ended(self->value);
        return PyBool_FromLong(has_ended);
    }

    static PyObject* State_player(StateObject* self, void*) noexcept {
        int player = Traits::get_player(self->value);
        if (player < 0)
            Py_RETURN_NONE;
        return PyLong_FromLong(player);
    }

    static PyObject* State_winner(StateObject* self, void*) noexcept {
        int player = Traits::get_winner(self->value);
        if (player < 0)
            Py_RETURN_NONE;
        return PyLong_FromLong(player);
    }

    // TODO reward (list or numpy array?)

    static PyObject* State_get_tensors(StateObject* self, PyObject* args) noexcept {
        PyObject* mode = Py_None;
        if (!PyArg_ParseTuple(args, "|O:get_tensors", &mode))
            return NULL;
        try {
            // TODO handle mode
            auto tuple = Traits::get_tensors(self->value);
            return to_object(tuple).release();
        }
        catch (const std::exception& e) {
            return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
        }
    }

    static PyObject* State_actions(StateObject* self, void*) noexcept {
        check_state(self);
        try {
            // TODO ideally, should reuse the same vector, to avoid allocation?
            std::vector<Action> actions;
            Traits::get_actions(self->value, actions);
            size_t count = actions.size();
            PyObject* tuple = PyTuple_New(count);
            if (tuple) {
                for (size_t i = 0; i < count; ++i) {
                    ActionObject* action = PyObject_New(ActionObject, Action_Type);
                    if (!action) {
                        Py_DECREF(tuple);
                        return NULL;
                    }
                    new (&action->value) Action(actions[i]);
                    if (action->value > 6)
                        return PyErr_Format(PyExc_RuntimeError, "XXX %d", action->value);
                    action->state = self;
                    Py_INCREF(self);
                    PyTuple_SET_ITEM(tuple, i, action);
                }
            }
            return tuple;
        }
        catch (const std::exception& e) {
            return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
        }
    }

    static PyObject* Action_sample_next_state(ActionObject* self, PyObject*) noexcept {
        if (self->value > 6)
            return PyErr_Format(PyExc_RuntimeError, "YYY %d", self->value);
        StateObject* state = PyObject_New(StateObject, State_Type);
        if (state) {
            new (&state->value) State(self->state->value);
            try {
                Traits::apply(state->value, self->value);
            }
            catch (const std::exception& e) {
                Py_DECREF(state);
                return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
            }
            check_state(state);
        }
        return (PyObject*)state;
    }

    static PyObject* State_to_json(StateObject* self, PyObject*) noexcept {
        try {
            nlohmann::json j = Traits::to_json(self->value);
            return to_object(j).release();
        }
        catch (const std::exception& e) {
            return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
        }
    }

    static PyObject* Action_to_json(ActionObject* self, PyObject*) noexcept {
        try {
            nlohmann::json j = Traits::to_json(self->state->value, self->value);
            return to_object(j).release();
        }
        catch (const std::exception& e) {
            return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
        }
    }

    static PyObject* State_from_json(PyObject* cls, PyObject* arg) noexcept {
        StateObject* state = nullptr;
        try {
            auto j = from_object<nlohmann::json>(arg);
            state = PyObject_New(StateObject, State_Type);
            if (state) {
                new (&state->value) State();
                Traits::from_json(state->value, j);
            }
            return (PyObject*)state;
        }
        catch (python_exception) {
            Py_XDECREF(state);
            return NULL;
        }
        catch (const std::exception& e) {
            Py_XDECREF(state);
            return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
        }
        return (PyObject*)state;
    }

    static PyObject* Action_from_json(PyObject* cls, PyObject* args) noexcept {
        StateObject* state;
        PyObject* arg;
        if (!PyArg_ParseTuple(args, "O!O:from_json", State_Type, &state, &arg))
            return NULL;
        ActionObject* action = nullptr;
        try {
            auto j = from_object<nlohmann::json>(arg);
            action = PyObject_New(ActionObject, Action_Type);
            if (action) {
                new (&action->value) Action();
                action->state = state;
                Py_INCREF(state);
                Traits::from_json(state->value, action->value, j);
            }
            return (PyObject*)action;
        }
        catch (python_exception) {
            Py_XDECREF(action);
            return NULL;
        }
        catch (const std::exception& e) {
            Py_XDECREF(action);
            return PyErr_Format(PyExc_RuntimeError, "internal error: %s", e.what());
        }
        return (PyObject*)action;
    }

    static PyObject* State_richcompare(PyObject* self, PyObject* other, int op) noexcept {
        if (Py_TYPE(other) != State_Type)
            Py_RETURN_NOTIMPLEMENTED;
        StateObject* left = (StateObject*)self;
        StateObject* right = (StateObject*)other;
        auto comparison = Traits::compare(left->value, right->value);
        bool value = richcompare(comparison, op);
        return PyBool_FromLong(value);
    }

    static PyObject* Action_richcompare(PyObject* self, PyObject* other, int op) noexcept {
        if (Py_TYPE(other) != Action_Type)
            Py_RETURN_NOTIMPLEMENTED;
        ActionObject* left = (ActionObject*)self;
        ActionObject* right = (ActionObject*)other;
        auto comparison = Traits::compare(left->state->value, left->value, right->state->value, right->value);
        bool value = richcompare(comparison, op);
        return PyBool_FromLong(value);
    }

    static Py_hash_t State_hash(StateObject* self) noexcept {
        size_t unsigned_hash = Traits::hash(self->value);
        Py_hash_t hash = (Py_ssize_t)unsigned_hash;
        if (hash == -1)
            hash = -2;
        return hash;
    }

    static Py_hash_t Action_hash(ActionObject* self) noexcept {
        size_t unsigned_hash = Traits::hash(self->state->value, self->value);
        Py_hash_t hash = (Py_ssize_t)unsigned_hash;
        if (hash == -1)
            hash = -2;
        return hash;
    }

    static bool define(PyObject* module, std::string const& name) noexcept {

        static PyGetSetDef State_getset[] = {
            {"player", (getter)State_player, NULL, NULL, NULL},
            {"has_ended", (getter)State_has_ended, NULL, NULL, NULL},
            {"actions", (getter)State_actions, NULL, NULL, NULL},
            {NULL}
        };

        static PyMethodDef State_methods[] = {
            {"sample_initial_state", (PyCFunction)State_sample_initial_state, METH_NOARGS | METH_CLASS, NULL},
            {"get_tensors", (PyCFunction)State_get_tensors, METH_VARARGS, NULL},
            {"to_json", (PyCFunction)State_to_json, METH_NOARGS, NULL},
            {"from_json", (PyCFunction)State_from_json, METH_O | METH_CLASS, NULL},
            {NULL}
        };

        static PyType_Slot State_slots[] = {
            {Py_tp_dealloc, (destructor)State_dealloc},
            {Py_tp_getset, State_getset},
            {Py_tp_methods, State_methods},
            {Py_tp_richcompare, State_richcompare},
            {Py_tp_hash, (hashfunc)State_hash},
            {0, NULL}
        };

        static std::string State_name = "game._core." + name + "State";

        static PyType_Spec State_spec = {
            State_name.c_str(),
            sizeof(StateObject),
            0,
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
            State_slots
        };

        State_Type = (PyTypeObject*)PyType_FromSpec(&State_spec);
        if (PyModule_AddObjectRef(module, &State_name[11], (PyObject*)State_Type) < 0)
            return false;

        static PyMethodDef Action_methods[] = {
            {"sample_next_state", (PyCFunction)Action_sample_next_state, METH_NOARGS, NULL},
            {"to_json", (PyCFunction)Action_to_json, METH_NOARGS, NULL},
            {"from_json", (PyCFunction)Action_from_json, METH_VARARGS | METH_CLASS, NULL},
            {NULL}
        };

        static PyType_Slot Action_slots[] = {
            {Py_tp_dealloc, (destructor)Action_dealloc},
            {Py_tp_traverse, Action_traverse},
            {Py_tp_methods, Action_methods},
            {Py_tp_richcompare, Action_richcompare},
            {Py_tp_hash, (hashfunc)Action_hash},
            {0, NULL}
        };

        static std::string Action_name = "game._core." + name + "Action";

        static PyType_Spec Action_spec = {
            Action_name.c_str(),
            sizeof(ActionObject),
            0,
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
            Action_slots
        };

        Action_Type = (PyTypeObject*)PyType_FromSpec(&Action_spec);
        if (PyModule_AddObjectRef(module, &Action_name[11], (PyObject*)Action_Type) < 0)
            return false;

        return true;
    }
};


#endif
