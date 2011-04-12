
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

	Elf *arf, *elf;
	Elf_Arhdr *arhdr;
	GElf_Ehdr ehdr;
	GElf_Phdr phdr;
	Elf_Scn *scn;
	GElf_Shdr shdr;
	Elf_Data *data;

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if(argc != 2)
		error(EXIT_FAILURE, 0, "Usage: %s <filename>", argv[0]);

	if((fd = open(argv[1], O_RDONLY)) == -1)
		error(EXIT_FAILURE, 0, "open() fail.");

	if((elf = elf_begin(fd, ELF_C_READ, (Elf *)0)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin() fail: %s", elf_errmsg(-1));

	if(elf_kind(elf) != ELF_K_ELF)
		error(EXIT_FAILURE, 0, "elf_kind() fail: this is not an elf file.");

	if(gelf_getehdr(elf,&ehdr) == NULL)
		error(EXIT_FAILURE, 0, "gelf_getehdr() fail: %s", elf_errmsg(-1));

	/*
	printf("ELF Type:\t\t");
	switch(ehdr.e_type)
	{
		case ET_NONE:	printf("ET_NONE\n"); break;
		case ET_REL:	printf("ET_REL\n"); break;
		case ET_EXEC:	printf("ET_EXEC\n"); break;
		case ET_DYN:	printf("ET_DYN\n"); break;
		case ET_CORE:	printf("ET_CORE\n"); break;
		case ET_NUM:	printf("ET_NUM\n"); break;
		case ET_LOOS:	printf("ET_LOOS\n"); break;
		case ET_HIOS:	printf("ET_HIOS\n"); break;
		case ET_LOPROC:	printf("ET_LOPROC\n"); break;
		case ET_HIPROC:	printf("ET_HIPROC\n"); break;
	} 
	printf("\n");
	*/

	elf_getphdrnum(elf, &phnum);
	for (i=0; i<phnum; ++i)
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
		}
		/*
		switch(phdr.p_type)
		{
			case PT_NULL:		printf("PT_NULL:\t\t"); break;
			case PT_LOAD:		printf("PT_LOAD:\t\t"); break;
			case PT_DYNAMIC:	printf("PT_DYNAMIC:\t\t"); break;
			case PT_INTERP:		printf("PT_INTERP:\t\t"); break;
			case PT_NOTE:		printf("PT_NOTE:\t\t"); break;
			case PT_SHLIB:		printf("PT_SHLIB:\t\t"); break;
			case PT_PHDR:		printf("PT_PHDR:\t\t"); break;
			case PT_TLS:		printf("PT_TLS:\t\t"); break;
			case PT_NUM:		printf("PT_NUM:\t\t"); break;
			case PT_LOOS:		printf("PT_LOOS:\t\t"); break;
			case PT_GNU_EH_FRAME:	printf("PT_GNU_EH_FRAME:\t"); break;
			case PT_GNU_STACK:	printf("PT_GNU_STACK:\t\t"); break;
			case PT_GNU_RELRO:	printf("PT_GNU_RELRO:\t\t"); break;
			case PT_PAX_FLAGS:	printf("PT_PAX_FLAGS:\t\t"); break;
			case PT_LOSUNW:		printf("PT_LOSUNW:\t\t"); break;
			//case PT_SUNWBSS:	printf("PT_SUNWBSS:\t\t"); break;
			case PT_SUNWSTACK:	printf("PT_SUNWSTACK:\t\t"); break;
			case PT_HISUNW:		printf("PT_HISUNW:\t\t"); break;
			//case PT_HIOS:		printf("PT_HIOS:\t\t"); break;
			case PT_LOPROC:		printf("PT_LOPROC:\t\t"); break;
			case PT_HIPROC:		printf("PT_HIPROC:\t\t"); break;
		}

		if( phdr.p_flags & PF_R ) printf("R");
		if( phdr.p_flags & PF_W ) printf("W");
		if( phdr.p_flags & PF_X ) printf("X");
		printf("\n");
		*/
	}

	elf_end(elf);
	close(fd);
}
