/*
	paxmodule.c: python module to get/set pax flags on an ELF object
	Copyright (C) 2011  Anthony G. Basile

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Python.h>

#include <string.h>

#include <gelf.h>

#ifdef XATTR
#include <attr/xattr.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef XATTR
#define PAX_NAMESPACE	"user.pax.flags"
#endif

#define FLAGS_SIZE	6


static PyObject * pax_getflags(PyObject *, PyObject *);
static PyObject * pax_setbinflags(PyObject *, PyObject *);
static PyObject * pax_setstrflags(PyObject *, PyObject *);

static PyMethodDef PaxMethods[] = {
	{"getflags",  pax_getflags, METH_VARARGS, "Get the pax flags as a string."},
	{"setbinflags",  pax_setbinflags, METH_VARARGS, "Set the pax flags using binary."},
	{"setstrflags",  pax_setstrflags, METH_VARARGS, "Set the pax flags using string."},
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
get_pt_flags(int fd)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	uint16_t pt_flags = UINT16_MAX;

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		PyErr_SetString(PaxError, "get_pt_flags: library out of date");
		return pt_flags;
	}

	if((elf = elf_begin(fd, ELF_C_READ_MMAP, NULL)) == NULL)
	{
		PyErr_SetString(PaxError, "get_pt_flags: elf_begin() failed");
		return pt_flags;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		PyErr_SetString(PaxError, "get_pt_flags: elf_kind() failed: this is not an elf file.");
		return pt_flags;
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			PyErr_SetString(PaxError, "get_pt_flags: gelf_getphdr() failed: could not get phdr.");
			return pt_flags;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
			pt_flags = phdr.p_flags;
	}

	elf_end(elf);

	return pt_flags;
}


#ifdef XATTR
uint16_t
string2bin(char *buf)
{
	uint16_t flags = 0;

	if( buf[0] == 'P' )
		flags |= PF_PAGEEXEC;
	else if( buf[0] == 'p' )
		flags |= PF_NOPAGEEXEC;

	if( buf[1] == 'S' )
		flags |= PF_SEGMEXEC;
	else if( buf[1] == 's' )
		flags |= PF_NOSEGMEXEC;

	if( buf[2] == 'M' )
		flags |= PF_MPROTECT;
	else if( buf[2] == 'm' )
		flags |= PF_NOMPROTECT;

	if( buf[3] == 'E' )
		flags |= PF_EMUTRAMP;
	else if( buf[3] == 'e' )
		flags |= PF_NOEMUTRAMP;

	if( buf[4] == 'R' )
		flags |= PF_RANDMMAP;
	else if( buf[4] == 'r' )
		flags |= PF_NORANDMMAP;

	return flags;
}


uint16_t
get_xt_flags(int fd)
{
	char buf[FLAGS_SIZE];
	uint16_t xt_flags = UINT16_MAX;

	memset(buf, 0, FLAGS_SIZE);

	if(fgetxattr(fd, PAX_NAMESPACE, buf, FLAGS_SIZE) != -1)
		xt_flags = string2bin(buf);

	return xt_flags;
}
#endif


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
}


static PyObject *
pax_getflags(PyObject *self, PyObject *args)
{
	const char *f_name;
	int fd;
	uint16_t flags;
	char buf[FLAGS_SIZE];

	memset(buf, 0, FLAGS_SIZE);

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

#ifdef XATTR
	flags = get_xt_flags(fd);
	if( flags != UINT16_MAX )
	{
		memset(buf, 0, FLAGS_SIZE);
		bin2string(flags, buf);
	}
	else
	{
#endif
		flags = get_pt_flags(fd);
		if( flags != UINT16_MAX )
		{
			memset(buf, 0, FLAGS_SIZE);
			bin2string(flags, buf);
		}
#ifdef XATTR
	}
#endif

	close(fd);

	return Py_BuildValue("si", buf, flags);
}


void
set_pt_flags(int fd, uint16_t pt_flags)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		PyErr_SetString(PaxError, "set_pt_flags: library out of date");
		return;
	}

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
	{
		PyErr_SetString(PaxError, "set_pt_flags: elf_begin() failed");
		return;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		PyErr_SetString(PaxError, "set_pt_flags: elf_kind() failed: this is not an elf file.");
		return;
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			PyErr_SetString(PaxError, "set_pt_flags: gelf_getphdr() failed");
			return;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			phdr.p_flags = pt_flags;

			if(!gelf_update_phdr(elf, i, &phdr))
			{
				elf_end(elf);
				PyErr_SetString(PaxError, "set_pt_flags: gelf_update_phdr() failed");
				return;
			}
		}
	}

	elf_end(elf);
}


#ifdef XATTR
void
set_xt_flags(int fd, uint16_t xt_flags)
{
	char buf[FLAGS_SIZE];

	memset(buf, 0, FLAGS_SIZE);
	bin2string(xt_flags, buf);
	fsetxattr(fd, PAX_NAMESPACE, buf, strlen(buf), XATTR_REPLACE);
}
#endif


static PyObject *
pax_setbinflags(PyObject *self, PyObject *args)
{
	const char *f_name;
	int fd, iflags;
	uint16_t flags;

	if (!PyArg_ParseTuple(args, "si", &f_name, &iflags))
	{
		PyErr_SetString(PaxError, "pax_setbinflags: PyArg_ParseTuple failed");
		return NULL;
	}

	if((fd = open(f_name, O_RDWR)) < 0)
	{
		PyErr_SetString(PaxError, "pax_setbinflags: open() failed");
		return NULL;
	}

	flags = (uint16_t) iflags;

	set_pt_flags(fd, flags);

#ifdef XATTR
	set_xt_flags(fd, flags);
#endif

	close(fd);

	return Py_BuildValue("");
}

static PyObject *
pax_setstrflags(PyObject *self, PyObject *args)
{
	char *f_name, *sflags;
	int fd;
	uint16_t flags;

	if (!PyArg_ParseTuple(args, "ss", &f_name, &sflags))
	{
		PyErr_SetString(PaxError, "pax_setbinflags: PyArg_ParseTuple failed");
		return NULL;
	}

	if((fd = open(f_name, O_RDWR)) < 0)
	{
		PyErr_SetString(PaxError, "pax_setbinflags: open() failed");
		return NULL;
	}

	flags = string2bin(sflags);

	set_pt_flags(fd, flags);

#ifdef XATTR
	set_xt_flags(fd, flags);
#endif

	close(fd);

	return Py_BuildValue("");
}
