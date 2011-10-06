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

	char pax_buf[BUF_SIZE];

	GElf_Ehdr ehdr;
	uint16_t ei_flags;

	GElf_Phdr phdr;
	char found_pt_pax;
	size_t i, phnum;

	memset(pax_buf, 0, BUF_SIZE);

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
		close(fd);
		PyErr_SetString(PaxError, "pax_getflags: elf_begin() failed");
		return NULL;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		close(fd);
		PyErr_SetString(PaxError, "pax_getflags: elf_kind() failed: this is not an elf file.");
		return NULL;
	}


	found_pt_pax = 0;
	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			close(fd);
			PyErr_SetString(PaxError, "pax_getflags: gelf_getphdr() failed");
			return NULL;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			found_pt_pax = 1;

			pax_buf[0] = phdr.p_flags & PF_PAGEEXEC ? 'P' :
				phdr.p_flags & PF_NOPAGEEXEC ? 'p' : '-' ;

			pax_buf[1] = phdr.p_flags & PF_SEGMEXEC   ? 'S' : 
				phdr.p_flags & PF_NOSEGMEXEC ? 's' : '-';

			pax_buf[2] = phdr.p_flags & PF_MPROTECT   ? 'M' :
				phdr.p_flags & PF_NOMPROTECT ? 'm' : '-';

			pax_buf[3] = phdr.p_flags & PF_EMUTRAMP   ? 'E' :
				phdr.p_flags & PF_NOEMUTRAMP ? 'e' : '-';

			pax_buf[4] = phdr.p_flags & PF_RANDMMAP   ? 'R' :
				phdr.p_flags & PF_NORANDMMAP ? 'r' : '-';

			pax_buf[5] = phdr.p_flags & PF_RANDEXEC   ? 'X' :
				phdr.p_flags & PF_NORANDEXEC ? 'x' : '-';
		}
	}

	if(!found_pt_pax)
	{
		if(gelf_getehdr(elf, &ehdr) != &ehdr)
		{
			elf_end(elf);
			close(fd);
			PyErr_SetString(PaxError, "pax_getflags: gelf_getehdr() failed");
			return NULL;
		}

		ei_flags = ehdr.e_ident[EI_PAX] + (ehdr.e_ident[EI_PAX + 1] << 8);

  		pax_buf[0] = ei_flags & HF_PAX_PAGEEXEC ? 'p' : 'P';
		pax_buf[1] = ei_flags & HF_PAX_SEGMEXEC ? 's' : 'S';
		pax_buf[2] = ei_flags & HF_PAX_MPROTECT ? 'm' : 'M';
		pax_buf[3] = ei_flags & HF_PAX_EMUTRAMP ? 'E' : 'e';
		pax_buf[4] = ei_flags & HF_PAX_RANDMMAP ? 'r' : 'R';
		pax_buf[5] = ei_flags & HF_PAX_RANDEXEC ? 'X' : 'x';
	}

	elf_end(elf);
	close(fd);

	return Py_BuildValue("s", pax_buf);
}
