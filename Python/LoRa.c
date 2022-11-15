#include <Python.h>
#include <structmember.h>
#include <LoRaDraginoDriver/LoRaCommunicator.h>

typedef struct
{
    PyObject_HEAD
    LoRaCommunicator* LoRaCommunicator;
    Sf SpreadingFactor;
    uint32_t Frequency;
    pthread_t ListenThread;
    PyObject *OnReceive;
} LoRaCom;

// Constructor
static PyObject *LoRaCom_new(PyTypeObject *type, PyObject* args, PyObject* kwds)
{
    LoRaCom *self;
    self = (LoRaCom *)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->LoRaCommunicator = NULL;
        self->SpreadingFactor = SF7;
        self->Frequency = 868000000;
        self->ListenThread = 0;
    }
    return (PyObject *)self;
}

static int LoRaCom_init(LoRaCom *self, PyObject *args, PyObject *kwds)
{
    uint32_t frequency = 868000000;
    Sf spreadingFactor = SF7;

    static char *kwlist[] = {"frequency", "spreading_factor", "mode", NULL };

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "i|ii", kwlist, &frequency, &spreadingFactor))
        return -1;

    self->LoRaCommunicator = InitLoRaCommunicator(frequency, spreadingFactor, LORA_ROLE_RECEIVER);
    self->Frequency = frequency;
    self->SpreadingFactor = spreadingFactor;
    return 0;
}

// Destructor
static void LoRaCom_dealloc(LoRaCom *self)
{
    StopLoRaListen(self->LoRaCommunicator);

    if(self->ListenThread > 0)
        pthread_join(self->ListenThread, NULL);

    Py_XDECREF(self->OnReceive);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

// Members
static PyMemberDef LoRaCom_members[] =
    {
            {"frequency", T_INT, offsetof(LoRaCom, Frequency), 0, "frequency"},
            {"spreading_factor", T_INT, offsetof(LoRaCom, SpreadingFactor), 0, "spreading_factor"},
            { "on_receive", T_OBJECT_EX, offsetof(LoRaCom, OnReceive), 0, "on_receive" },
            {NULL}
    };

// Methods
static PyObject *LoRaCom_send(LoRaCom *self, PyObject *args)
{
    char* message;
    if(!PyArg_ParseTuple(args, "s", &message))
        return NULL;

    LoRaSend(self->LoRaCommunicator, message);
    Py_RETURN_NONE;
}

static void OnReceiveCallback(char* message, int length, void* pyObject)
{
    LoRaCom *self = (LoRaCom*)pyObject;
    PyObject* pyMessage = PyUnicode_FromStringAndSize(message, length);
    PyObject* pyArgs = PyTuple_New(1);
    PyTuple_SetItem(pyArgs, 0, pyMessage);

    PyObject_CallObject(self->OnReceive, pyArgs);
    Py_DECREF(pyArgs);
    Py_DECREF(pyMessage);
}

static PyObject *LoRaCom_set_on_receive(LoRaCom *self, PyObject *args)
{
    PyObject *callback;
    if(!PyArg_ParseTuple(args, "O", &callback))
        return NULL;

    if(!PyCallable_Check(callback))
    {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    Py_XINCREF(callback);
    Py_XDECREF(self->OnReceive);
    self->OnReceive = callback;
    SetOnReceive(self->LoRaCommunicator, OnReceiveCallback, self);

    Py_RETURN_NONE;
}

static PyObject *LoRaCom_stop(LoRaCom *self, PyObject *args)
{
    StopLoRaListen(self->LoRaCommunicator);

    if(self->ListenThread > 0)
        pthread_join(self->ListenThread, NULL);

    Py_RETURN_NONE;
}

static PyObject *LoRaCom_listen(LoRaCom *self, PyObject *args)
{
    char run_thread = 1;
    // optional boolean argument
    if(!PyArg_ParseTuple(args, "|b", &run_thread))
        run_thread = 1;

    if(run_thread)
    {
        if(self->ListenThread > 0)
            pthread_join(self->ListenThread, NULL);

        self->ListenThread = LoRaListenThread(self->LoRaCommunicator);
    }
    else
    {
        if(self->ListenThread > 0)
            pthread_join(self->ListenThread, NULL);

        self->ListenThread = 0;
        LoRaListen(self->LoRaCommunicator);
    }

    Py_RETURN_NONE;
}

static PyObject *LoRaCom_str(LoRaCom *self)
{
    return PyUnicode_FromFormat("LoRaCom: Frequency: %d, SpreadingFactor: %d", self->Frequency, self->SpreadingFactor);
}

static PyMethodDef LoRaCom_methods[] =
    {
        {"send", (PyCFunction)LoRaCom_send, METH_VARARGS, "Send a message"},
        {"set_on_receive", (PyCFunction)LoRaCom_set_on_receive, METH_VARARGS, "Set the callback for when a message is received"},
        {"listen", (PyCFunction)LoRaCom_listen, METH_VARARGS, "Listen for messages"},
        {"stop", (PyCFunction)LoRaCom_stop, METH_NOARGS, "Stop listening for messages"},
        { "__str__", (PyCFunction)LoRaCom_str, METH_NOARGS, "String representation" },
        {NULL}
    };

// Type
static PyTypeObject lora_LoRaComType =
{
        PyVarObject_HEAD_INIT(NULL, 0)
        "lorapy.LoRaCom",             /* tp_name */
        sizeof(LoRaCom),            /* tp_basicsize */
        0,                          /* tp_itemsize */
        LoRaCom_dealloc,            /* tp_dealloc */
        0,                          /* tp_print */
        0,                          /* tp_getattr */
        0,                          /* tp_setattr */
        0,                          /* tp_reserved */
        0,                          /* tp_repr */
        0,                          /* tp_as_number */
        0,                          /* tp_as_sequence */
        0,                          /* tp_as_mapping */
        0,                          /* tp_hash  */
        0,                          /* tp_call */
        0,                          /* tp_str */
        0,                          /* tp_getattro */
        0,                          /* tp_setattro */
        0,                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,         /* tp_flags */
        "Fraction objects",         /* tp_doc */
        0,                          /* tp_traverse */
        0,                          /* tp_clear */
        0,                          /* tp_richcompare */
        0,                          /* tp_weaklistoffset */
        0,                          /* tp_iter */
        0,                          /* tp_iternext */
        LoRaCom_methods,            /* tp_methods */
        LoRaCom_members,            /* tp_members */
        0,                          /* tp_getset */
        0,                          /* tp_base */
        0,                          /* tp_dict */
        0,                          /* tp_descr_get */
        0,                          /* tp_descr_set */
        0,                          /* tp_dictoffset */
        LoRaCom_init,               /* tp_init */
        0,                          /* tp_alloc */
        LoRaCom_new
};

// Module
static PyModuleDef lora_module =
    {
        PyModuleDef_HEAD_INIT,
        "lorapy",
        "LoRa module",
        -1,
        NULL, NULL, NULL, NULL, NULL
    };

PyMODINIT_FUNC PyInit_lorapy(void)
{
    PyObject *m;
    if (PyType_Ready(&lora_LoRaComType) < 0)
    {
        return NULL;
    }

    m = PyModule_Create(&lora_module);
    if (m == NULL)
    {
        return NULL;
    }

    Py_INCREF(&lora_LoRaComType);
    PyModule_AddObject(m, "LoRaCom", (PyObject *)&lora_LoRaComType);
    return m;
}
