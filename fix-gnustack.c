
#include <stdio.h>	// printf
#include <stdlib.h>	// EXIT_FAILURE
#include <error.h>	// error

#include <gelf.h>	// elf_* and gelf_*

#include <sys/types.h>	// open
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	// close


int main( int argc, char *argv[])
{
	int fd;
	size_t i, phnum;

	Elf *elf;
	GElf_Ehdr ehdr;
	GElf_Phdr phdr;
	GElf_Word nflags;

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if(argc != 2)
		error(EXIT_FAILURE, 0, "Usage: %s <filename>", argv[0]);

	if((fd = open(argv[1], O_RDWR)) < 0)
		error(EXIT_FAILURE, 0, "open() fail.");

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(-1));

	if(elf_kind(elf) != ELF_K_ELF)
		error(EXIT_FAILURE, 0, "elf_kind() report: this is not an elf file.");

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

			if((phdr.p_flags & PF_W) && (phdr.p_flags & PF_X))
			{
				printf("W&X found, flipping X flag ...\n");
				nflags = phdr.p_flags ^ PF_X; 
				printf("oflags=%u PF_X=%d nflags=%u\n", phdr.p_flags, PF_X, nflags);

				phdr.p_flags = nflags ;
				gelf_update_phdr(elf, i, &phdr);
			}
		}
	}

	elf_end(elf);
	close(fd);
}
