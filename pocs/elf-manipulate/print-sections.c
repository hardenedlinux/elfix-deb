
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include <gelf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main( int argc, char *argv[])
{
	int fd, cmd;
	size_t i;
	char shname[1024];

	Elf *arf, *elf;
	GElf_Ehdr ehdr;
	Elf_Scn *scn;
	GElf_Shdr shdr;
	Elf_Data *data;

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

				strcpy(shname, elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name));
				printf("%s\n", shname);

				/*
				if( !strcmp(shname, ".bss") || !strcmp(shname, ".tbss") )
					continue; 
				*/
				if(shdr.sh_type == SHT_NOBITS)
					continue;

				data = NULL;
				while((data = elf_getdata(scn, data)) != NULL)
				{
					printf( "Type:\t\t%d\nSize:\t\t%lu\n"
						"Off:\t\t%lu\nAlign:\t\t%lu\nVersion:\t%u\n",
						data->d_type,
						data->d_size,
						data->d_off,
						data->d_align,
   						data->d_version
					);

					for(i = 0; i < (int)data->d_size; i++)
					{
						printf("%2x ", (uint8_t)((char *)(data->d_buf))[i]);
						if((i+1)%16 == 0)
							printf("\n");
					}
				}

				printf("\n\n");
			}
		}

		cmd = elf_next(elf);
		elf_end(elf);

	}

	elf_end(arf);

	close(fd);
}
