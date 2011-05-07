/*
	mangle-paxflags.c: check and optionally remove EI_PAX and/or PT_PAX_FLAGS
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
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include <gelf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <config.h>

// From chpax.h
#define EI_PAX			14	// Index in e_ident[] where to read flags
#define HF_PAX_PAGEEXEC         1	// 0: Paging based non-exec pages
#define HF_PAX_EMUTRAMP         2	// 0: Emulate trampolines
#define HF_PAX_MPROTECT         4	// 0: Restrict mprotect()
#define HF_PAX_RANDMMAP         8	// 0: Randomize mmap() base
#define HF_PAX_RANDEXEC         16	// 1: Randomize ET_EXEC base
#define HF_PAX_SEGMEXEC         32	// 0: Segmentation based non-exec pages

#define PRINT(E,F,I)		printf("%s:\t%s\n", #E, E&F? (I? "enabled" : "disabled") : (I? "disabled" : "enabled"));
#define SPRINT(E,F,A,B)		printf("%c", E&F? A : B);
#define CPRINT(N,P)		case P: printf("%d: %s\n", (int)N, #P); break
#define FPRINT(N,D,F,A,B)	printf("%c", N&F? (D&F? '*' : B) : (D&F? A : '-'))


void
print_help(char *v)
{
        printf(
                "Package Name : " PACKAGE_STRING "\n"
                "Bug Reports  : " PACKAGE_BUGREPORT "\n"
                "Description  : Check for, or conditionally remove, executable flag from PT_GNU_STACK\n\n"
                "Usage        : %s {[-e] [-p] ELFfile | -h}\n"
                "options      :     Print out EI_PAX and PT_PAX_FLAGS information\n"
                "             : -e  Set all EI_PAX flags to least secure setting, pEmrXs\n"
                "             : -p  Remove PT_PAX_FLAGS program header\n"
		"             : -v  Verbose expanation of flags (rather than short list)\n"
                "             : -h  Print out this help\n",
                v
        );

        exit(EXIT_SUCCESS);
}


char *
parse_cmd_args(int c, char *v[], int *flag_ei_pax, int *flag_pt_pax_flags, int *verbose)
{
	int i, oc;

	if((c != 2)&&(c != 3)&&(c != 4))
		error(EXIT_FAILURE, 0, "Usage: %s {[-e] [-p] [-v] ELFfile | [-h]}", v[0]);

	*flag_ei_pax = 0;
	*flag_pt_pax_flags = 0;
	*verbose = 0;

	while((oc = getopt(c, v,":epvh")) != -1)
		switch(oc)
		{
			case 'e':
				*flag_ei_pax = 1;
				break ;
			case 'p':
				*flag_pt_pax_flags = 1;
				break;
			case 'v':
				*verbose = 1;
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


int
main( int argc, char *argv[])
{
	int fd, flag_ei_pax, flag_pt_pax_flags, verbose, found_ei_pax;
	char *f_name;
	size_t i, phnum;

	Elf *elf;
	GElf_Ehdr ehdr;
	GElf_Phdr phdr;

	f_name = parse_cmd_args(argc, argv, &flag_ei_pax, &flag_pt_pax_flags, &verbose);

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if( flag_ei_pax || flag_pt_pax_flags )
	{
		if((fd = open(f_name, O_RDWR)) < 0)
			error(EXIT_FAILURE, 0, "open() fail.");
		if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
			error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(elf_errno()));
	}
	else
	{
		if((fd = open(f_name, O_RDONLY)) < 0)
			error(EXIT_FAILURE, 0, "open() fail.");
		if((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
			error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(elf_errno()));
	}

	if(elf_kind(elf) != ELF_K_ELF)
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");

	if(gelf_getehdr(elf,&ehdr) != &ehdr)
		error(EXIT_FAILURE, 0, "gelf_getehdr(): %s", elf_errmsg(elf_errno()));

	found_ei_pax = ((u_long) ehdr.e_ident[EI_PAX + 1] << 8) + (u_long) ehdr.e_ident[EI_PAX];

	printf("==== EI_PAX ====\n") ;
	if(verbose)
	{
		PRINT(HF_PAX_PAGEEXEC, found_ei_pax, 0);
		PRINT(HF_PAX_EMUTRAMP, found_ei_pax, 1);
		PRINT(HF_PAX_MPROTECT, found_ei_pax, 0);
		PRINT(HF_PAX_RANDMMAP, found_ei_pax, 0);
		PRINT(HF_PAX_RANDEXEC, found_ei_pax, 1);
		PRINT(HF_PAX_SEGMEXEC, found_ei_pax, 0);
		printf("\n");
	}
	else
	{
		SPRINT(HF_PAX_PAGEEXEC, found_ei_pax, 'p', 'P');
		SPRINT(HF_PAX_EMUTRAMP, found_ei_pax, 'E', 'e');
		SPRINT(HF_PAX_MPROTECT, found_ei_pax, 'm', 'M');
		SPRINT(HF_PAX_RANDMMAP, found_ei_pax, 'r', 'R');
		SPRINT(HF_PAX_RANDEXEC, found_ei_pax, 'X', 'x');
		SPRINT(HF_PAX_SEGMEXEC, found_ei_pax, 's', 'S');
		printf("\n\n");
	}

	if( flag_ei_pax )
	{
		printf("Disabling EI_PAX\n\n");
		ehdr.e_ident[EI_PAX]     = 0xFF;
		ehdr.e_ident[EI_PAX + 1] = 0xFF;
		if(!gelf_update_ehdr(elf, &ehdr))
			error(EXIT_FAILURE, 0, "gelf_update_ehdr(): %s", elf_errmsg(elf_errno()));
	}

	printf("==== PHRDs ====\n") ;
	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));

		if(verbose)
		{
			switch(phdr.p_type)
			{
				CPRINT(i,PT_NULL);
				CPRINT(i,PT_LOAD);
				CPRINT(i,PT_DYNAMIC);
				CPRINT(i,PT_INTERP);
				CPRINT(i,PT_NOTE);
				CPRINT(i,PT_SHLIB);
				CPRINT(i,PT_PHDR);
				CPRINT(i,PT_TLS);
				CPRINT(i,PT_NUM);
				CPRINT(i,PT_LOOS);
				CPRINT(i,PT_GNU_EH_FRAME);
				CPRINT(i,PT_GNU_STACK);
				CPRINT(i,PT_GNU_RELRO);
				CPRINT(i,PT_PAX_FLAGS);
				CPRINT(i,PT_LOSUNW);
				//CPRINT(i,PT_SUNWBSS);
				CPRINT(i,PT_SUNWSTACK);
				CPRINT(i,PT_HISUNW);
				//CPRINT(i,PT_HIOS);
				CPRINT(i,PT_LOPROC);
				CPRINT(i,PT_HIPROC);
			}
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			if(verbose)
			{
				PRINT(PF_PAGEEXEC,   phdr.p_flags, 1);
				PRINT(PF_NOPAGEEXEC, phdr.p_flags, 1);
				PRINT(PF_SEGMEXEC,   phdr.p_flags, 1);
				PRINT(PF_NOSEGMEXEC, phdr.p_flags, 1);
				PRINT(PF_MPROTECT,   phdr.p_flags, 1);
				PRINT(PF_NOMPROTECT, phdr.p_flags, 1);
				PRINT(PF_RANDEXEC,   phdr.p_flags, 1);
				PRINT(PF_NORANDEXEC, phdr.p_flags, 1);
				PRINT(PF_EMUTRAMP,   phdr.p_flags, 1);
				PRINT(PF_NOEMUTRAMP, phdr.p_flags, 1);
				PRINT(PF_RANDMMAP,   phdr.p_flags, 1);
				PRINT(PF_NORANDMMAP, phdr.p_flags, 1);
			}
			else
			{
				printf("%d: PT_PAX_FLAGS\n", (int)i);
				FPRINT(PF_PAGEEXEC, PF_NOPAGEEXEC, phdr.p_flags, 'p', 'P');
				FPRINT(PF_EMUTRAMP, PF_NOEMUTRAMP, phdr.p_flags, 'e', 'E');
				FPRINT(PF_MPROTECT, PF_NOMPROTECT, phdr.p_flags, 'm', 'M');
				FPRINT(PF_RANDMMAP, PF_NORANDMMAP, phdr.p_flags, 'r', 'R');
				FPRINT(PF_RANDEXEC, PF_NORANDEXEC, phdr.p_flags, 'x', 'X');
				FPRINT(PF_SEGMEXEC, PF_NOSEGMEXEC, phdr.p_flags, 's', 'S');
			}
		}

		if((phdr.p_type == PT_PAX_FLAGS) && flag_pt_pax_flags )
		{
			printf("CONVERTED -> PT_NULL\n\n");
			phdr.p_type = PT_NULL;
			if(!gelf_update_phdr(elf, i, &phdr))
				error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
		}
	}
	printf("\n\n");

	elf_end(elf);
	close(fd);
}
