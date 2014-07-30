#!/bin/sh

aclocal && \
autoheader && \
autoconf && \
automake --add-missing --copy

cd doc
./make.sh
