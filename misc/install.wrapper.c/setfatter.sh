#!/bin/bash
touch a b c
setfattr -n user.pax.flags -v "mr" a
setfattr -n user.blah -v "hithere" a
setfattr -n user.pax.flags -v p b
setfattr -n user.pax.flags -v r c
