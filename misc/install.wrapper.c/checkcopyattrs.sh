#!/bin/bash
set -e

touch a b c
mkdir -p d e
setfattr -n user.foo -v "bar" a
setfattr -n user.pax.flags -v "mr" a
setfattr -n user.pax.flags -v "p" b
setfattr -n user.pax.flags -v "r" c

./install-xattr a x
./install-xattr b y
./install-xattr c z

[ "$(getfattr --only-values -n user.foo x)" == "bar" ]
[ "$(getfattr --only-values -n user.pax.flags x)" == "mr" ]
[ "$(getfattr --only-values -n user.pax.flags y)" == "p" ]
[ "$(getfattr --only-values -n user.pax.flags z)" == "r" ]

./install-xattr a b c d

[ "$(getfattr --only-values -n user.foo d/a)" == "bar" ]
[ "$(getfattr --only-values -n user.pax.flags d/a)" == "mr" ]
[ "$(getfattr --only-values -n user.pax.flags d/b)" == "p" ]
[ "$(getfattr --only-values -n user.pax.flags d/c)" == "r" ]

./install-xattr -t e a b c

[ "$(getfattr --only-values -n user.foo e/a)" == "bar" ]
[ "$(getfattr --only-values -n user.pax.flags e/a)" == "mr" ]
[ "$(getfattr --only-values -n user.pax.flags e/b)" == "p" ]
[ "$(getfattr --only-values -n user.pax.flags e/c)" == "r" ]

./install-xattr a -D f/a
[ "$(getfattr --only-values -n user.foo f/a)" == "bar" ]
[ "$(getfattr --only-values -n user.pax.flags f/a)" == "mr" ]
