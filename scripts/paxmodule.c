#include <Python.h>

#include <stdio.h> //remove when you remove printf
#include <string.h>

#include <gelf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define HF_PAX_PAGEEXEC		1
#define HF_PAX_EMUTRAMP		2
#define HF_PAX_MPROTECT		4
#define HF_PAX_RANDMMAP		8
#define HF_PAX_RANDEXEC		16
#define HF_PAX_SEGMEXEC		32

#define EI_PAX			14	// Index to read the PaX flags into ELF header e_ident[] array

#define BUF_SIZE		7	//Buffer for holding human readable flags


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
	const char *f_name;
	int fd, sts;
	Elf *elf;

	GElf_Ehdr ehdr;
	char ei_buf[BUF_SIZE];
	uint16_t ei_flags;

	GElf_Phdr phdr;
	char pt_buf[BUF_SIZE];
	char found_pt_pax;
	size_t i, phnum;

	memset(ei_buf, 0, BUF_SIZE);
	memset(pt_buf, 0, BUF_SIZE);

	if (!PyArg_ParseTuple(args, "s", &f_name))
	{
		PyErr_SetString(PaxError, "pax_getflags: PyArg_ParseTuple failed");
		return NULL;
	}

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		PyErr_SetString(PaxError, "pax_getflags: library out of date");
		return NULL;
	}

	if((fd = open(f_name, O_RDONLY)) < 0)
	{
		PyErr_SetString(PaxError, "pax_getflags: open() failed");
		return NULL;
	}

	if((elf = elf_begin(fd, ELF_C_READ_MMAP, NULL)) == NULL)
	{
		PyErr_SetString(PaxError, "pax_getflags: elf_begin() failed");
		return NULL;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		PyErr_SetString(PaxError, "pax_getflags: elf_kind() failed: this is not an elf file.");
		return NULL;
	}


	found_pt_pax = 0;
	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			PyErr_SetString(PaxError, "pax_getflags: gelf_getphdr() failed");
			return NULL;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			found_pt_pax = 1;

			pt_buf[0] = phdr.p_flags & PF_PAGEEXEC ? 'P' :
				phdr.p_flags & PF_NOPAGEEXEC ? 'p' : '-' ;

			pt_buf[1] = phdr.p_flags & PF_SEGMEXEC   ? 'S' : 
				phdr.p_flags & PF_NOSEGMEXEC ? 's' : '-';

			pt_buf[2] = phdr.p_flags & PF_MPROTECT   ? 'M' :
				phdr.p_flags & PF_NOMPROTECT ? 'm' : '-';

			pt_buf[3] = phdr.p_flags & PF_EMUTRAMP   ? 'E' :
				phdr.p_flags & PF_NOEMUTRAMP ? 'e' : '-';

			pt_buf[4] = phdr.p_flags & PF_RANDMMAP   ? 'R' :
				phdr.p_flags & PF_NORANDMMAP ? 'r' : '-';

			pt_buf[5] = phdr.p_flags & PF_RANDEXEC   ? 'X' :
				phdr.p_flags & PF_NORANDEXEC ? 'x' : '-';
		}
	}

	if(found_pt_pax)
		printf("PT_PAX: %s\n", pt_buf);
	else
	{
		if(gelf_getehdr(elf, &ehdr) != &ehdr)
		{
			PyErr_SetString(PaxError, "pax_getflags: gelf_getehdr() failed");
			return NULL;
		}

		ei_flags = ehdr.e_ident[EI_PAX] + (ehdr.e_ident[EI_PAX + 1] << 8);

  		ei_buf[0] = ei_flags & HF_PAX_PAGEEXEC ? 'p' : 'P';
		ei_buf[1] = ei_flags & HF_PAX_SEGMEXEC ? 's' : 'S';
		ei_buf[2] = ei_flags & HF_PAX_MPROTECT ? 'm' : 'M';
		ei_buf[3] = ei_flags & HF_PAX_EMUTRAMP ? 'E' : 'e';
		ei_buf[4] = ei_flags & HF_PAX_RANDMMAP ? 'r' : 'R';
		ei_buf[5] = ei_flags & HF_PAX_RANDEXEC ? 'X' : 'x';

		printf("EI_PAX: %s\n", ei_buf);
	}

	elf_end(elf);
	close(fd);

	return Py_BuildValue("i", sts);
}
