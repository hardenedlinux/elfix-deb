/*
	paxctl-xattr.c: get/set pax flags on xattr for an ELF object
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
#include <errno.h>
#include <libgen.h>

#include <gelf.h>
#include <attr/xattr.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <config.h>

#define PAX_NAMESPACE "user.pax"

void
print_help(char *v)
{
	printf(
		"\n"
		"Package Name : " PACKAGE_STRING "\n"
		"Bug Reports  : " PACKAGE_BUGREPORT "\n"
		"Program Name : %s\n"
		"Description  : Get or set xattr pax flags on an ELF object\n\n"
		"Usage        : %s [-PpEeMmRrXxSsv ELF] | [-Z ELF] | [-z ELF] | [-h]\n\n"
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
parse_cmd_args(int c, char *v[], int *pax_flags, int *view_flags)
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
					PF_NOEMUTRAMP | PF_RANDMMAP | PF_RANDEXEC;
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


#define BUF_SIZE 7
void
print_flags(int fd)
{
	char xt_buf[BUF_SIZE];
	uint16_t xt_flags;

	static ssize_t vsize = 1024;
	static char *value = NULL;
	ssize_t i, vret = -1;

	memset(xt_buf, 0, BUF_SIZE);
	value  = malloc(vsize);

	//If at first we don't succeed, grow buffer size
	while(((vret = fgetxattr(fd, PAX_NAMESPACE, value, vsize)) == -1) && (errno == ERANGE))
	{
		vsize <<= 1;
		value = realloc(value, vsize);
	}

	if(errno == ENOATTR)
	{
		printf("XT_PAX: not found or permission denied\n");
		return;
	}

	if(errno == ENOTSUP)
	{
		printf("XT_PAX: extended attribute not supported\n");
		return;
	}

	xt_flags = (uint16_t)value[0];
	xt_flags = xt_flags << 8 + value[1];

	xt_buf[0] = xt_flags & PF_PAGEEXEC ? 'P' :
		xt_flags & PF_NOPAGEEXEC ? 'p' : '-' ;

	xt_buf[1] = xt_flags & PF_SEGMEXEC   ? 'S' : 
		xt_flags & PF_NOSEGMEXEC ? 's' : '-';

	xt_buf[2] = xt_flags & PF_MPROTECT   ? 'M' :
		xt_flags & PF_NOMPROTECT ? 'm' : '-';

	xt_buf[3] = xt_flags & PF_EMUTRAMP   ? 'E' :
		xt_flags & PF_NOEMUTRAMP ? 'e' : '-';

	xt_buf[4] = xt_flags & PF_RANDMMAP   ? 'R' :
		xt_flags & PF_NORANDMMAP ? 'r' : '-';

	xt_buf[5] = xt_flags & PF_RANDEXEC   ? 'X' :
		xt_flags & PF_NORANDEXEC ? 'x' : '-';

	printf("XT_PAX: %s\n", xt_buf);
}


void
set_flags(int fd, int *pax_flags)
{
	uint16_t xt_flags;

	//int fsetxattr(int fd, const char *name, const void *value, size_t size, int flags);

	/*
	if( / DOME xattrs is supported / )
	{
		//PAGEEXEC
		if(*pax_flags & PF_PAGEEXEC)
		{
			phdr.p_flags |= PF_PAGEEXEC;
			phdr.p_flags &= ~PF_NOPAGEEXEC;
		}
		if(*pax_flags & PF_NOPAGEEXEC)
		{
			phdr.p_flags &= ~PF_PAGEEXEC;
			phdr.p_flags |= PF_NOPAGEEXEC;
		}
		if((*pax_flags & PF_PAGEEXEC) && (*pax_flags & PF_NOPAGEEXEC))
		{
			phdr.p_flags &= ~PF_PAGEEXEC;
			phdr.p_flags &= ~PF_NOPAGEEXEC;
		}

		//SEGMEXEC
		if(*pax_flags & PF_SEGMEXEC)
		{
			phdr.p_flags |= PF_SEGMEXEC;
			phdr.p_flags &= ~PF_NOSEGMEXEC;
		}
		if(*pax_flags & PF_NOSEGMEXEC)
		{
			phdr.p_flags &= ~PF_SEGMEXEC;
			phdr.p_flags |= PF_NOSEGMEXEC;
		}
		if((*pax_flags & PF_SEGMEXEC) && (*pax_flags & PF_NOSEGMEXEC))
		{
			phdr.p_flags &= ~PF_SEGMEXEC;
			phdr.p_flags &= ~PF_NOSEGMEXEC;
		}

		//MPROTECT
		if(*pax_flags & PF_MPROTECT)
		{
			phdr.p_flags |= PF_MPROTECT;
			phdr.p_flags &= ~PF_NOMPROTECT;
		}
		if(*pax_flags & PF_NOMPROTECT)
		{
			phdr.p_flags &= ~PF_MPROTECT;
			phdr.p_flags |= PF_NOMPROTECT;
		}
		if((*pax_flags & PF_MPROTECT) && (*pax_flags & PF_NOMPROTECT))
		{
			phdr.p_flags &= ~PF_MPROTECT;
			phdr.p_flags &= ~PF_NOMPROTECT;
		}

		//EMUTRAMP
		if(*pax_flags & PF_EMUTRAMP)
		{
			phdr.p_flags |= PF_EMUTRAMP;
			phdr.p_flags &= ~PF_NOEMUTRAMP;
		}
		if(*pax_flags & PF_NOEMUTRAMP)
		{
			phdr.p_flags &= ~PF_EMUTRAMP;
			phdr.p_flags |= PF_NOEMUTRAMP;
		}
		if((*pax_flags & PF_EMUTRAMP) && (*pax_flags & PF_NOEMUTRAMP))
		{
			phdr.p_flags &= ~PF_EMUTRAMP;
			phdr.p_flags &= ~PF_NOEMUTRAMP;
		}

		//RANDMMAP
		if(*pax_flags & PF_RANDMMAP)
		{
			phdr.p_flags |= PF_RANDMMAP;
			phdr.p_flags &= ~PF_NORANDMMAP;
		}
		if(*pax_flags & PF_NORANDMMAP)
		{
			phdr.p_flags &= ~PF_RANDMMAP;
			phdr.p_flags |= PF_NORANDMMAP;
		}
		if((*pax_flags & PF_RANDMMAP) && (*pax_flags & PF_NORANDMMAP))
		{
			phdr.p_flags &= ~PF_RANDMMAP;
			phdr.p_flags &= ~PF_NORANDMMAP;
		}

		//RANDEXEC
		if(*pax_flags & PF_RANDEXEC)
		{
			phdr.p_flags |= PF_RANDEXEC;
			phdr.p_flags &= ~PF_NORANDEXEC;
		}
		if(*pax_flags & PF_NORANDEXEC)
		{
			phdr.p_flags &= ~PF_RANDEXEC;
			phdr.p_flags |= PF_NORANDEXEC;
		}
		if((*pax_flags & PF_RANDEXEC) && (*pax_flags & PF_NORANDEXEC))
		{
			phdr.p_flags &= ~PF_RANDEXEC;
			phdr.p_flags &= ~PF_NORANDEXEC;
		}

		/ update xattr /
	}
	else
		printf("XT_PAX: not found\n");
	*/
}


int
main( int argc, char *argv[])
{
	int fd;
	int pax_flags, view_flags;
	char *f_name;

	f_name = parse_cmd_args(argc, argv, &pax_flags, &view_flags);

	if((fd = open(f_name, O_RDWR)) < 0)
		error(EXIT_FAILURE, 0, "open() fail.");

	if(pax_flags != 0)
		set_flags(fd, &pax_flags);

	if(view_flags == 1)
		print_flags(fd);

	close(fd);
}
