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

// From chpax.h
#define EI_PAX			14	// Index in e_ident[] where to read flags
#define HF_PAX_PAGEEXEC         1	// 0: Paging based non-exec pages
#define HF_PAX_EMUTRAMP         2	// 0: Emulate trampolines
#define HF_PAX_MPROTECT         4	// 0: Restrict mprotect()
#define HF_PAX_RANDMMAP         8	// 0: Randomize mmap() base
#define HF_PAX_RANDEXEC         16	// 1: Randomize ET_EXEC base
#define HF_PAX_SEGMEXEC         32	// 0: Segmentation based non-exec pages


#define PRINT(E,F,I) printf("%s: %s\n", #E, E & F ? ( I ? "enabled" : "disabled" ) : ( I ? "disabled" : "enabled" ) );
#define CASE(N,P) case P: printf("%d: %s\n", (int)N, #P); break


char *
parse_cmd_args( int c, char *v[], int *flag_ei_pax, int *flag_pt_pax_flags  )
{
	int i, oc;

	if((c != 2)&&(c != 3)&&(c != 4))
		error(EXIT_FAILURE, 0, "Usage: %s [-e] [-p] elffile", v[0]);

	*flag_ei_pax = 0;
	*flag_pt_pax_flags = 0;
	while((oc = getopt(c, v,":ep")) != -1)
		switch(oc)
		{
			case 'e':
				*flag_ei_pax = 1;
				break ;
			case 'p':
				*flag_pt_pax_flags = 1;
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
	int fd;
	int flag_ei_pax, flag_pt_pax_flags;
	int found_ei_pax, found_pt_pax_flags;
	char *f_name;
	size_t i, phnum;

	Elf *elf;
	GElf_Ehdr ehdr;
	GElf_Phdr phdr;

	f_name = parse_cmd_args( argc, argv, &flag_ei_pax, &flag_pt_pax_flags );

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
		error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));

	found_ei_pax = ((u_long) ehdr.e_ident[EI_PAX + 1] << 8) + (u_long) ehdr.e_ident[EI_PAX];

	printf("==== EI_PAX ====\n") ;
	PRINT(HF_PAX_PAGEEXEC, found_ei_pax, 0);
	PRINT(HF_PAX_EMUTRAMP, found_ei_pax, 1);
	PRINT(HF_PAX_MPROTECT, found_ei_pax, 0);
	PRINT(HF_PAX_RANDMMAP, found_ei_pax, 0);
	PRINT(HF_PAX_RANDEXEC, found_ei_pax, 1);
	PRINT(HF_PAX_SEGMEXEC, found_ei_pax, 0);
	printf("\n");

	if( flag_ei_pax )
	{
		printf("Disabling EI_PAX\n\n");
		ehdr.e_ident[EI_PAX]     = 0xFF;
		ehdr.e_ident[EI_PAX + 1] = 0xFF;
		if(!gelf_update_ehdr(elf, &ehdr))
			error(EXIT_FAILURE, 0, "gelf_update_ehdr(): %s", elf_errmsg(elf_errno()));
	}

	printf("==== PHRDs ====\n") ;
	found_pt_pax_flags = 0 ;
	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));

		switch(phdr.p_type)
		{
			CASE(i,PT_NULL);
			CASE(i,PT_LOAD);
			CASE(i,PT_DYNAMIC);
			CASE(i,PT_INTERP);
			CASE(i,PT_NOTE);
			CASE(i,PT_SHLIB);
			CASE(i,PT_PHDR);
			CASE(i,PT_TLS);
			CASE(i,PT_NUM);
			CASE(i,PT_LOOS);
			CASE(i,PT_GNU_EH_FRAME);
			CASE(i,PT_GNU_STACK);
			CASE(i,PT_GNU_RELRO);
			CASE(i,PT_PAX_FLAGS);
			CASE(i,PT_LOSUNW);
			//CASE(i,PT_SUNWBSS);
			CASE(i,PT_SUNWSTACK);
			CASE(i,PT_HISUNW);
			//CASE(i,PT_HIOS);
			CASE(i,PT_LOPROC);
			CASE(i,PT_HIPROC);
		}

		if((phdr.p_type == PT_PAX_FLAGS) && flag_pt_pax_flags )
		{
			found_pt_pax_flags = 1 ;
			phdr.p_type = PT_NULL;
			if(!gelf_update_phdr(elf, i, &phdr))
				error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
		}
	}

	if( found_pt_pax_flags )
		printf("Setting PT_PAX_FLAGS to PT_NULL\n\n");
	else
		printf("\n\n");

	elf_end(elf);
	close(fd);
}
