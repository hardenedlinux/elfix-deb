#include <Python.h>

#include <string.h>

#include <gelf.h>
#include <attr/xattr.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE	7	//Buffer size for holding human readable flags
#define PAX_NAMESPACE	"user.pax"


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


uint16_t
read_pt_flags(int fd)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	uint16_t pt_flags;
	char found_pt_pax;

	pt_flags = UINT16_MAX;

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		close(fd);
		PyErr_SetString(PaxError, "pax_getflags: library out of date");
		return pt_flags;
	}

	if((elf = elf_begin(fd, ELF_C_READ_MMAP, NULL)) == NULL)
	{
		close(fd);
		PyErr_SetString(PaxError, "pax_getflags: elf_begin() failed");
		return pt_flags;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		close(fd);
		PyErr_SetString(PaxError, "pax_getflags: elf_kind() failed: this is not an elf file.");
		return pt_flags;
	}

	found_pt_pax = 0;
	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			found_pt_pax = 1;
			pt_flags = phdr.p_flags;
		}
	}

	if(!found_pt_pax)
		printf("PT_PAX: not found\n");

	return pt_flags;
}


uint16_t
read_xt_flags(int fd)
{
	uint16_t xt_flags;

	xt_flags = UINT16_MAX;

	if(fgetxattr(fd, PAX_NAMESPACE, &xt_flags, sizeof(uint16_t)) == -1)
	{
		// ERANGE  = xattrs supported, PAX_NAMESPACE present, but wrong size
		// ENOATTR = xattrs supported, PAX_NAMESPACE not present
		if(errno == ERANGE || errno == ENOATTR)
		{
			printf("XT_PAX: not present or corrupted\n");
			/*
			printf("XT_PAX: creating/repairing flags\n");
			xt_flags = PF_NOEMUTRAMP | PF_NORANDEXEC;
			if(fsetxattr(fd, PAX_NAMESPACE, &xt_flags, sizeof(uint16_t), 0) == -1)
			{
				xt_flags = UINT16_MAX;
				if(errno == ENOSPC || errno == EDQUOT)
					printf("XT_PAX: access error\n");
				if(errno == ENOTSUP)
					printf("XT_PAX: not supported\n");
			}
			*/
		}

		// ENOTSUP = xattrs not supported
		if(errno == ENOTSUP)
			printf("XT_PAX: not supported\n");
	}

	return xt_flags;
}


void
bin2string(uint16_t flags, char *buf)
{
	buf[0] = flags & PF_PAGEEXEC ? 'P' :
		flags & PF_NOPAGEEXEC ? 'p' : '-' ;

	buf[1] = flags & PF_SEGMEXEC   ? 'S' :
		flags & PF_NOSEGMEXEC ? 's' : '-';

	buf[2] = flags & PF_MPROTECT   ? 'M' :
		flags & PF_NOMPROTECT ? 'm' : '-';

	buf[3] = flags & PF_EMUTRAMP   ? 'E' :
		flags & PF_NOEMUTRAMP ? 'e' : '-';

	buf[4] = flags & PF_RANDMMAP   ? 'R' :
		flags & PF_NORANDMMAP ? 'r' : '-';

	buf[5] = flags & PF_RANDEXEC   ? 'X' :
		flags & PF_NORANDEXEC ? 'x' : '-';
}


static PyObject *
pax_getflags(PyObject *self, PyObject *args)
{
	const char *f_name;
	int fd;

        uint16_t flags;
        char buf[BUF_SIZE];

	memset(buf, 0, BUF_SIZE);

	if (!PyArg_ParseTuple(args, "s", &f_name))
	{
		PyErr_SetString(PaxError, "pax_getflags: PyArg_ParseTuple failed");
		return NULL;
	}

	if((fd = open(f_name, O_RDONLY)) < 0)
	{
		PyErr_SetString(PaxError, "pax_getflags: open() failed");
		return NULL;
	}

        flags = read_xt_flags(fd);
        if( flags != UINT16_MAX )
        {
                memset(buf, 0, BUF_SIZE);
                bin2string(flags, buf);
        }
	else
	{
       		flags = read_pt_flags(fd);
	        if( flags != UINT16_MAX )
		{
			memset(buf, 0, BUF_SIZE);
			bin2string(flags, buf);
		}
	}

	return Py_BuildValue("si", buf, flags);
}


static PyObject *
pax_setflags(PyObject *self, PyObject *args)
{
	const char *f_name;
	uint16_t pax_flags;
	int fd;

	Elf *elf;
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
