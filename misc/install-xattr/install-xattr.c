/* Copyright 2014 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 *
 * Wrapper for coreutil's install to preserve extended attributes.
 *
 * Copyright 2014 Anthony G. Basile - <blueness@gentoo.org>
 * Copyright 2014 Mike Frysinger    - <vapier@gentoo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

static char *
xstrdup(const char *s)
{
	char *ret = strdup(s);
	if (ret == NULL)
		err(1, "strdup() failed");
	return ret;
}

static void *
xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (ret == NULL)
		err(1, "malloc() failed");
	return ret;
}

static void *
xrealloc(void *p, size_t size)
{
	void *ret = realloc(p, size);
	if (ret == NULL)
		err(1, "realloc() failed");
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
	char *lxattr;                  /* string of xattr names                       */
	static char *value = NULL ;    /* string of an xattr name's value             */
	static size_t value_size = 0;  /* size of the value string                    */

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
		if (xsize > value_size) {
			value_size = xsize;
			value = xrealloc(value, value_size);
		}
		xgetxattr(source, &lxattr[i-1], value, xsize);
		xsetxattr(target, &lxattr[i-1], value, xsize);

 skip:
		while (lxattr[i++] != 0)
			continue;
		if (i >= lsize)
			break;
	}

	/* No need to free(value) on return because its static and we */
	/* just keep reusing the same allocated memory on each call.  */
	free(lxattr);
}


static char *
which(char *mypath, char *portage_helper_path)
{
	/* $PATH for system install */
	char *path = NULL, *env_path = getenv("PATH");

	/* If we don't have $PATH in our environment, then pick a sane path. */
	if (env_path == NULL) {
		size_t len = confstr(_CS_PATH, 0, 0);
		path = xmalloc(len);
		confstr(_CS_PATH, path, len);
	} else
		path = xstrdup(env_path);

	char *dir;       /* one directory in the colon delimited $PATH string */
	char *canpath;   /* candidate install's canonical path                */
	char *savedptr;  /* reentrant context for strtok_r()                  */

	dir = strtok_r(path, ":", &savedptr);

	while (dir) {
		char *canfile = path_join(dir, "install");
		canpath = realpath(canfile, NULL);
		free(canfile);

		/* ignore invalid paths that cannot be canonicalized */
		if (!canpath)
			goto skip;

		/* If argv[0]'s canonical path == candidate install's canonical path,
		 * then we skip this path otheriwise we get into an infinite self-invocation.
		 */
		if (!strcmp(mypath, canpath))
			goto skip;

		/* If portage install's canonical path == candidate install's canonical path,
		 * then we skip this path otheriwise we get into an infinite self-invocation.
		 */
		if (portage_helper_path)
			if (!strcmp(portage_helper_path, canpath))
				goto skip;

		/* If the canpath exists and is either a regular file or sym link,
		 * assume we found the system's install.
                 */
		struct stat s;

		if (stat(canpath, &s) == 0)
			if (S_ISREG(s.st_mode)) {
				free(path);
				return canpath;
			}


 skip:
		free(canpath);
		dir = strtok_r(NULL, ":", &savedptr);
	}

	if (env_path == NULL)
		err(1, "failed to find 'install' in standard utilities path");
	else
		err(1, "failed to find 'install' in PATH=%s", env_path);
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

	char *mypath = realpath("/proc/self/exe", NULL); /* path to argv[0]                            */
	char *install;                                   /* path to the system install                 */

	struct stat s;                 /* test if a file is a regular file or a directory              */

	char *portage_xattr_exclude;   /* strings of excluded xattr names from $PORTAGE_XATTR_EXCLUDE  */

	portage_xattr_exclude = getenv("PORTAGE_XATTR_EXCLUDE");
	if (portage_xattr_exclude == NULL)
		exclude = xstrdup("btrfs.* security.* trusted.* system.nfs4_acl");
	else
		exclude = xstrdup(portage_xattr_exclude);

	len_exclude = strlen(exclude);

	/* We convert exclude[] to an array of concatenated NUL terminated
	 * strings.  Also, no need to free(exclude) before we exit().
	 */
	char *p = exclude;
	char *pend = p + len_exclude;
	while (p != pend) {
		if (isspace(*p))
			*p = '\0';
		p++;
	}

	opterr = 0; /* we skip many legitimate flags, so silence any warning */

	while (1) {
		static struct option long_options[] = {
			{           "directory",       no_argument, 0, 'd'},
			{    "target-directory", required_argument, 0, 't'},
			{               "group", required_argument, 0, 'g'},
			{                "mode", required_argument, 0, 'm'},
			{               "owner", required_argument, 0, 'o'},
			{              "suffix", required_argument, 0, 'S'},
			{             "context", optional_argument, 0, 'Z'},
			{              "backup", optional_argument, 0, 'b'},
			{                "help",       no_argument, 0,  0 },
			{                     0,                 0, 0,  0 }
		};

		int option_index;
		int c = getopt_long(argc, argv, "dt:g:m:o:S:Z:", long_options, &option_index);

 
		if (c == -1)
			break;

		switch (c) {
			case 0:
			case 'g':
			case 'm':
			case 'o':
			case 'S':
			case 'Z':
			case 'b':
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

	/* Do we need to chdir to OLDPWD?  This is required when we are called my a
	 * wrapper like ${__PORTAGE_HELPER_PATH} which then passes its directory as
	 * $PWD and the source directory from which it was called as $OLDPWD.  But
	 * we want the system install to run in the source directory, ie $OLDPWD,
	 * so we chdir to it.  Currently we assume that if __PORTAGE_HELPER_PATH
	 * is set, then we chdir to oldpwd.
	 */
	char *oldpwd = getenv("OLDPWD");
	char *portage_helper_path = getenv("__PORTAGE_HELPER_PATH");
	char *portage_helper_canpath = NULL;
	if (portage_helper_path)
		chdir(oldpwd);

	switch (fork()) {
		case -1:
			err(1, "fork() failed");

		case 0:
			/* find system install avoiding mypath and portage_helper_path! */
			if (portage_helper_path)
				portage_helper_canpath = realpath(portage_helper_path, NULL);
			install = which(mypath, portage_helper_canpath);
			free(mypath);
			free(portage_helper_canpath);
			argv[0] = install;        /* so coreutils' lib/program.c behaves  */
			execv(install, argv);     /* The kernel will free(install).       */
			err(1, "execv() failed");

		default:
			wait(&status);

			/* Are there enough files/directories on the cmd line to
			 * proceed?  This can happen if install is called with no
			 * arguments or with just --help.  In which case there is
			 * nothing for the parent to do.
                         */
			if (first >= last)
				goto done;

			/* If all the targets are directories, do nothing. */
			if (opts_directory)
				goto done;

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

					path = path_join(target, basename(argv[i]));
					copyxattr(argv[i], path);
					free(path);
				}
			} else
				copyxattr(argv[first], target);


 done:
			/* Do the right thing and pass the signal back up.  See:
			 * http://www.cons.org/cracauer/sigint.html
			 */
			if (WIFSIGNALED(status)) {
				int signum = WTERMSIG(status);
				kill(getpid(), signum);
				return 128 + signum;
			} else if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return EXIT_FAILURE; /* We should never get here. */

	}
}
