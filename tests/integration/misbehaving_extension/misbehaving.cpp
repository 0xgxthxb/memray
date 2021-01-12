#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>
#include <pthread.h>
#include <malloc.h>

pthread_t thread;

void*
worker(void* args)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* callback = (PyObject*) args;
    PyObject* result = PyObject_CallFunction(callback, NULL);
    assert(result != NULL);
    Py_DECREF(result);
    PyGILState_Release(gstate);
    return NULL;
}

void start_threads(void* args)
{
    int ret = pthread_create(&thread, NULL, &worker, args);
    assert(0 == ret);
}

void join_threads()
{
    pthread_join(thread, NULL);
}

PyObject*
call_fn(PyObject*, PyObject* args)
{
    PyObject* callback;
    if (!PyArg_ParseTuple(args,"O", &callback))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to parse arguments");
        Py_RETURN_NONE;
    }
    Py_BEGIN_ALLOW_THREADS
    start_threads(callback);
    join_threads();
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
        {"call_fn", call_fn, METH_VARARGS, "Call Python function on a thread"},
        {NULL, NULL, 0, NULL},
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {PyModuleDef_HEAD_INIT, "misbehaving", "", -1, methods};

PyMODINIT_FUNC
PyInit_misbehaving(void)
{
    return PyModule_Create(&moduledef);
}
#else
PyMODINIT_FUNC
initmisbehaving(void)
{
    Py_InitModule("misbehaving", methods);
}
#endif

