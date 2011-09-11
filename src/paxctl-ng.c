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
		"Package Name : " PACKAGE_STRING "\n"
		"Bug Reports  : " PACKAGE_BUGREPORT "\n"
		"Program Name : %s\n"
		"Description  : Get or set pax flags on an ELF object\n\n"
		"Usage        : %s {[-pPeEmMrRxXsSzZC]  ELFfile | [-h]}\n"
		"options      :     Print out pax flag information\n"
		"             : -p  Disable PAGEEXEC\t-P  Enable  PAGEEXEC\n"
		"             : -e  Disable EMUTRAMP\t-E  Enable  EMUTRAMP\n"
		"             : -m  Disable MPROTECT\t-M  Enable  MPROTECT\n"
		"             : -r  Disable RANDMMAP\t-R  Enable  RANDMMAP\n"
		"             : -x  Disable RANDEXEC\t-X  Enable  RANDEXEC\n"
		"             : -s  Disable SEGMEXEC\t-X  Enable  SEGMEXEC\n"
		"             : -z  Default least secure\t-Z Default most secure\n"
		"             : -v  view the flags\n"
		"             : -h  Print out this help\n",
		basename(v),
		basename(v)
	);

	exit(EXIT_SUCCESS);
}


char *
parse_cmd_args(int c, char *v[], int *pax_flags, int *view_flags)
{
	int i, oc;

	if((c != 2)&&(c != 3)&&(c != 4))
		error(EXIT_FAILURE, 0, "Usage: %s {[-pPeEmMrRxXsSzZv] ELFfile | [-h]}", v[0]);

	*pax_flags = 0;
	*view_flags = 0;
	while((oc = getopt(c, v,":pPeEmMrRxXsSzZvh")) != -1)
		switch(oc)
		{
			case 'p':
				break ;
			case 'P':
				break;
			case 'e':
				break ;
			case 'E':
				break;
			case 'm':
				break ;
			case 'M':
				break;
			case 'r':
				break ;
			case 'R':
				break;
			case 'x':
				break ;
			case 'X':
				break;
			case 's':
				break ;
			case 'S':
				break;
			case 'z':
				break ;
			case 'Z':
				break;
			case 'v':
				*view_flags = 1;
				break;
			case 'h':
				print_help(v[0]);
				break;
			case '?':
			default:
				error(EXIT_FAILURE, 0, "option -%c is invalid: ignored.", optopt ) ;
		}

	return v[optind] ;
}


#define BUF_SIZE 7
void
print_flags(Elf *e, GElf_Ehdr *eh)
{
	char ei_buf[BUF_SIZE];
	char pt_buf[BUF_SIZE];
	uint16_t ei_flags;

	char found_pt_pax;
	size_t i, phnum;
	GElf_Phdr phdr;

	memset(ei_buf, 0, BUF_SIZE);
	memset(pt_buf, 0, BUF_SIZE);

	ei_flags = eh->e_ident[EI_PAX] + (eh->e_ident[EI_PAX + 1] << 8);

  	ei_buf[0] = ei_flags & HF_PAX_PAGEEXEC ? 'p' : 'P';
	ei_buf[1] = ei_flags & HF_PAX_SEGMEXEC ? 's' : 'S';
	ei_buf[2] = ei_flags & HF_PAX_MPROTECT ? 'm' : 'M';
	ei_buf[3] = ei_flags & HF_PAX_EMUTRAMP ? 'E' : 'e';
	ei_buf[4] = ei_flags & HF_PAX_RANDMMAP ? 'r' : 'R';
	ei_buf[5] = ei_flags & HF_PAX_RANDEXEC ? 'X' : 'x';

	printf("EI_PAX: %s\n", ei_buf);

	found_pt_pax = 0;
	elf_getphdrnum(e, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(e, i, &phdr) != &phdr)
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

	if(strcmp(ei_buf, pt_buf))
		printf("EI_PAX != PT_PAX\n");
}


int
main( int argc, char *argv[])
{
	int fd;
	int pax_flags, view_flags;
	char *f_name;

	Elf *elf;
	GElf_Ehdr ehdr;

	f_name = parse_cmd_args(argc, argv, &pax_flags, &view_flags);

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if((fd = open(f_name, O_RDWR)) < 0)
		error(EXIT_FAILURE, 0, "open() fail.");

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(elf_errno()));

	if(elf_kind(elf) != ELF_K_ELF)
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");

	// get ehdr
	if(gelf_getehdr(elf, &ehdr) != &ehdr)
		error(EXIT_FAILURE, 0, "gelf_getehdr(): %s", elf_errmsg(elf_errno()));

	if(view_flags == 1)
		print_flags(elf, &ehdr);

	/*
	if(!gelf_update_ehdr(elf, &ehdr))
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
	printf("\n\n");
	*/

	elf_end(elf);
	close(fd);
}
