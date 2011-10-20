/*
	paxctl-ng.c: get/set pax flags on an ELF object
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <libgen.h>

#include <gelf.h>
#include <attr/xattr.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <config.h>


#define PAX_NAMESPACE	"user.pax"
#define BUF_SIZE	7

void
print_help(char *v)
{
	printf(
		"\n"
		"Package Name : " PACKAGE_STRING "\n"
		"Bug Reports  : " PACKAGE_BUGREPORT "\n"
		"Program Name : %s\n"
		"Description  : Get or set pax flags on an ELF object\n\n"
		"Usage        : %s -PpEeMmRrXxSsv ELF | -Zv ELF | -zv ELF | -h\n\n"
		"Options      : -P enable PAGEEXEC\t-p disable  PAGEEXEC\n"
		"             : -S enable SEGMEXEC\t-s disable  SEGMEXEC\n"
		"             : -M enable MPROTECT\t-m disable  MPROTECT\n"
		"             : -E enable EMUTRAMP\t-e disable  EMUTRAMP\n"
		"             : -R enable RANDMMAP\t-r disable  RANDMMAP\n"
		"             : -X enable RANDEXEC\t-x disable  RANDEXEC\n"
		"             : -Z most secure settings\t-z all default settings\n"
		"             : -v view the flags\n"
		"             : -h print out this help\n\n"
		"Note         :  If both enabling and disabling flags are set, the default - is used\n\n",
		basename(v),
		basename(v)
	);

	exit(EXIT_SUCCESS);
}


char *
parse_cmd_args(int c, char *v[], uint16_t *pax_flags, int *view_flags)
{
	int i, oc;
	int compat;

	compat = 0;

	*pax_flags = 0;
	*view_flags = 0;
	while((oc = getopt(c, v,":PpEeMmRrXxSsZzvh")) != -1)
		switch(oc)
		{
			case 'P':
				*pax_flags |= PF_PAGEEXEC;
				compat |= 1;
				break;
			case 'p':
				*pax_flags |= PF_NOPAGEEXEC;
				compat |= 1;
				break ;
			case 'S':
				*pax_flags |= PF_SEGMEXEC;
				compat |= 1;
				break;
			case 's':
				*pax_flags |= PF_NOSEGMEXEC;
				compat |= 1;
				break ;
			case 'M':
				*pax_flags |= PF_MPROTECT;
				compat |= 1;
				break;
			case 'm':
				*pax_flags |= PF_NOMPROTECT;
				compat |= 1;
				break ;
			case 'E':
				*pax_flags |= PF_EMUTRAMP;
				compat |= 1;
				break;
			case 'e':
				*pax_flags |= PF_NOEMUTRAMP;
				compat |= 1;
				break ;
			case 'R':
				*pax_flags |= PF_RANDMMAP;
				compat |= 1;
				break;
			case 'r':
				*pax_flags |= PF_NORANDMMAP;
				compat |= 1;
				break ;
			case 'X':
				*pax_flags |= PF_RANDEXEC;
				compat |= 1;
				break;
			case 'x':
				*pax_flags |= PF_NORANDEXEC;
				compat |= 1;
				break ;
			case 'Z':
				*pax_flags = PF_PAGEEXEC | PF_SEGMEXEC | PF_MPROTECT |
					PF_NOEMUTRAMP | PF_RANDMMAP | PF_NORANDEXEC;
				compat += 1;
				break ;
			case 'z':
				*pax_flags = PF_PAGEEXEC | PF_NOPAGEEXEC | PF_SEGMEXEC | PF_NOSEGMEXEC |
					PF_MPROTECT | PF_NOMPROTECT | PF_EMUTRAMP | PF_NOEMUTRAMP |
					PF_RANDMMAP | PF_NORANDMMAP | PF_RANDEXEC | PF_NORANDEXEC;
				compat += 1;
				break;
			case 'v':
				*view_flags = 1;
				compat |= 1;
				break;
			case 'h':
				print_help(v[0]);
				break;
			case '?':
			default:
				error(EXIT_FAILURE, 0, "option -%c is invalid: ignored.", optopt ) ;
		}

	if(compat != 1 || v[optind] == NULL)
		print_help(v[0]);

	return v[optind] ;
}


uint16_t
get_pt_flags(int fd)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	uint16_t pt_flags = UINT16_MAX;

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if((elf = elf_begin(fd, ELF_C_READ_MMAP, NULL)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(elf_errno()));

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));
		}

		if(phdr.p_type == PT_PAX_FLAGS)
			pt_flags = phdr.p_flags;
	}

	elf_end(elf);
	return pt_flags;
}


uint16_t
get_xt_flags(int fd)
{
	uint16_t xt_flags = UINT16_MAX;

	if(fgetxattr(fd, PAX_NAMESPACE, &xt_flags, sizeof(uint16_t)) == -1)
	{
		if(errno == ERANGE )
			printf("XT_PAX: corrupted\n");
		if( errno == ENOATTR)
			printf("XT_PAX: not present\n");
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


void
print_flags(int fd)
{
	uint16_t flags;
	char buf[BUF_SIZE];

	flags = get_pt_flags(fd);
	if( flags == UINT16_MAX )
		printf("PT_PAX: not found\n");
	else
	{
		memset(buf, 0, BUF_SIZE);
		bin2string(flags, buf);
		printf("PT_PAX: %s\n", buf);
	}

	flags = get_xt_flags(fd);
	if( flags == UINT16_MAX )
		printf("XT_PAX: not found\n");
	else
	{
		memset(buf, 0, BUF_SIZE);
		bin2string(flags, buf);
		printf("XT_PAX: %s\n", buf);
	}
}



uint16_t
new_flags(uint16_t flags, uint16_t pax_flags)
{
	//PAGEEXEC
	if(pax_flags & PF_PAGEEXEC)
	{
		flags |= PF_PAGEEXEC;
		flags &= ~PF_NOPAGEEXEC;
	}
	if(pax_flags & PF_NOPAGEEXEC)
	{
		flags &= ~PF_PAGEEXEC;
		flags |= PF_NOPAGEEXEC;
	}
	if((pax_flags & PF_PAGEEXEC) && (pax_flags & PF_NOPAGEEXEC))
	{
		flags &= ~PF_PAGEEXEC;
		flags &= ~PF_NOPAGEEXEC;
	}

	//SEGMEXEC
	if(pax_flags & PF_SEGMEXEC)
	{
		flags |= PF_SEGMEXEC;
		flags &= ~PF_NOSEGMEXEC;
	}
	if(pax_flags & PF_NOSEGMEXEC)
	{
		flags &= ~PF_SEGMEXEC;
		flags |= PF_NOSEGMEXEC;
	}
	if((pax_flags & PF_SEGMEXEC) && (pax_flags & PF_NOSEGMEXEC))
	{
		flags &= ~PF_SEGMEXEC;
		flags &= ~PF_NOSEGMEXEC;
	}

	//MPROTECT
	if(pax_flags & PF_MPROTECT)
	{
		flags |= PF_MPROTECT;
		flags &= ~PF_NOMPROTECT;
	}
	if(pax_flags & PF_NOMPROTECT)
	{
		flags &= ~PF_MPROTECT;
		flags |= PF_NOMPROTECT;
	}
	if((pax_flags & PF_MPROTECT) && (pax_flags & PF_NOMPROTECT))
	{
		flags &= ~PF_MPROTECT;
		flags &= ~PF_NOMPROTECT;
	}

	//EMUTRAMP
	if(pax_flags & PF_EMUTRAMP)
	{
		flags |= PF_EMUTRAMP;
		flags &= ~PF_NOEMUTRAMP;
	}
	if(pax_flags & PF_NOEMUTRAMP)
	{
		flags &= ~PF_EMUTRAMP;
		flags |= PF_NOEMUTRAMP;
	}
	if((pax_flags & PF_EMUTRAMP) && (pax_flags & PF_NOEMUTRAMP))
	{
		flags &= ~PF_EMUTRAMP;
		flags &= ~PF_NOEMUTRAMP;
	}

	//RANDMMAP
	if(pax_flags & PF_RANDMMAP)
	{
		flags |= PF_RANDMMAP;
		flags &= ~PF_NORANDMMAP;
	}
	if(pax_flags & PF_NORANDMMAP)
	{
		flags &= ~PF_RANDMMAP;
		flags |= PF_NORANDMMAP;
	}
	if((pax_flags & PF_RANDMMAP) && (pax_flags & PF_NORANDMMAP))
	{
		flags &= ~PF_RANDMMAP;
		flags &= ~PF_NORANDMMAP;
	}

	//RANDEXEC
	if(pax_flags & PF_RANDEXEC)
	{
		flags |= PF_RANDEXEC;
		flags &= ~PF_NORANDEXEC;
	}
	if(pax_flags & PF_NORANDEXEC)
	{
		flags &= ~PF_RANDEXEC;
		flags |= PF_NORANDEXEC;
	}
	if((pax_flags & PF_RANDEXEC) && (pax_flags & PF_NORANDEXEC))
	{
		flags &= ~PF_RANDEXEC;
		flags &= ~PF_NORANDEXEC;
	}

	return flags;
}


void
set_pt_flags(int fd, uint16_t pt_flags)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(elf_errno()));

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			phdr.p_flags = pt_flags;

			if(!gelf_update_phdr(elf, i, &phdr))
			{
				elf_end(elf);
				error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
			}
		}
	}

	elf_end(elf);
}


void
set_xt_flags(int fd, uint16_t xt_flags)
{
	if(fsetxattr(fd, PAX_NAMESPACE, &xt_flags, sizeof(uint16_t), 0) == -1)
	{
		if(errno == ENOSPC || errno == EDQUOT)
			printf("XT_PAX: insufficient space\n");
		if(errno == ENOTSUP)
			printf("XT_PAX: not supported\n");
	}
}


void
set_flags(int fd, uint16_t *pax_flags)
{
	uint16_t flags;

	flags = get_pt_flags(fd);
	if( flags == UINT16_MAX )
		flags = PF_PAGEEXEC | PF_SEGMEXEC | PF_MPROTECT |
			PF_NOEMUTRAMP | PF_RANDMMAP | PF_NORANDEXEC;
	flags = new_flags( flags, *pax_flags);
	set_pt_flags(fd, flags);

	flags = get_xt_flags(fd);
	if( flags == UINT16_MAX )
		flags = PF_PAGEEXEC | PF_SEGMEXEC | PF_MPROTECT |
			PF_NOEMUTRAMP | PF_RANDMMAP | PF_NORANDEXEC;
	flags = new_flags( flags, *pax_flags);
	set_xt_flags(fd, flags);
}


int
main( int argc, char *argv[])
{
	const char *f_name;
	int fd;
	uint16_t flags;
	int view_flags;

	f_name = parse_cmd_args(argc, argv, &flags, &view_flags);

	if((fd = open(f_name, O_RDWR)) < 0)
		error(EXIT_FAILURE, 0, "open() fail.");

	if(flags != 0)
		set_flags(fd, &flags);

	if(view_flags == 1)
		print_flags(fd);

	close(fd);
}
