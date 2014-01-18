/* Copyright 2014 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 *
 * Copyright 2014 Anthony G. Basile - <blueness@gentoo.org>
 *
 * Wrapper for coreutil's install to preserve extended attributes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <fnmatch.h>
#include <ctype.h>
#include <getopt.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/xattr.h>

static void *
xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (ret == NULL)
		err(1, "malloc() failed");
	return ret;
}

static char *
path_join(const char *path, const char *file)
{
	size_t len_path = strlen(path);
	size_t len_file = strlen(file);
	char *ret = xmalloc(len_path + len_file + 2);

	memcpy(ret, path, len_path);
	ret[len_path] = '/';
	memcpy(ret + len_path + 1, file, len_file);
	ret[len_path + len_file + 1] = '\0';

	return ret;
}

static ssize_t
xlistxattr(const char *path, char *list, size_t size)
{
	ssize_t ret = listxattr(path, list, size);
	if (ret < 0)
		err(1, "listxattr() failed");
	return ret;
}

static ssize_t
xgetxattr(const char *path, char *list, char *value, size_t size)
{
	ssize_t ret = getxattr(path, list, value, size);
	if (ret < 0)
		err(1, "getxattr() failed");
	return ret;
}

static ssize_t
xsetxattr(const char *path, char *list, char *value, size_t size)
{
	ssize_t ret = setxattr(path, list, value, size, 0);
	if (ret < 0)
		err(1, "setxattr() failed");
	return ret;
}

char *exclude;      /* strings of excluded xattr names              */
size_t len_exclude; /* length of the string of excluded xattr names */

static void
copyxattr(const char *source, const char *target)
{
	ssize_t i, j;           /* counters to walk through strings                   */
	ssize_t lsize, xsize;   /* size in bytes of the list of xattrs and the values */
	char *lxattr, *value;   /* string of xattr names and the values               */

	lsize = xlistxattr(source, NULL, 0);
	lxattr = xmalloc(lsize);
	xlistxattr(source, lxattr, lsize);

	i = 0;
	while (1) {
		while (lxattr[i++] == 0)
			continue;
		if (i >= lsize)
			break;

		j = 0;
		while (1) {
			while (exclude[j++] == 0)
				continue;
			if (j >= len_exclude)
				break;
			if (!fnmatch(&exclude[j - 1], &lxattr[i - 1], 0))
				goto skip;
			while (exclude[j++] != 0)
				continue;
			if (j >= len_exclude)
				break;
		}

		xsize = xgetxattr(source, &lxattr[i-1], 0, 0);
		if (xsize > 0) {
			value = xmalloc(xsize);
			xgetxattr(source, &lxattr[i-1], value, xsize);
			xsetxattr(target, &lxattr[i-1], value, xsize);
			free(value);
		}

 skip:
		while (lxattr[i++] != 0)
			continue;
		if (i >= lsize)
			break;
	}

	free(lxattr);
}


static char *
which(const char *mydir)
{
	char *mycandir = realpath(mydir, NULL);  /* canonical value of argv[0]'s dirname */
	char *path, *env_path = getenv("PATH");  /* full $PATH string                    */

	/* If we don't have $PATH in our environment, then pick a sane path. */
	if (env_path == NULL) {
		size_t len = confstr(_CS_PATH, 0, 0);
		path = xmalloc(len);
		confstr(_CS_PATH, path, len);
	} else
		path = strdup(env_path);

	char *dir;       /* one directory in the $PATH string */
	char *candir;    /* canonical value of that directory */
	char *file;      /* file name = path + "/install"     */
	char *savedptr;  /* reentrant context for strtok_r()  */

	struct stat s;

	dir = strtok_r(path, ":", &savedptr);

	while (dir) {
		candir = realpath(dir, NULL);

		/* ignore invalid paths that cannot be canonicalized */
		if (!candir)
			goto skip;

		/* If argv[0]'s canonical dirname == the path's canonical dirname, then we
		 * skip this path otheriwise we get into an infinite self-invocation.
		 */
		if (!strcmp(mycandir, candir))
			goto skip;

		file = path_join(candir, "install");

		/* If the file exists and is either a regular file or sym link,
		 * assume we found the system's install.
                 */
		if (stat(file, &s) == 0)
			if (S_ISREG(s.st_mode)) {
				free(candir);
				free(mycandir);
				free(path);
				return file;
			}

		free(file);

 skip:
		free(candir);
		dir = strtok_r(NULL, ":", &savedptr);
	}

	err(1, "failed to find system 'install'");
}



int
main(int argc, char* argv[])
{
	int i;
	int status;                    /* exit status of child "install" process                       */

	int opts_directory = 0;        /* if -d was given, then all arguments are directories          */
	int opts_target_directory = 0; /* if -t was given, then we're installing to a target directory */
	int target_is_directory = 0;   /* is the target a directory?                                   */

	int first, last;               /* argv indices of the first file/directory and last            */
	char *target;                  /* the target file or directory                                 */
	char *path;                    /* path to the target file                                      */
	char *install;                 /* path to the system install                                   */

	struct stat s;                 /* test if a file is a regular file or a directory              */

	char *portage_xattr_exclude;  /* strings of excluded xattr names from $PORTAGE_XATTR_EXCLUDE   */

	portage_xattr_exclude = getenv("PORTAGE_XATTR_EXCLUDE");
	if (portage_xattr_exclude == NULL)
		exclude = strdup("security.* system.nfs4_acl");
	else
		exclude = strdup(portage_xattr_exclude);

	len_exclude = strlen(exclude);

	/* We convert exclude[] to an array of concatenated NUL terminated
	 * strings.  Also, no need to free(exclude) before we exit().
	 */
	char *p = exclude;
	while ((p = strchr(p, ' ')))
		*p++ = '\0';

	opterr = 0; /* we skip many legitimate flags, so silence any warning */

	while (1) {
		static struct option long_options[] = {
			{           "directory",       no_argument, 0, 'd'},
			{    "target-directory", required_argument, 0, 't'},
			{                "help",       no_argument, 0,  0 },
			{                     0,                 0, 0,  0 }
		};

		int option_index;
		int c = getopt_long(argc, argv, "dt:", long_options, &option_index);
 
		if (c == -1)
			break;

		switch (c) {
			case 0:
			case '?':
				/* We skip the flags we don't care about */
				break;

			case 'd':
				opts_directory = 1;
				break;

			case 't':
				opts_target_directory = 1;
				target = optarg;
				break;

			default:
				err(1, "getopt_long() failed");
		}
	}

	first = optind;
	last = argc - 1;

	switch (fork()) {
		case -1:
			err(1, "fork() failed");

		case 0:
			install = which(dirname(argv[0]));
			argv[0] = install;    /* so coreutils' lib/program.c behaves */
			execv(install, argv); /* The kernel will free(install).      */
			err(1, "execv() failed");

		default:
			wait(&status);
			status = WEXITSTATUS(status);

			/* Are there enough files/directories on the cmd line to
			 * proceed?  This can happen if install is called with no
			 * arguments or with just --help.  In which case there is
			 * nothing the parent to do.
                         */
			if (first >= last)
				return status;

			/* If all the targets are directories, do nothing. */
			if (opts_directory)
				return status;

			if (!opts_target_directory) {
				target = argv[last];
				if (stat(target, &s) != 0)
					return EXIT_FAILURE;
				target_is_directory = S_ISDIR(s.st_mode);
			} else {
				/* target was set above with the -t option */
				target_is_directory = 1;
			}

			if (target_is_directory) {
				/* If -t was passed, then the last argv *is*
				 * a file, so we include it for copyxattr().
                                 */
				if (opts_target_directory)
					last++;

				for (i = first; i < last; i++) {
					if (stat(argv[i], &s) != 0)
						return EXIT_FAILURE;
					/* We reproduce install's behavior and skip
					 * all extra directories on the command line
					 * that are not the final target directory.
                                         */
					if (S_ISDIR(s.st_mode))
						continue;

					path = path_join(target, argv[i]);
					copyxattr(argv[i], path);
					free(path);
				}
			} else
				copyxattr(argv[first], target);

			return status;
	}

	/* We should never get here. */
	return EXIT_FAILURE;
}
