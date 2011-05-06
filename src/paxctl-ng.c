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
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include <gelf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <config.h>

#define EI_PAX 14 // Index in e_ident[] where to read flags - from chpax.h

#define PRINT(E,F,I) printf("%s:\t%s\n", #E, E & F ? ( I ? "enabled" : "disabled" ) : ( I ? "disabled" : "enabled" ) );
#define CASE(N,P) case P: printf("%d: %s\n", (int)N, #P); break


void
print_help(char *v)
{
        printf(
                "Package Name : " PACKAGE_STRING "\n"
                "Bug Reports  : " PACKAGE_BUGREPORT "\n"
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
		"             : -C  Created PT_PAX_FLAGS program header\n"
                "             : -h  Print out this help\n",
                v
        );

        exit(EXIT_SUCCESS);
}


char *
parse_cmd_args( int c, char *v[], int *pax_flags, int *create_flag )
{
	int i, oc;

	if((c != 2)&&(c != 3)&&(c != 4))
		error(EXIT_FAILURE, 0, "Usage: %s {[-pPeEmMrRxXsSzZC] ELFfile | [-h]}", v[0]);

	*pax_flags = 0;
	*create_flag = 0;
	while((oc = getopt(c, v,":pPeEmMrRxXsSzZCh")) != -1)
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
			case 'C':
				*create_flag = 1;
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
no_pt_pax_flags(Elf *e)
{
	size_t i, phnum;
	GElf_Phdr phdr;

	elf_getphdrnum(e, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(e, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));
		if(phdr.p_type == PT_PAX_FLAGS)
			return 0;
	}
	return 1;
}


int
create_pt_pax_flags(Elf *e)
{
	size_t i, phnum;
	GElf_Phdr phdr;

	elf_getphdrnum(e, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(e, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(elf_errno()));
		if(phdr.p_type == PT_NULL)
		{
			phdr.p_type = PT_PAX_FLAGS;
			phdr.p_flags = PF_NOEMUTRAMP|PF_NORANDEXEC;
			if(!gelf_update_phdr(e, i, &phdr))
				error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
			return 1;
		}
	}


	/*
	if( !(phdr = gelf_newphdr(Elf *e, size_t phnum)) )
	{
		phdr.p_type = PT_PAX_FLAGS;
		//phdr.p_offset
		//phdr.p_vaddr
		//phdr.p_paddr
		//phdr.p_filesz
		//phdr.p_memsz
		phdr.p_flags = PF_NOEMUTRAMP|PF_NORANDEXEC;
		//phdr.p_align

		if(!gelf_update_phdr(e, i, &phdr))
			error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
		return 1;
	}
		error(EXIT_FAILURE, 0, "gelf_newphdr(): %s", elf_errmsg(elf_errno()));
	*/

}


int
main( int argc, char *argv[])
{
	int fd;
	int pax_flags, create_flag;
	char *f_name;

	Elf *elf;
	GElf_Ehdr ehdr;

	f_name = parse_cmd_args(argc, argv, &pax_flags, &create_flag);

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if(create_flag)
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





	if(create_flag)
	{
		//To be safe, let's make sure EI_PAX flags are zero-ed for most secure legacy
		if(gelf_getehdr(elf, &ehdr) != &ehdr)
			error(EXIT_FAILURE, 0, "gelf_getehdr(): %s", elf_errmsg(elf_errno()));

		ehdr.e_ident[EI_PAX] = 0;
		ehdr.e_ident[EI_PAX + 1] = 0;

		if(!gelf_update_ehdr(elf, &ehdr))
			error(EXIT_FAILURE, 0, "gelf_update_ehdr(): %s", elf_errmsg(elf_errno()));

		if(no_pt_pax_flags(elf))
		{
			printf("PT_PAX_FLAGS phdr not found: creating one\n");
			if(create_pt_pax_flags(elf))
			{
				printf("PT_PAX_FLAGS phdr create: succeeded\n");
			}
			else
				error(EXIT_FAILURE, 0, "PT_PAX_FLAGS phdr create: failed");
		}
		else
			error(EXIT_FAILURE, 0, "PT_PAX_FLAGS phdr found: nothing to do");
	}	



	/*
	printf("==== EI_PAX ====\n") ;
	PRINT(HF_PAX_PAGEEXEC, found_ei_pax, 0);
	PRINT(HF_PAX_EMUTRAMP, found_ei_pax, 1);
	PRINT(HF_PAX_MPROTECT, found_ei_pax, 0);
	PRINT(HF_PAX_RANDMMAP, found_ei_pax, 0);
	PRINT(HF_PAX_RANDEXEC, found_ei_pax, 1);
	PRINT(HF_PAX_SEGMEXEC, found_ei_pax, 0);
	printf("\n");


	printf("==== PHRDs ====\n") ;
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

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			PRINT(PF_PAGEEXEC, phdr.p_flags, 1);
			PRINT(PF_NOPAGEEXEC, phdr.p_flags, 1);
			PRINT(PF_SEGMEXEC, phdr.p_flags, 1);
			PRINT(PF_NOSEGMEXEC, phdr.p_flags, 1);
			PRINT(PF_MPROTECT, phdr.p_flags, 1);
			PRINT(PF_NOMPROTECT, phdr.p_flags, 1);
			PRINT(PF_RANDEXEC, phdr.p_flags, 1);
			PRINT(PF_NORANDEXEC, phdr.p_flags, 1);
			PRINT(PF_EMUTRAMP, phdr.p_flags, 1);
			PRINT(PF_NOEMUTRAMP, phdr.p_flags, 1);
			PRINT(PF_RANDMMAP, phdr.p_flags, 1);
			PRINT(PF_NORANDMMAP, phdr.p_flags, 1);
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
	*/

	elf_end(elf);
	close(fd);
}
