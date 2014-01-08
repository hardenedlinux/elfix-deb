#!/bin/bash

getfattr -d a b c 2>/dev/null
getfattr -d d/*   2>/dev/null
getfattr -d x y z 2>/dev/null
