#include <Python.h>

static PyObject * pax_getflags(PyObject *, PyObject *);

static PyMethodDef PaxMethods[] = {
	{"getflags",  pax_getflags, METH_VARARGS, "Get the pax flags."},
	{NULL, NULL, 0, NULL}
};

static PyObject *PaxError;

PyMODINIT_FUNC
initpax(void)
{
	PyObject *m;

	m = Py_InitModule("pax", PaxMethods);
	if (m == NULL)
		return;

	PaxError = PyErr_NewException("pax.error", NULL, NULL);
	Py_INCREF(PaxError);
	PyModule_AddObject(m, "error", PaxError);
}


static PyObject *
pax_getflags(PyObject *self, PyObject *args)
{
	const char *value;
	int sts;

	if (!PyArg_ParseTuple(args, "s", &value))
		return NULL;

	printf("%s\n", value);

	sts = 1;

	if (sts < 0)
	{
		PyErr_SetString(PaxError, "pax_getflags failed");
		return NULL;
	}

	return Py_BuildValue("i", sts);
}
