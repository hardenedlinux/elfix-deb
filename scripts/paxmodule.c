#include <Python.h>

#include <string.h>

#include <gelf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Gentoo bug #387459

#define HF_PAX_PAGEEXEC		1
#define HF_PAX_EMUTRAMP		2
#define HF_PAX_MPROTECT		4
#define HF_PAX_RANDMMAP		8
#define HF_PAX_RANDEXEC		16
#define HF_PAX_SEGMEXEC		32

#define EI_PAX			14	// Index to read the PaX flags into ELF header e_ident[] array
*/

#define BUF_SIZE		7	//Buffer for holding human readable flags


static PyObject * pax_getflags(PyObject *, PyObject *);
static PyObject * pax_setflags(PyObject *, PyObject *);

static PyMethodDef PaxMethods[] = {
	{"getflags",  pax_getflags, METH_VARARGS, "Get the pax flags."},
	{"setflags",  pax_setflags, METH_VARARGS, "Set the pax flags."},
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
	int fd;
	Elf *elf;

	char pax_buf[BUF_SIZE];
	uint16_t pax_flags;

	/* Gentoo bug #387459
	GElf_Ehdr ehdr; 
	*/
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
	pax_flags = 0;

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
			pax_flags = phdr.p_flags;

			pax_buf[0] = pax_flags & PF_PAGEEXEC ? 'P' :
				pax_flags & PF_NOPAGEEXEC ? 'p' : '-' ;

			pax_buf[1] = pax_flags & PF_SEGMEXEC   ? 'S' : 
				pax_flags & PF_NOSEGMEXEC ? 's' : '-';

			pax_buf[2] = pax_flags & PF_MPROTECT   ? 'M' :
				pax_flags & PF_NOMPROTECT ? 'm' : '-';

			pax_buf[3] = pax_flags & PF_EMUTRAMP   ? 'E' :
				pax_flags & PF_NOEMUTRAMP ? 'e' : '-';

			pax_buf[4] = pax_flags & PF_RANDMMAP   ? 'R' :
				pax_flags & PF_NORANDMMAP ? 'r' : '-';

			pax_buf[5] = pax_flags & PF_RANDEXEC   ? 'X' :
				pax_flags & PF_NORANDEXEC ? 'x' : '-';
		}
	}

	if(!found_pt_pax)
	{
		//Set to the strictest possible
	}

	/* Gentoo bug #387459
	if(!found_pt_pax)
	{
		if(gelf_getehdr(elf, &ehdr) != &ehdr)
		{
			elf_end(elf);
			close(fd);
			PyErr_SetString(PaxError, "pax_getflags: gelf_getehdr() failed");
			return NULL;
		}

		pax_flags = ehdr.e_ident[EI_PAX] + (ehdr.e_ident[EI_PAX + 1] << 8);

  		pax_buf[0] = pax_flags & HF_PAX_PAGEEXEC ? 'p' : 'P';
		pax_buf[1] = pax_flags & HF_PAX_SEGMEXEC ? 's' : 'S';
		pax_buf[2] = pax_flags & HF_PAX_MPROTECT ? 'm' : 'M';
		pax_buf[3] = pax_flags & HF_PAX_EMUTRAMP ? 'E' : 'e';
		pax_buf[4] = pax_flags & HF_PAX_RANDMMAP ? 'r' : 'R';
		pax_buf[5] = pax_flags & HF_PAX_RANDEXEC ? 'X' : 'x';
	}
	*/

	elf_end(elf);
	close(fd);

	return Py_BuildValue("si", pax_buf, pax_flags);
}


static PyObject *
pax_setflags(PyObject *self, PyObject *args)
{
	const char *f_name;
	uint16_t pax_flags;
	int fd;

	Elf *elf;

	/* Gentoo bug #387459
	GElf_Ehdr ehdr;
	uint16_t ei_flags;
	*/

	GElf_Phdr phdr;
	size_t i, phnum;

	if (!PyArg_ParseTuple(args, "si", &f_name, &pax_flags))
	{
		PyErr_SetString(PaxError, "pax_setflags: PyArg_ParseTuple failed");
		return NULL;
	}

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		PyErr_SetString(PaxError, "pax_setflags: library out of date");
		return NULL;
	}

	if((fd = open(f_name, O_RDWR)) < 0)
	{
		PyErr_SetString(PaxError, "pax_setflags: open() failed");
		return NULL;
	}

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
	{
		close(fd);
		PyErr_SetString(PaxError, "pax_setflags: elf_begin() failed");
		return NULL;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		close(fd);
		PyErr_SetString(PaxError, "pax_setflags: elf_kind() failed: this is not an elf file.");
		return NULL;
	}

	/* Gentoo bug #387459

	if(gelf_getehdr(elf, &ehdr) != &ehdr)
	{
		elf_end(elf);
		close(fd);
		PyErr_SetString(PaxError, "pax_setflags: gelf_getehdr() failed");
		return NULL;
	}

	ei_flags = ehdr.e_ident[EI_PAX] + (ehdr.e_ident[EI_PAX + 1] << 8);

	ei_flags &= ~HF_PAX_PAGEEXEC;
	ei_flags &= ~HF_PAX_SEGMEXEC;
	ei_flags &= ~HF_PAX_MPROTECT;
	ei_flags |= HF_PAX_EMUTRAMP;
	ei_flags &= ~HF_PAX_RANDMMAP;
	ei_flags |= HF_PAX_RANDEXEC;

	//PAGEEXEC
	if(pax_flags & PF_PAGEEXEC)
		ei_flags &= ~HF_PAX_PAGEEXEC;
	if(pax_flags & PF_NOPAGEEXEC)
		ei_flags |= HF_PAX_PAGEEXEC;

	//SEGMEXEC
	if(pax_flags & PF_SEGMEXEC)
		ei_flags &= ~HF_PAX_SEGMEXEC;
	if(pax_flags & PF_NOSEGMEXEC)
		ei_flags |= HF_PAX_SEGMEXEC;

	//MPROTECT
	if(pax_flags & PF_MPROTECT)
		ei_flags &= ~HF_PAX_MPROTECT;
	if(pax_flags & PF_NOMPROTECT)
		ei_flags |= HF_PAX_MPROTECT;

	//EMUTRAMP
	if(pax_flags & PF_EMUTRAMP)
		ei_flags |= HF_PAX_EMUTRAMP;
	if(pax_flags & PF_NOEMUTRAMP)
		ei_flags &= ~HF_PAX_EMUTRAMP;

	//RANDMMAP
	if(pax_flags & PF_RANDMMAP)
		ei_flags &= ~HF_PAX_RANDMMAP;
	if(pax_flags & PF_NORANDMMAP)
		ei_flags |= HF_PAX_RANDMMAP;

	//RANDEXEC
	if(pax_flags & PF_RANDEXEC)
		ei_flags |= HF_PAX_RANDEXEC;
	if(pax_flags & PF_NORANDEXEC)
		ei_flags &= ~HF_PAX_RANDEXEC;

	ehdr.e_ident[EI_PAX] = (uint8_t)ei_flags  ;
	ehdr.e_ident[EI_PAX + 1] = (uint8_t)(ei_flags >> 8) ;

	if(!gelf_update_ehdr(elf, &ehdr))
	{
		elf_end(elf);
		close(fd);
		PyErr_SetString(PaxError, "pax_setflags: gelf_update_ehdr() failed");
		return NULL;
	}
	*/


	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			close(fd);
			PyErr_SetString(PaxError, "pax_setflags: gelf_getphdr() failed");
			return NULL;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			phdr.p_flags = pax_flags;

			if(!gelf_update_phdr(elf, i, &phdr))
			{
				elf_end(elf);
				close(fd);
				PyErr_SetString(PaxError, "pax_setflags: gelf_update_phdr() failed");
				return NULL;
			}
		}
	}

	elf_end(elf);
	close(fd);

	return Py_BuildValue("");
}
