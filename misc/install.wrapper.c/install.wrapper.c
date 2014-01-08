/*
 * Copyright 2014 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 *
 * Copyright 2014 Anthony G. Basile - <blueness@gentoo.org>
 *
 * Wrapper for coreutil's install to preserve extended attributes.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <ctype.h>
#include <libgen.h>
#include <getopt.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/xattr.h>

static void
copyxattr(const char *source, const char *target)
{
	int i, j;
	char *exclude, *portage_xattr_exclude;  /* strings of excluded xattr names              */
	int len;                                /* length of the string of excluded xattr names */
	ssize_t lsize, xsize;   /* size in bytes of the list of xattrs and the values           */
	char *lxattr, *value;   /* string of xattr names and the values                         */

	lsize = listxattr(source, 0, 0);
	lxattr = (char *)malloc(lsize);
	listxattr(source, lxattr, lsize);

	portage_xattr_exclude = getenv("PORTAGE_XATTR_EXCLUDE");
	if (portage_xattr_exclude == NULL)
		exclude = strdup("security.* system.nfs4_acl");
	else
		exclude = strdup(portage_xattr_exclude);

	len = strlen(exclude);

	/* We convert exclude[] to an array of concatenated null   */
	/* terminated strings.  lxattr[] is already such an array. */
	for (i = 0; i < len; i++)
		if (isspace(exclude[i]))
			exclude[i] = 0;

	i = 0 ;
	while(1) {
		while (lxattr[i++] == 0);
		if (i >= lsize)
			break;

		j = 0 ;
		while(1) {
			while (exclude[j++] == 0);
			if (j >= len)
				break;
			if (!fnmatch(&exclude[j-1], &lxattr[i-1], 0))
				goto skip;
			while (exclude[j++] != 0);
			if (j >= len)
				break;
		}

		xsize = getxattr(source, &lxattr[i-1], 0, 0);
		if (xsize != -1) {
			value = (char *)malloc(xsize);
			memset(value, 0, xsize);
			getxattr(source, &lxattr[i-1], value, xsize);
			setxattr(target, &lxattr[i-1], value, xsize, 0);
			free(value);
		}

	skip:
		while (lxattr[i++] != 0);
		if (i >= lsize)
			break;
	}

	free(lxattr);
	free(exclude);

	return;
}


static char *
which(char *mydir)
{
	char *mycandir = realpath(mydir, NULL);  /* canonical value of argv[0]'s dirname */
	char *path, *env_path = getenv("PATH");  /* full $PATH string                    */

	/* If we don't have $PATH in our environment, then pick a sane path. */
	if (env_path == NULL)
		path = strdup("/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin");
	else
		path = strdup(env_path);

	char *dir;       /* one directory in the $PATH string */
	char *candir ;   /* canonical value of that directory */
	char *file;      /* file name = path + "/install"     */
	char *savedptr;  /* reentrant context for strtok_r()  */

	struct stat s;

	dir = strtok_r(path, ":", &savedptr);

	while (dir) {
		candir = realpath(dir, NULL);

		/* You don't get a canonical path for ~/bin etc., so skip. */
		if (!candir)
			goto skip;

		/* If argv[0]'s canonical dirname == the path's canonical dirname, then we  */
		/* skip this path otheriwise we get into an infinite self-invocation.       */
		if ( !fnmatch(mycandir, candir, 0) )
			goto skip;

		if (asprintf(&file,"%s/%s", candir, "install") == -1)
			abort();

		/* If the file exists and is either a regular file or sym link, */
		/* assume we found the system's install. */
		if (stat(file, &s) == 0)
			if (S_ISREG(s.st_mode) || S_ISLNK(s.st_mode)) {
				free(candir);
				free(mycandir);
				free(path);
				return strdup(file);
			}

		free(file);

	skip:
		free(candir);
		dir = strtok_r(NULL, ":", &savedptr);
	}

	/* If we got here, then we didn't find install in the path */
	abort();
}



int
main(int argc, char* argv[], char* envp[])
{
	int i ;
	int status ;                   /* exit status of child "install" process                       */

	int opts_directory = 0;        /* if -d was given, then all arguments are directories          */
	int opts_target_directory = 0; /* if -t was given, then we're installing to a target directory */
	int target_is_directory = 0;   /* is the target a directory?                                   */

	int first, last;               /* argv indices of the first file/directory and last            */
	char *target;                  /* the target file or directory                                 */
	char *path;                    /* path to the target file                                      */
	char *install;                 /* path to the system install                                   */

	struct stat s;                 /* test if a file is a regular file or a directory              */


	opterr = 0; /* we skip many legitimate arguments, so silence any warning */

	while (1) {
		static struct option long_options[] = {
			{           "directory",       no_argument, 0, 'd'},
			{    "target-directory", required_argument, 0, 't'},
			{                     0,                 0, 0,  0 }
		};

		int option_index = 0;

		int c = getopt_long(argc, argv, "dt:", long_options, &option_index);
 
		if (c == -1)
			break;

		switch (c) {
			case  0 :
			case '?':
				break;

			case 'd':
				opts_directory = 1 ;
				break;

			case 't':
				opts_target_directory = 1 ;
				if (asprintf(&target, "%s", optarg) == -1)
					abort();
				break;

			default:
				abort();
		}
	}

	first = optind;
	last = argc-1;

	switch (fork()) {
		case -1:
			abort();

		case 0:
			install = which(dirname(argv[0]));
			execve(install, argv, envp);
			free(install);
			break;

		default:
			wait(&status);

			/* If all the targets are directories, do nothing. */
			if (opts_directory)
				return EXIT_SUCCESS;

			if (!opts_target_directory) {
				target = strdup(argv[last]);
				if (stat(target, &s) != 0)
					return EXIT_FAILURE;
				target_is_directory = s.st_mode & S_IFDIR;
			} else {
				/* target was set above with the -t option */
				target_is_directory = 1;
			}

			if (target_is_directory) {
				/* If -t was passed, then the last argv *is* */
				/* a file, so we include it for copyxattr(). */
				if (opts_target_directory)
					last++;

				for (i = first; i < last; i++) {
					/* We reproduce install's behavior and skip  */
					/* all extra directories on the command line */
					/* that are not the final target directory.  */
					stat(argv[i], &s);
					if (S_ISDIR(s.st_mode))
						continue;
					if (asprintf(&path, "%s/%s", target, argv[i]) == -1)
						abort();

					copyxattr(argv[i], path);

					free(path);
				}
			} else
				copyxattr(argv[first], target);

			free(target);
	}

	return EXIT_SUCCESS;
}
