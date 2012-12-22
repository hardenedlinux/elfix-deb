/*
	paxctl-ng.c: this file is part of the elfix package
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef PTPAX
 #include <gelf.h>
#endif

#ifdef NEED_PAX_DECLS
 #define PT_PAX_FLAGS    0x65041580      /* Indicates PaX flag markings */
 #define PF_PAGEEXEC     (1 << 4)        /* Enable  PAGEEXEC */
 #define PF_NOPAGEEXEC   (1 << 5)        /* Disable PAGEEXEC */
 #define PF_SEGMEXEC     (1 << 6)        /* Enable  SEGMEXEC */
 #define PF_NOSEGMEXEC   (1 << 7)        /* Disable SEGMEXEC */
 #define PF_MPROTECT     (1 << 8)        /* Enable  MPROTECT */
 #define PF_NOMPROTECT   (1 << 9)        /* Disable MPROTECT */
 #define PF_RANDEXEC     (1 << 10)       /* DEPRECATED: Enable  RANDEXEC */
 #define PF_NORANDEXEC   (1 << 11)       /* DEPRECATED: Disable RANDEXEC */
 #define PF_EMUTRAMP     (1 << 12)       /* Enable  EMUTRAMP */
 #define PF_NOEMUTRAMP   (1 << 13)       /* Disable EMUTRAMP */
 #define PF_RANDMMAP     (1 << 14)       /* Enable  RANDMMAP */
 #define PF_NORANDMMAP   (1 << 15)       /* Disable RANDMMAP */
#endif

#ifdef XTPAX
 #include <attr/xattr.h>
 #define PAX_NAMESPACE	"user.pax.flags"
 #define CREATE_XT_FLAGS_SECURE         1
 #define CREATE_XT_FLAGS_DEFAULT        2
 #define DELETE_XT_FLAGS                3
#endif

#if defined(PTPAX) && defined(XTPAX)
 #define COPY_PT_TO_XT_FLAGS            4
 #define COPY_XT_TO_PT_FLAGS            5
 #define LIMIT_TO_PT_FLAGS              6
 #define LIMIT_TO_XT_FLAGS              7
#endif

#define FLAGS_SIZE                      6

#include <config.h>

void
print_help_exit(char *v)
{
	printf(
		"\n"
		"Package Name : " PACKAGE_STRING "\n"
		"Bug Reports  : " PACKAGE_BUGREPORT "\n"
		"Program Name : %s\n"
		"Description  : Get or set pax flags on an ELF object\n\n"
		"Usage        : %s -PpEeMmRrSsv ELF | -Zv ELF | -zv ELF\n"
#ifdef XTPAX
		"             : %s -Cv ELF | -cv ELF | -dv ELF\n"
#endif
#if defined(PTPAX) && defined(XTPAX)
		"             : %s -Fv ELF | -fv ELF\n"
		"             : %s -Lv ELF | -lv ELF\n"
#endif
		"             : %s -v ELF | -h\n\n"
		"Options      : -P enable PAGEEXEC\t-p disable  PAGEEXEC\n"
		"             : -E enable EMUTRAMP\t-e disable  EMUTRAMP\n"
		"             : -M enable MPROTECT\t-m disable  MPROTECT\n"
		"             : -R enable RANDMMAP\t-r disable  RANDMMAP\n"
		"             : -S enable SEGMEXEC\t-s disable  SEGMEXEC\n"
		"             : -Z all secure settings\t-z all default settings\n"
		"             :\n"
#ifdef XTPAX
		"             : -C create XATTR_PAX with most secure setting\n"
		"             : -c create XATTR_PAX all default settings\n"
		"             : -d delete XATTR_PAX field\n"
#endif
#if defined(PTPAX) && defined(XTPAX)
		"             : -F copy PT_PAX to XATTR_PAX\n"
		"             : -f copy XATTR_PAX to PT_PAX\n"
		"             : -L set only PT_PAX flags\n"
		"             : -l set only XATTR_PAX flags\n"
#endif
		"             :\n"
		"             : -v view the flags, along with any accompanying operation\n"
		"             : -h print out this help\n\n"
		"Note         :  If both enabling and disabling flags are set, the default - is used\n\n",
		basename(v),
		basename(v),
#ifdef XTPAX
		basename(v),
#endif
#if defined(PTPAX) && defined(XTPAX)
		basename(v),
		basename(v),
#endif
		basename(v)
	);

	exit(EXIT_SUCCESS);
}


void
parse_cmd_args(int argc, char *argv[], uint16_t *pax_flags, int *verbose, int *cp_flags,
	int *limit, int *begin, int *end)
{
	int i, oc;
	int compat, solitaire;

	compat = 0;
	solitaire = 0;
	*pax_flags = 0;
	*verbose = 0;
	*cp_flags = 0; 

	/* Accept all options and silently ignore irrelevant ones below.
	 * We can then pass any parameter in scripts without failure.
	 *
	 * Alternatively we could do
	 *
	 * #if !defined(PTPAX) && defined(XTPAX)
	 *	while((oc = getopt(argc, argv,":PpSsMmEeRrZzCcvh")) != -1)
	 * #elif defined(PTPAX) && defined(XTPAX)
	 *	while((oc = getopt(argc, argv,":PpSsMmEeRrZzCcFfvh")) != -1)
	 * #else
	 *	while((oc = getopt(argc, argv,":PpSsMmEeRrZzvh")) != -1)
	 * #endif
	 */

	while((oc = getopt(argc, argv,":PpEeMmRrSsZzCcdFfLlvh")) != -1)
	{
		switch(oc)
		{
			case 'P':
				*pax_flags |= PF_PAGEEXEC;
				compat |= 1;
				break;
			case 'p':
				*pax_flags |= PF_NOPAGEEXEC;
				compat |= 1;
				break ;
			case 'E':
				*pax_flags |= PF_EMUTRAMP;
				compat |= 1;
				break;
			case 'e':
				*pax_flags |= PF_NOEMUTRAMP;
				compat |= 1;
				break ;
			case 'M':
				*pax_flags |= PF_MPROTECT;
				compat |= 1;
				break;
			case 'm':
				*pax_flags |= PF_NOMPROTECT;
				compat |= 1;
				break ;
			case 'R':
				*pax_flags |= PF_RANDMMAP;
				compat |= 1;
				break;
			case 'r':
				*pax_flags |= PF_NORANDMMAP;
				compat |= 1;
				break ;
			case 'S':
				*pax_flags |= PF_SEGMEXEC;
				compat |= 1;
				break;
			case 's':
				*pax_flags |= PF_NOSEGMEXEC;
				compat |= 1;
				break ;
			case 'Z':
				*pax_flags = PF_PAGEEXEC | PF_SEGMEXEC | PF_MPROTECT |
					PF_NOEMUTRAMP | PF_RANDMMAP ;
				solitaire += 1;
				break ;
			case 'z':
				*pax_flags = PF_PAGEEXEC | PF_NOPAGEEXEC | PF_SEGMEXEC | PF_NOSEGMEXEC |
					PF_MPROTECT | PF_NOMPROTECT | PF_EMUTRAMP | PF_NOEMUTRAMP |
					PF_RANDMMAP | PF_NORANDMMAP ;
				solitaire += 1;
				break;
#ifdef XTPAX
			case 'C':
				solitaire += 1;
				*cp_flags = CREATE_XT_FLAGS_SECURE;
				break;
			case 'c':
				solitaire += 1;
				*cp_flags = CREATE_XT_FLAGS_DEFAULT;
				break;
			case 'd':
				solitaire += 1;
				*cp_flags = DELETE_XT_FLAGS;
				break;
#else
			case 'C':
			case 'c':
				break;
#endif
#if defined(PTPAX) && defined(XTPAX)
			case 'F':
				solitaire += 1;
				*cp_flags = COPY_PT_TO_XT_FLAGS;
				break;
			case 'f':
				solitaire += 1;
				*cp_flags = COPY_XT_TO_PT_FLAGS;
				break;
			case 'L':
				*limit = LIMIT_TO_PT_FLAGS;
				break;
			case 'l':
				*limit = LIMIT_TO_XT_FLAGS;
				break;
#else
			case 'F':
			case 'f':
			case 'L':
			case 'l':
				break;
#endif
			case 'v':
				*verbose = 1;
				break;
			case 'h':
				print_help_exit(argv[0]);
				break;
			case '?':
			default:
				error(EXIT_FAILURE, 0, "option -%c is invalid: ignored.", optopt ) ;
		}
	}

	if(
		(
		 (compat == 1 && solitaire == 0) ||
		 (compat == 0 && solitaire == 1) ||
		 (compat == 0 && solitaire == 0 && *verbose == 1)
		)
		&& argv[optind] != NULL
	)
	{
		*begin = optind;
		*end = argc;
	}
	else
		print_help_exit(argv[0]);
}


#ifdef PTPAX
uint16_t
get_pt_flags(int fd, int verbose)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	uint16_t pt_flags = UINT16_MAX;

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		if(verbose)
			printf("\tELF ERROR: Library out of date.\n");
		return pt_flags;
	}

	if((elf = elf_begin(fd, ELF_C_READ_MMAP, NULL)) == NULL)
	{
		if(verbose)
			printf("\tELF ERROR: elf_begin() fail: %s\n", elf_errmsg(elf_errno()));
		return pt_flags;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		if(verbose)
			printf("\tELF ERROR: elf_kind() fail: this is not an elf file.\n");
		return pt_flags;
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			if(verbose)
				printf("\tELF ERROR: gelf_getphdr(): %s\n", elf_errmsg(elf_errno()));
			return pt_flags;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
			pt_flags = phdr.p_flags;
	}

	elf_end(elf);
	return pt_flags;
}
#endif


#ifdef XTPAX
uint16_t
string2bin(char *buf)
{
	int i;
	uint16_t flags = 0;

	for(i = 0; i < 5; i++)
	{
		if(buf[i] == 'P')
			flags |= PF_PAGEEXEC;
		else if(buf[i] == 'p')
			flags |= PF_NOPAGEEXEC;

		if(buf[i] == 'E')
			flags |= PF_EMUTRAMP;
		else if(buf[i] == 'e')
			flags |= PF_NOEMUTRAMP;

		if(buf[i] == 'M')
			flags |= PF_MPROTECT;
		else if(buf[i] == 'm')
			flags |= PF_NOMPROTECT;

		if(buf[i] == 'R')
			flags |= PF_RANDMMAP;
		else if(buf[i] == 'r')
			flags |= PF_NORANDMMAP;

		if(buf[i] == 'S')
			flags |= PF_SEGMEXEC;
		else if(buf[i] == 's')
			flags |= PF_NOSEGMEXEC;
	}

	return flags;
}


uint16_t
get_xt_flags(int fd)
{
	char buf[FLAGS_SIZE];
	uint16_t xt_flags = UINT16_MAX;

	memset(buf, 0, FLAGS_SIZE);

	if(fgetxattr(fd, PAX_NAMESPACE, buf, FLAGS_SIZE) != -1)
		xt_flags = string2bin(buf);

	return xt_flags;
}
#endif


void
bin2string4print(uint16_t flags, char *buf)
{
	buf[0] = flags & PF_PAGEEXEC ? 'P' :
		flags & PF_NOPAGEEXEC ? 'p' : '-' ;

	buf[1] = flags & PF_EMUTRAMP   ? 'E' :
		flags & PF_NOEMUTRAMP ? 'e' : '-';

	buf[2] = flags & PF_MPROTECT   ? 'M' :
		flags & PF_NOMPROTECT ? 'm' : '-';

	buf[3] = flags & PF_RANDMMAP   ? 'R' :
		flags & PF_NORANDMMAP ? 'r' : '-';

	buf[4] = flags & PF_SEGMEXEC   ? 'S' :
		flags & PF_NOSEGMEXEC ? 's' : '-';
}


void
bin2string(uint16_t flags, char *buf)
{
	int i;

	for(i = 0; i < 5; i++)
		buf[i] = 0;

	i = 0;

	if(flags & PF_PAGEEXEC)
		buf[i++] = 'P';
	else if(flags & PF_NOPAGEEXEC)
		buf[i++] = 'p';

	if(flags & PF_EMUTRAMP)
		buf[i++] = 'E';
	else if(flags & PF_NOEMUTRAMP)
		buf[i++] = 'e';

	if(flags & PF_MPROTECT)
		buf[i++] = 'M';
	else if(flags & PF_NOMPROTECT)
		buf[i++] = 'm';

	if(flags & PF_RANDMMAP)
		buf[i++] = 'R';
	if(flags & PF_NORANDMMAP)
		buf[i++] = 'r';

	if(flags & PF_SEGMEXEC)
		buf[i++] = 'S';
	else if(flags & PF_NOSEGMEXEC)
		buf[i++] = 's';
}


void
print_flags(int fd, int verbose)
{
	uint16_t flags;
	char buf[FLAGS_SIZE];

#ifdef PTPAX
	flags = get_pt_flags(fd, verbose);
	if( flags == UINT16_MAX )
		printf("\tPT_PAX   : not found\n");
	else
	{
		memset(buf, 0, FLAGS_SIZE);
		bin2string4print(flags, buf);
		printf("\tPT_PAX   : %s\n", buf);
	}
#endif

#ifdef XTPAX
	flags = get_xt_flags(fd);
	if( flags == UINT16_MAX )
		printf("\tXATTR_PAX: not found\n");
	else
	{
		memset(buf, 0, FLAGS_SIZE);
		bin2string4print(flags, buf);
		printf("\tXATTR_PAX: %s\n", buf);
	}
#endif
}



uint16_t
update_flags(uint16_t flags, uint16_t pax_flags)
{
	//PAGEEXEC
	if(pax_flags & PF_PAGEEXEC)
	{
		flags |= PF_PAGEEXEC;
		flags &= ~PF_NOPAGEEXEC;
	}
	if(pax_flags & PF_NOPAGEEXEC)
	{
		flags &= ~PF_PAGEEXEC;
		flags |= PF_NOPAGEEXEC;
	}
	if((pax_flags & PF_PAGEEXEC) && (pax_flags & PF_NOPAGEEXEC))
	{
		flags &= ~PF_PAGEEXEC;
		flags &= ~PF_NOPAGEEXEC;
	}

	//EMUTRAMP
	if(pax_flags & PF_EMUTRAMP)
	{
		flags |= PF_EMUTRAMP;
		flags &= ~PF_NOEMUTRAMP;
	}
	if(pax_flags & PF_NOEMUTRAMP)
	{
		flags &= ~PF_EMUTRAMP;
		flags |= PF_NOEMUTRAMP;
	}
	if((pax_flags & PF_EMUTRAMP) && (pax_flags & PF_NOEMUTRAMP))
	{
		flags &= ~PF_EMUTRAMP;
		flags &= ~PF_NOEMUTRAMP;
	}

	//MPROTECT
	if(pax_flags & PF_MPROTECT)
	{
		flags |= PF_MPROTECT;
		flags &= ~PF_NOMPROTECT;
	}
	if(pax_flags & PF_NOMPROTECT)
	{
		flags &= ~PF_MPROTECT;
		flags |= PF_NOMPROTECT;
	}
	if((pax_flags & PF_MPROTECT) && (pax_flags & PF_NOMPROTECT))
	{
		flags &= ~PF_MPROTECT;
		flags &= ~PF_NOMPROTECT;
	}

	//RANDMMAP
	if(pax_flags & PF_RANDMMAP)
	{
		flags |= PF_RANDMMAP;
		flags &= ~PF_NORANDMMAP;
	}
	if(pax_flags & PF_NORANDMMAP)
	{
		flags &= ~PF_RANDMMAP;
		flags |= PF_NORANDMMAP;
	}
	if((pax_flags & PF_RANDMMAP) && (pax_flags & PF_NORANDMMAP))
	{
		flags &= ~PF_RANDMMAP;
		flags &= ~PF_NORANDMMAP;
	}

	//SEGMEXEC
	if(pax_flags & PF_SEGMEXEC)
	{
		flags |= PF_SEGMEXEC;
		flags &= ~PF_NOSEGMEXEC;
	}
	if(pax_flags & PF_NOSEGMEXEC)
	{
		flags &= ~PF_SEGMEXEC;
		flags |= PF_NOSEGMEXEC;
	}
	if((pax_flags & PF_SEGMEXEC) && (pax_flags & PF_NOSEGMEXEC))
	{
		flags &= ~PF_SEGMEXEC;
		flags &= ~PF_NOSEGMEXEC;
	}

	return flags;
}


#ifdef PTPAX
int
set_pt_flags(int fd, uint16_t pt_flags, int verbose)
{
	Elf *elf;
	GElf_Phdr phdr;
	size_t i, phnum;

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		if(verbose)
			printf("\tELF ERROR: Library out of date.\n");
		return EXIT_FAILURE;
	}

	if((elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL)) == NULL)
	{
		if(verbose)
			printf("\tELF ERROR: elf_begin() fail: %s\n", elf_errmsg(elf_errno()));
		return EXIT_FAILURE;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		elf_end(elf);
		if(verbose)
			printf("\tELF ERROR: elf_kind() fail: this is not an elf file.\n");
		return EXIT_FAILURE;
	}

	elf_getphdrnum(elf, &phnum);

	for(i=0; i<phnum; i++)
	{
		if(gelf_getphdr(elf, i, &phdr) != &phdr)
		{
			elf_end(elf);
			if(verbose)
				printf("\tELF ERROR: gelf_getphdr(): %s\n", elf_errmsg(elf_errno()));
			return EXIT_FAILURE;
		}

		if(phdr.p_type == PT_PAX_FLAGS)
		{
			//RANDEXEC is deprecated, we'll force it off like paxctl
			phdr.p_flags = pt_flags | PF_NORANDEXEC;

			if(!gelf_update_phdr(elf, i, &phdr))
			{
				elf_end(elf);
				if(verbose)
					printf("\tELF ERROR: gelf_update_phdr(): %s", elf_errmsg(elf_errno()));
				return EXIT_FAILURE;
			}
		}
	}

	elf_end(elf);
	return EXIT_SUCCESS;
}
#endif


#ifdef XTPAX
int
set_xt_flags(int fd, uint16_t xt_flags)
{
	char buf[FLAGS_SIZE];

	memset(buf, 0, FLAGS_SIZE);
	bin2string(xt_flags, buf);

	if( !fsetxattr(fd, PAX_NAMESPACE, buf, strlen(buf), 0) )
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
#endif


int
set_flags(int fd, uint16_t *pax_flags, int rdwr_pt_pax, int limit, int verbose)
{
	uint16_t flags;
	int ret = EXIT_FAILURE;

#ifdef PTPAX
	if(rdwr_pt_pax)
	{
#ifdef XTPAX
		if( !(limit == LIMIT_TO_XT_FLAGS))
		{
#endif
			flags = get_pt_flags(fd, verbose);
			if( flags == UINT16_MAX )
				flags = PF_NOEMUTRAMP ;
			flags = update_flags( flags, *pax_flags);
			ret = set_pt_flags(fd, flags, verbose);
#ifdef XTPAX
		}
#endif

	}
#endif

#ifdef XTPAX
#ifdef PTPAX
	if( !(limit == LIMIT_TO_PT_FLAGS) )
	{
#endif
		flags = get_xt_flags(fd);
		if( flags == UINT16_MAX )
			flags = PF_NOEMUTRAMP ;
		flags = update_flags( flags, *pax_flags);
		ret = set_xt_flags(fd, flags);
#ifdef PTPAX
	}
#endif
#endif

	return ret;
}


#ifdef XTPAX
int
create_xt_flags(int fd, int cp_flags)
{
	char buf[FLAGS_SIZE];
	uint16_t xt_flags;

	if(cp_flags == CREATE_XT_FLAGS_SECURE)
		xt_flags = PF_PAGEEXEC | PF_SEGMEXEC | PF_MPROTECT |
			PF_NOEMUTRAMP | PF_RANDMMAP ;
	else if(cp_flags == CREATE_XT_FLAGS_DEFAULT)
		xt_flags = 0;

	memset(buf, 0, FLAGS_SIZE);
	bin2string(xt_flags, buf);

	if( !fsetxattr(fd, PAX_NAMESPACE, buf, strlen(buf), XATTR_CREATE) )
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

int
delete_xt_flags(int fd)
{
	if( !fremovexattr(fd, PAX_NAMESPACE) )
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
#endif


#if defined(PTPAX) && defined(XTPAX)
int
copy_xt_flags(int fd, int cp_flags, int verbose)
{
	uint16_t flags;
	int ret = EXIT_FAILURE;

	if(cp_flags == COPY_PT_TO_XT_FLAGS)
	{
		flags = get_pt_flags(fd, verbose);
		if( flags != UINT16_MAX )
			ret = set_xt_flags(fd, flags);
	}
	else if(cp_flags == COPY_XT_TO_PT_FLAGS)
	{
		flags = get_xt_flags(fd);
		if( flags != UINT16_MAX )
			ret = set_pt_flags(fd, flags, verbose);
	}

	return ret;
}
#endif


int
main( int argc, char *argv[])
{
	int fd, fi;
	uint16_t pax_flags;
	int verbose, cp_flags, limit, begin, end;
	int rdwr_pt_pax = 1;

	int ret = EXIT_SUCCESS;

	parse_cmd_args(argc, argv, &pax_flags, &verbose, &cp_flags, &limit, &begin, &end);

	for(fi = begin; fi < end; fi++)
	{
		if(verbose)
			printf("%s:\n", argv[fi]);

		if((fd = open(argv[fi], O_RDWR)) < 0)
		{
			rdwr_pt_pax = 0;
#ifdef PTPAX
			if(verbose)
				printf("\topen(O_RDWR) failed: cannot change PT_PAX flags\n");
#endif
			if((fd = open(argv[fi], O_RDONLY)) < 0)
			{
				if(verbose)
					printf("\topen(O_RDONLY) failed: cannot read/change PAX flags\n\n");
				continue;
			}
		}

#ifdef XTPAX
		if(cp_flags == CREATE_XT_FLAGS_SECURE || cp_flags == CREATE_XT_FLAGS_DEFAULT)
			ret |= create_xt_flags(fd, cp_flags);
		if(cp_flags == DELETE_XT_FLAGS)
			ret |= delete_xt_flags(fd);
#endif

#if defined(PTPAX) && defined(XTPAX)
		if(cp_flags == COPY_PT_TO_XT_FLAGS || (cp_flags == COPY_XT_TO_PT_FLAGS && rdwr_pt_pax))
			ret |= copy_xt_flags(fd, cp_flags, verbose);
#endif

		if(pax_flags != 0)
			ret |= set_flags(fd, &pax_flags, rdwr_pt_pax, limit, verbose);

		if(verbose == 1)
			print_flags(fd, verbose);

		close(fd);

		if(verbose)
			printf("\n");
	}

	exit(ret);
}
