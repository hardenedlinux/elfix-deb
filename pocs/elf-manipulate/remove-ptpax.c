/*
	remove-ptpax.c: this file is part of the elfix package
	Copyright (C) 2013  Anthony G. Basile

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gelf.h>

#ifndef PT_PAX_FLAGS
 #define PT_PAX_FLAGS 0x65041580
#endif

int
remove_ptpax(int fd)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		printf("\tELF ERROR: Library out of date.\n");
		return EXIT_FAILURE;
	}

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
	{
		printf("\tELF ERROR: elf_begin() fail: %s\n", elf_errmsg(elf_errno()));
		return EXIT_FAILURE;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		printf("\tELF ERROR: elf_kind() fail: this is not an elf file.\n");
		return EXIT_FAILURE;
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			printf("\tELF ERROR: gelf_getphdr(): %s\n", elf_errmsg(elf_errno()));
			return EXIT_FAILURE;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			phdr.p_type = PT_NULL;
			if(!gelf_update_phdr(elf, i, &phdr))
			{
				elf_end(elf);
				printf("\tELF ERROR: gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
				return EXIT_FAILURE;
			}
		}
	}

	elf_end(elf);
	return EXIT_SUCCESS;
}


int
main( int argc, char *argv[])
{
	int fd;

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]) ;
		exit(EXIT_SUCCESS);
	}

	if((fd = open(argv[1], O_RDWR)) < 0)
	{
		printf("\topen(O_RDWR) failed: cannot change PT_PAX flags\n");
		exit(EXIT_FAILURE);
	}
	else
		exit(remove_ptpax(fd));

}
