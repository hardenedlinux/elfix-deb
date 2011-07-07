
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
		error(EXIT_FAILURE, 0, "Failed open file.");


	cmd = ELF_C_READ;

	if((arf = elf_begin(fd, cmd, (Elf *)0)) == NULL)
		error(EXIT_FAILURE, 0, "Failed open elf: %s", elf_errmsg ( -1));
		

	switch(elf_kind(arf))
	{
		case ELF_K_AR:
			printf("This is an archive.\n");
			arhdr = elf_getarhdr(arf);
			/*
			printf("\n ********** ARCHIVE HEADER ********** \n");
			printf( "Name:\t\t%s\nDate:\t\t%lu\nUID:\t\t%d\nGID:\t\t%d\n"
				"Mode:\t\t%d\n:Size:\t\t%lu\nRaw:\t\t%s\n\n ",
				arhdr->ar_name,
				arhdr->ar_date,
				arhdr->ar_uid,
				arhdr->ar_gid,
				arhdr->ar_mode,
				arhdr->ar_size,
				arhdr->ar_rawname
			);
			*/
			break;
		case ELF_K_COFF:
			printf("This is a COFF.\n"); break;
		case ELF_K_ELF:
			printf("This is an ELF.\n"); break;
		default:
		case ELF_K_NONE:
			printf("This is an unknown.\n"); break;
	}

	while((elf = elf_begin(fd, cmd, arf)) != NULL)
	{
		if(gelf_getehdr(elf,&ehdr) != NULL)
		{
			printf("\n ********** HEADER ********** \n");
			switch(ehdr.e_type)
			{
				case ET_NONE:	printf("This is a ET_NONE Type.\n"); break;
				case ET_REL:	printf("This is a ET_REL Type.\n"); break;
				case ET_EXEC:	printf("This is a ET_EXEC Type.\n"); break;
				case ET_DYN:	printf("This is a ET_DYN Type.\n"); break;
				case ET_CORE:	printf("This is a ET_CORE Type.\n"); break;
				case ET_NUM:	printf("This is a ET_NUM Type.\n"); break;
				case ET_LOOS:	printf("This is a ET_LOOS Type.\n"); break;
				case ET_HIOS:	printf("This is a ET_HIOS Type.\n"); break;
				case ET_LOPROC:	printf("This is a ET_LOPROC Type.\n"); break;
				case ET_HIPROC:	printf("This is a ET_HIPROC Type.\n"); break;
			}

			printf( "Ident:\t\t%d\nType:\t\t%d\nMachine:\t%d\nVersion:\t%d\n"
				"Entry:\t\t%lu\nPHoff:\t\t%lu\nSHoff:\t\t%lu\n"
				"Flags:\t\t%d\nEHSize:\t\t%d\nPHentsize:\t%d\n"
				"PHnum:\t\t%d\nSHentsize\t%d\nSHnum\t\t%d\nSHstrndx\t%d\n\n",
				ehdr.e_ident[EI_NIDENT],
				ehdr.e_type,
				ehdr.e_machine,
				ehdr.e_version,

				ehdr.e_entry,
				ehdr.e_phoff,
   				ehdr.e_shoff,

				ehdr.e_flags,
				ehdr.e_ehsize,
				ehdr.e_phentsize,

				ehdr.e_phnum,
				ehdr.e_shentsize,
				ehdr.e_shnum,
				ehdr.e_shstrndx
			);

			elf_getphdrnum(elf, &n);
			printf("NOTE: elf_getphdrnum=%lu ehdr.e_phnum=%d\n", n, ehdr.e_phnum );

			for (i = 0; i < ehdr.e_phnum; ++i)
			{
				if(gelf_getphdr(elf, i, &phdr) != &phdr)
					error(EXIT_FAILURE, 0, "Failed getphdr: %s", elf_errmsg ( -1));

				printf("\n ********** PROGRAM HEADER TABLE ENTRY ********** \n");
				switch(phdr.p_type)
				{
					case PT_NULL:		printf("This is a PT_NULL type\n"); break;
					case PT_LOAD:		printf("This is a PT_LOAD type\n"); break;
					case PT_DYNAMIC:	printf("This is a PT_DYNAMIC type\n"); break;
					case PT_INTERP:		printf("This is a PT_INTERP type\n"); break;
					case PT_NOTE:		printf("This is a PT_NOTE type\n"); break;
					case PT_SHLIB:		printf("This is a PT_SHLIB type\n"); break;
					case PT_PHDR:		printf("This is a PT_PHDR type\n"); break;
					case PT_TLS:		printf("This is a PT_TLS type\n"); break;
					case PT_NUM:		printf("This is a PT_NUM type\n"); break;
					case PT_LOOS:		printf("This is a PT_LOOS type\n"); break;
					case PT_GNU_EH_FRAME:	printf("This is a PT_GNU_EH_FRAME type\n"); break;
					case PT_GNU_STACK:	printf("This is a PT_GNU_STACK type\n"); break;
					case PT_GNU_RELRO:	printf("This is a PT_GNU_RELRO type\n"); break;
					case PT_PAX_FLAGS:	printf("This is a PT_PAX_FLAGS type\n"); break;
					case PT_LOSUNW:		printf("This is a PT_LOSUNW type\n"); break;
					//case PT_SUNWBSS:	printf("This is a PT_SUNWBSS type\n"); break;
					case PT_SUNWSTACK:	printf("This is a PT_SUNWSTACK type\n"); break;
					case PT_HISUNW:		printf("This is a PT_HISUNW type\n"); break;
					//case PT_HIOS:		printf("This is a PT_HIOS type\n"); break;
					case PT_LOPROC:		printf("This is a PT_LOPROC type\n"); break;
					case PT_HIPROC:		printf("This is a PT_HIPROC type\n"); break;
				}

				printf("Flags:\t\t");
				if( phdr.p_flags & PF_R ) printf("R");
				if( phdr.p_flags & PF_W ) printf("W");
				if( phdr.p_flags & PF_X ) printf("X");
				printf("\n");

				printf( "Type:\t\t%d\nFlags:\t\t%d\nOffset:\t\t%lu\nVaddr:\t\t%lu\nPaddr:\t\t%lu\n"
					"Filesz:\t\t%lu\nMemsz:\t\t%lu\nAlign:\t\t%lu\n",
					phdr.p_type,
					phdr.p_flags,
					phdr.p_offset,
					phdr.p_vaddr,
					phdr.p_paddr,
					phdr.p_filesz,
					phdr.p_memsz,
					phdr.p_align
				);

				printf("\n\n");
			}

			scn = NULL;
			while((scn = elf_nextscn(elf, scn)) != NULL)
			{
				gelf_getshdr(scn, &shdr);
				printf("\n ********** SECTION ********** \n");

				printf("Section name: %s\n", elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name));

				switch(shdr.sh_type)
				{
					case SHT_DYNAMIC:	printf("This is a SHT_DYNAMIC type\n"); break;
					case SHT_DYNSYM:	printf("This is a SHT_DYNSYM type\n"); break;
					case SHT_HASH:		printf("This is a SHT_HASH type\n"); break;
					case SHT_NOBITS:	printf("This is a SHT_NOBITS type\n"); break;
					case SHT_NOTE:		printf("This is a SHT_NOTE type\n"); break;
					case SHT_NULL:		printf("This is a SHT_NULL type\n"); break;
					case SHT_PROGBITS:	printf("This is a SHT_PROGBITS type\n"); break;
					case SHT_REL:		printf("This is a SHT_REL type\n"); break;
					case SHT_RELA:		printf("This is a SHT_RELA type\n"); break;
					case SHT_STRTAB:	printf("This is a SHT_STRTAB type\n"); break;
					case SHT_SYMTAB:	printf("This is a SHT_SYMTAB type\n"); break;
					default:		printf("This is an unknown section type\n"); break;
				}

				printf(
					"Name:\t\t%d\nType:\t\t%d\nFlags:\t\t%lu\n"
					"Addr:\t\t%lu\nOffset:\t\t%lu\nSize:\t\t%lu\n"
					"Link:\t\t%d\nInfo:\t\t%d\nAddrAlign:\t%lu\nEntsize:\t%lu\n",
					shdr.sh_name,
   					shdr.sh_type,
					shdr.sh_flags,
   					shdr.sh_addr,
					shdr.sh_offset,
   					shdr.sh_size,
					shdr.sh_link,
   					shdr.sh_info,
					shdr.sh_addralign,
   					shdr.sh_entsize
				);

				if((data = elf_getdata(scn, data)) != NULL)
				{
					printf("\n ***** DATA ***** \n");
					printf( "Data:\t\t%s\nType:\t\t%d\nSize:\t\t%lu\n"
						"Off:\t\t%lu\nAlign:\t\t%lu\nVersion:\t%u\n",
						(char *)data->d_buf,
						data->d_type,
						data->d_size,
						data->d_off,
						data->d_align,
   						data->d_version
					);
				}
				printf("\n\n");

			}


			//Print out data in .shstrtab section
			if((scn = elf_getscn(elf, ehdr.e_shstrndx)) == NULL)
				error(EXIT_FAILURE, 0, "getscn() failed: %s.", elf_errmsg(-1));

			if(gelf_getshdr( scn, &shdr ) != &shdr)
				error(EXIT_FAILURE, 0, "getshdr(ehdr.e_shstrndx) failed: %s.", elf_errmsg(-1));

			printf(" .shstrab: size=%jd\n", (uintmax_t)shdr.sh_size);

			data = NULL;
			n = 0;
			while( n < shdr.sh_size && (data = elf_getdata(scn, data)) != NULL )
			{
				p = (char *)data->d_buf ;
				while(p < (char *)data->d_buf + data->d_size)
				{
					printf("%c", *p);
					n++;
					p++;
					if(!(n%16)) printf("\n");
				}
			}
			printf("\n");

		}
		cmd = elf_next(elf);
		elf_end(elf);
	}

	elf_end(arf);

	close(fd);
}
