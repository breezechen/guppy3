/* Support for multiple Python interpreters */

/*
static char hp_interpreter_doc[] =

"interpreter(command:string [,locals:dict] ) -> int\n"
"\n"
"Create a new interpreter structure with a new thread and return the\n"
"thread identity number.\n"
"\n"
"The arguments are:\n"
"\n"
"    command   A command that will be exec'd in the new environment.\n"
"    locals    Local variables passed to the command when exec'd.\n"
"\n"
"\n"
"The new interpreter and thread is started in a new environment.  This\n"
"environment consists of a new '__main__' module, with the optional\n"
"locals dict as local variables.\n"
"\n"
"The interpreter() function will return after the new thread structure\n"
"has been created. The command will execute sooner or later.  The\n"
"thread will terminate, and the interpreter structure be deallocated,\n"
"when the command has been executed, and dependent threads have\n"
"terminated.";


static char hp_set_async_exc_doc[] =
"set_async_exc(thread_id:integer, exception:object)\n"
"\n"
"Set an exception to be raised asynchronously in a thread.\n"
;

#include "pythread.h"
#include "eval.h"

struct bootstate {
    PyObject *cmd;
    PyObject *locals;
};

static void
t_bootstrap(void *boot_raw)
{
    struct bootstate *boot = (struct bootstate *) boot_raw;
    PyThreadState *tstate;
    PyObject *v;
    int res;
    char *str;

    PyEval_AcquireLock();
    tstate = Py_NewInterpreter();
    if (!tstate) {
        PyThread_exit_thread();
        return;
    }
    if (PyString_AsStringAndSize(boot->cmd, &str, NULL))
        res = -1;
    else {
        PyObject *mainmod = PyImport_ImportModule("__main__");
        PyObject *maindict = PyModule_GetDict(mainmod);
        v = PyRun_String(str, Py_file_input, maindict, boot->locals);
        if (!v)
            res = -1;
        else {
            Py_DECREF(v);
            res = 0;
        }
        Py_DECREF(mainmod);
    }
    if (res == -1) {
        if (PyErr_ExceptionMatches(PyExc_SystemExit))
            PyErr_Clear();
        else {
            PyObject *file;
            PySys_WriteStderr(
                    "Unhandled exception in thread started by ");
            file = PySys_GetObject("stderr");
            if (file)
                    PyFile_WriteObject(boot->cmd, file, 0);
            else
                    PyObject_Print(boot->cmd, stderr, 0);
            PySys_WriteStderr("\n");
            PyErr_PrintEx(0);
        }
	}
    Py_DECREF(boot->cmd);
    Py_XDECREF(boot->locals);
    PyMem_DEL(boot_raw);

    if (!(tstate->interp->tstate_head == tstate && tstate->next == NULL)) {
        PyObject *timemod = PyImport_ImportModule("time");
        PyObject *sleep = 0;
        PyObject *time;
        if (timemod) {
            sleep = PyObject_GetAttrString(timemod, "sleep");
            Py_DECREF(timemod);
        }
        time = PyFloat_FromDouble(0.05);
        while (!(tstate->interp->tstate_head == tstate && tstate->next == NULL)) {
            PyObject *res;
            res = PyObject_CallFunction(sleep, "O", time);
            if (res) {
                Py_DECREF(res);
            }
        }
        Py_DECREF(time);
        Py_DECREF(sleep);
    }
    Py_EndInterpreter(tstate);
    PyEval_ReleaseLock();
    PyThread_exit_thread();
}

static PyObject *
hp_interpreter(PyObject *self, PyObject *args)
{
    PyObject *cmd = NULL;
    PyObject *locals = NULL;

    struct bootstate *boot;
    long ident;

    if (!PyArg_ParseTuple(args, "O|O!:interpreter",
                    &cmd,
                    &PyDict_Type, &locals))
            return NULL;


    boot = PyMem_NEW(struct bootstate, 1);
    if (boot == NULL)
            return PyErr_NoMemory();
    boot->cmd = cmd;
    boot->locals = locals;
    Py_INCREF(cmd);
    Py_XINCREF(locals);

    PyEval_InitThreads(); // Start the interpreter's thread-awareness
    ident = PyThread_start_new_thread(t_bootstrap, (void*) boot);
    if (ident == -1) {
        PyErr_SetString(PyExc_ValueError, "can't start new thread\n");
        Py_DECREF(cmd);
        Py_XDECREF(locals);
        PyMem_DEL(boot);
        return NULL;
    }
    return PyInt_FromLong(ident);
}

#define ZAP(x) {                     \
    PyObject *tmp = (PyObject *)(x); \
    (x) = NULL;                      \
    Py_XDECREF(tmp);                 \
}

*//* As PyThreadState_SetAsyncExc in pystate.c,
   but searches all interpreters.
   Thus it finds any task, and it should not be of
   any disadvantage, what I can think of..
*//*


int
NyThreadState_SetAsyncExc(long id, PyObject *exc) {
    PyInterpreterState *interp;
    int count = 0;
    for (interp = PyInterpreterState_Head(); interp;
         interp = PyInterpreterState_Next(interp)) {
        PyThreadState *p;
        for (p = interp->tstate_head; p != NULL; p = p->next) {
            if (THREAD_ID(p) != id)
                continue;
            ZAP(p->async_exc);
            Py_XINCREF(exc);
            p->async_exc = exc;
            count += 1;
        }
    }
    return count;
}



static PyObject *
hp_set_async_exc(PyObject *self, PyObject *args)
{
    PyObject *idobj, *exc;
    long id, r;
    if (!PyArg_ParseTuple(args, "OO",
                          &idobj, &exc))
        return NULL;
    if ((id = PyInt_AsLong(idobj)) == -1 && PyErr_Occurred())
        return NULL;
    if ((r = NyThreadState_SetAsyncExc(id, exc)) > 1) {
        NyThreadState_SetAsyncExc(id, NULL);
        r = -1;
    }
    return PyLong_FromLong(r);
}
*/
