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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <config.h>

#define HF_PAX_PAGEEXEC		1
#define HF_PAX_EMUTRAMP		2
#define HF_PAX_MPROTECT		4
#define HF_PAX_RANDMMAP		8
#define HF_PAX_RANDEXEC		16
#define HF_PAX_SEGMEXEC		32

#define EI_PAX			14   // Index to read the PaX flags into ELF header e_ident[] array


void
print_help(char *v)
{
	printf(
		"\n"
		"Package Name : " PACKAGE_STRING "\n"
		"Bug Reports  : " PACKAGE_BUGREPORT "\n"
		"Program Name : %s\n"
		"Description  : Get or set pax flags on an ELF object\n\n"
		"Usage        : %s [-PpEeMmRrXxSsv ELF] | [-Z ELF] | [-z ELF] | [-h]\n\n"
		"Options      : -P enable PAGEEXEC\t-p disable  PAGEEXEC\n"
		"             : -E enable EMUTRAMP\t-e disable  EMUTRAMP\n"
		"             : -M enable MPROTECT\t-m disable  MPROTECT\n"
		"             : -R enable RANDMMAP\t-r disable  RANDMMAP\n"
		"             : -X enable RANDEXEC\t-x disable  RANDEXEC\n"
		"             : -S enable SEGMEXEC\t-s disable  SEGMEXEC\n"
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
				*pax_flags = -1;
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
print_flags(Elf *elf)
{
	GElf_Ehdr ehdr;
	char ei_buf[BUF_SIZE];
	uint16_t ei_flags;

	GElf_Phdr phdr;
	char pt_buf[BUF_SIZE];
	char found_pt_pax;
	size_t i, phnum;


	memset(ei_buf, 0, BUF_SIZE);
	memset(pt_buf, 0, BUF_SIZE);

	if(gelf_getehdr(elf, &ehdr) != &ehdr)
		error(EXIT_FAILURE, 0, "gelf_getehdr(): %s", elf_errmsg(elf_errno()));

	ei_flags = ehdr.e_ident[EI_PAX] + (ehdr.e_ident[EI_PAX + 1] << 8);

  	ei_buf[0] = ei_flags & HF_PAX_PAGEEXEC ? 'p' : 'P';
	ei_buf[1] = ei_flags & HF_PAX_SEGMEXEC ? 's' : 'S';
	ei_buf[2] = ei_flags & HF_PAX_MPROTECT ? 'm' : 'M';
	ei_buf[3] = ei_flags & HF_PAX_EMUTRAMP ? 'E' : 'e';
	ei_buf[4] = ei_flags & HF_PAX_RANDMMAP ? 'r' : 'R';
	ei_buf[5] = ei_flags & HF_PAX_RANDEXEC ? 'X' : 'x';

	printf("EI_PAX: %s\n", ei_buf);

	found_pt_pax = 0;
	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));
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
		printf("PT_PAX: not found\n");

	//Only compare non default flags
	//if(strcmp(ei_buf, pt_buf))
	//	printf("EI_PAX != PT_PAX\n");
}


void
set_flags(Elf *elf)
{
	GElf_Ehdr ehdr;
	char ei_buf[BUF_SIZE];
	uint16_t ei_flags;

	GElf_Phdr phdr;
	char pt_buf[BUF_SIZE];
	char found_pt_pax;
	size_t i, phnum;


	memset(ei_buf, 0, BUF_SIZE);
	memset(pt_buf, 0, BUF_SIZE);

	/*
	if(!gelf_update_ehdr(e, &ehdr))
		error(EXIT_FAILURE, 0, "gelf_update_ehdr(): %s", elf_errmsg(elf_errno()));

	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));

		if((phdr.p_type == PT_PAX_FLAGS) && flag_pt_pax_flags )
		{
			printf("CONVERTED -> PT_NULL\n\n");
			phdr.p_type = PT_NULL;
			if(!gelf_update_phdr(elf, i, &phdr))
				error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
		}
	}
	*/
}


int
main( int argc, char *argv[])
{
	int fd;
	int pax_flags, view_flags;
	char *f_name;

	Elf *elf;

	f_name = parse_cmd_args(argc, argv, &pax_flags, &view_flags);

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if((fd = open(f_name, O_RDWR)) < 0)
		error(EXIT_FAILURE, 0, "open() fail.");

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(elf_errno()));

	if(elf_kind(elf) != ELF_K_ELF)
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");

	if(view_flags == 1)
		print_flags(elf);

	if(pax_flags != 0)
		set_flags(elf);

	elf_end(elf);
	close(fd);
}
