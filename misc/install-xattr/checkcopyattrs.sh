#!/bin/bash
set -e

touch a b c
mkdir -p d e f
setfattr -n user.foo -v "bar" a
setfattr -n user.bas -v "x" a
setfattr -n user.pax.flags -v "mr" a
setfattr -n user.pax.flags -v "p" b
setfattr -n user.pax.flags -v "r" c

./install-xattr a x
./install-xattr b y
./install-xattr c z

[ "$(getfattr --only-values -n user.foo x)" == "bar" ]
[ "$(getfattr --only-values -n user.bas x)" == "x" ]
[ "$(getfattr --only-values -n user.pax.flags x)" == "mr" ]
[ "$(getfattr --only-values -n user.pax.flags y)" == "p" ]
[ "$(getfattr --only-values -n user.pax.flags z)" == "r" ]

./install-xattr a b c d

[ "$(getfattr --only-values -n user.foo d/a)" == "bar" ]
[ "$(getfattr --only-values -n user.bas d/a)" == "x" ]
[ "$(getfattr --only-values -n user.pax.flags d/a)" == "mr" ]
[ "$(getfattr --only-values -n user.pax.flags d/b)" == "p" ]
[ "$(getfattr --only-values -n user.pax.flags d/c)" == "r" ]

# This tests if the src file was inside a directory
# the correct dst location should be f/a. NOT f/d/a.
./install-xattr d/a f

[ -x f/a ]
[ ! -x f/d/a ]
[ "$(getfattr --only-values -n user.foo f/a)" == "bar" ]
[ "$(getfattr --only-values -n user.bas f/a)" == "x" ]

./install-xattr -t e a b c

[ "$(getfattr --only-values -n user.foo e/a)" == "bar" ]
[ "$(getfattr --only-values -n user.bas e/a)" == "x" ]
[ "$(getfattr --only-values -n user.pax.flags e/a)" == "mr" ]
[ "$(getfattr --only-values -n user.pax.flags e/b)" == "p" ]
[ "$(getfattr --only-values -n user.pax.flags e/c)" == "r" ]

./install-xattr a -D f/a
[ "$(getfattr --only-values -n user.foo f/a)" == "bar" ]
[ "$(getfattr --only-values -n user.bas f/a)" == "x" ]
[ "$(getfattr --only-values -n user.pax.flags f/a)" == "mr" ]


# The following are just tests to make sure the raw install
# options don't get lost in our optargs parsing.
# See: https://bugs.gentoo.org/show_bug.cgi?id=465000#c57
# These should all silently succeed.

./install-xattr --backup=off a backup-a
./install-xattr --backup=numbered a backup-a
./install-xattr --backup=existing a backup-a
./install-xattr --backup=simple a backup-a
./install-xattr --backup a backup-a
./install-xattr -b a backup-a
./install-xattr -C a backup-a
./install-xattr -p a backup-a
./install-xattr -d g/g/g

./install-xattr -o $(id -u) a mode-a
./install-xattr -g $(id -g) a mode-a
./install-xattr -m 666 a mode-a

# Let's abuse ourselves
./install-xattr -s install-xattr target-install-xattr
[[ -x /usr/bin/sstrip ]] && ./install-xattr -s --strip-program=/usr/bin/sstrip install-xattr target-install-xattr

./install-xattr -T a target-a
./install-xattr --help >/dev/null
./install-xattr --version >/dev/null

#       -S, --suffix=SUFFIX
#              override the usual backup suffix
#
#       --preserve-context
#              preserve SELinux security context
#
#       -Z, --context=CONTEXT
#              set SELinux security context of files and directories

# Okay, let's clean up after ourselves
rm -rf a b c d e f g x y z backup-a* mode-a target-a target-install-xattr
