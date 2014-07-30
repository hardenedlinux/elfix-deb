
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
	int fd, cmd;
	size_t i, n;
	char *p; 

	Elf *arf, *elf;
	GElf_Ehdr ehdr;
	Elf_Scn *scn;
	GElf_Shdr shdr;
	Elf_Data *data;
	GElf_Dyn dyn;

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if(argc != 2)
		error(EXIT_FAILURE, 0, "Usage: %s <filename>", argv[0]);

	if((fd = open(argv[1], O_RDONLY)) == -1)
		error(EXIT_FAILURE, 0, "Failed open file.");


	cmd = ELF_C_READ;

	if((arf = elf_begin(fd, cmd, (Elf *)0)) == NULL)
		error(EXIT_FAILURE, 0, "Failed open elf: %s", elf_errmsg ( -1));
		

	while((elf = elf_begin(fd, cmd, arf)) != NULL)
	{
		if(gelf_getehdr(elf,&ehdr) != NULL)
		{
			scn = NULL;
			while((scn = elf_nextscn(elf, scn)) != NULL)
			{
				gelf_getshdr(scn, &shdr);

				if(shdr.sh_type != SHT_DYNAMIC)
					continue;

				printf("Section name: %s\n", elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name));

				data = NULL;
				while((data = elf_getdata(scn, data)) != NULL)
				{
					if(data != NULL)
						for( i=0; gelf_getdyn(data, i, &dyn) != NULL; i++)
						{
							if(dyn.d_tag == DT_RPATH)
								printf("DT_RPATH found\n");
							if(dyn.d_tag == DT_RUNPATH)
								printf("DT_RUNPATH found\n");
						}
				}
			}
		}

		cmd = elf_next(elf);
		elf_end(elf);

	}

	elf_end(arf);

	close(fd);
}
