
#include <stdio.h>	// printf
#include <stdlib.h>	// EXIT_FAILURE
#include <error.h>	// error
#include <string.h>	// strncpy

#include <gelf.h>	// elf_* and gelf_*

#include <sys/types.h>	// open
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	// close


int main( int argc, char *argv[])
{
	int fd, cmd;
	char *scn_name;

	Elf *elf;
	GElf_Ehdr ehdr;
	Elf_Scn *scn;
	GElf_Shdr shdr;
	Elf_Data *data;
	GElf_Dyn dyn;

	if(argc != 3)
		error(EXIT_FAILURE, 0, "Usage: %s <elf> <ld.so>", argv[0]);

	if(elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "Library out of date.");

	if((fd = open(argv[1], O_RDWR)) == -1)
		error(EXIT_FAILURE, 0, "Failed open file.");

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, elf)) != NULL)
	{
		if(gelf_getehdr(elf,&ehdr) != NULL)
		{
			scn = NULL;
			while((scn = elf_nextscn(elf, scn)) != NULL)
			{
				gelf_getshdr(scn, &shdr);
				scn_name = elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name);

				if(strcmp(scn_name, ".interp"))
					continue;

				printf("Section name: %s\n", elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name));

				if((data = elf_getdata(scn, data)) != NULL)
				{
					printf("Old ld.so: %s\n", (char *)data->d_buf);
					printf("Data size: %zu\n", data->d_size);
					/*
					printf( "Data:\t\t%s\nType:\t\t%d\nSize:\t\t%lu\n"
						"Off:\t\t%lu\nAlign:\t\t%lu\nVersion:\t%u\n",
						(char *)data->d_buf,
						data->d_type,
						data->d_size,
						data->d_off,
						data->d_align,
						data->d_version
					);
					*/

					if(data->d_size >= strlen(argv[2]))
					{
						memset(data->d_buf, 0, data->d_size);
						strncpy(data->d_buf, argv[2], data->d_size);

						if(!gelf_update_shdr(scn, &shdr))
						{
							elf_end(elf);
							close(fd);
							error(EXIT_FAILURE, 0, "\tELF ERROR: gelf_update_shdr(): %s", elf_errmsg(elf_errno()));
						}
					}
					else
					{
						elf_end(elf);
						close(fd);
						error(EXIT_FAILURE, 0, "\tld.so path too long: max=%zu", data->d_size);
					}

					printf( "New ld.so: %s\n", (char *)data->d_buf);
					/*
					printf( "Data:\t\t%s\nType:\t\t%d\nSize:\t\t%lu\n"
						"Off:\t\t%lu\nAlign:\t\t%lu\nVersion:\t%u\n",
						(char *)data->d_buf,
						data->d_type,
						data->d_size,
						data->d_off,
						data->d_align,
						data->d_version
					);
					*/
				}
			}
		}
	}

	elf_end(elf);
	close(fd);
}
