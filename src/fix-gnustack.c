/*
	fix-gnustack.c: check and optionally remove exec flag on Elf GNU_STACK
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



char *
parse_cmd_args( int c, char *v[], int *flagv  )
{
	int i, oc;

	if((c != 2)&&(c != 3))
		error(EXIT_FAILURE, 0, "Usage: %s [-f] elffile", v[0]);

	*flagv = 0 ;
	while((oc = getopt(c, v,":f")) != -1)
		switch(oc)
		{
			case 'f':
				*flagv = 1 ;
				break ;
			case '?':
			default:
				error(EXIT_FAILURE, 0, "option -%c is invalid: ignored.", optopt ) ;
		}

	return v[optind] ;
}



int
main( int argc, char *argv[])
{
	int fd, flagv;
	char *f_name;
	size_t i, phnum;

	Elf *elf;
	GElf_Ehdr ehdr;
	GElf_Phdr phdr;

	f_name = parse_cmd_args( argc, argv, &flagv );

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if(flagv)
	{
		if((fd = open(f_name, O_RDWR)) < 0)
			error(EXIT_FAILURE, 0, "open() fail.");
		if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
			error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(-1));
	}
	else
	{
		if((fd = open(f_name, O_RDONLY)) < 0)
			error(EXIT_FAILURE, 0, "open() fail.");
		if((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
			error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(-1));
	}

	if(elf_kind(elf) != ELF_K_ELF)
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");

	if(gelf_getehdr(elf,&ehdr) == NULL)
		error(EXIT_FAILURE, 0, "gelf_getehdr() fail: %s", elf_errmsg(-1));

	elf_getphdrnum(elf, &phnum);
	for(i=0; i<phnum; ++i)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
			error(EXIT_FAILURE, 0, "gelf_getphdr(): %s", elf_errmsg(-1));

		if(phdr.p_type == PT_GNU_STACK)
		{
			printf("GNU_STACK: ");
			if( phdr.p_flags & PF_R ) printf("R");
			if( phdr.p_flags & PF_W ) printf("W");
			if( phdr.p_flags & PF_X ) printf("X");
			printf("\n");

			if(flagv && (phdr.p_flags & PF_W) && (phdr.p_flags & PF_X))
			{
				printf("W&X FOUND: flipping X flag ...\n");
				phdr.p_flags ^= PF_X;
				if(!gelf_update_phdr(elf, i, &phdr))
					error(EXIT_FAILURE, 0, "gelf_update_phdr(): %s", elf_errmsg(-1));
			}
		}
	}

	elf_end(elf);
	close(fd);
}
