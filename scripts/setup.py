#!/usr/bin/env python

import os
from distutils.core import setup, Extension

xattr = os.getenv('XATTR')

if xattr != None:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['elf', 'attr'],
		define_macros = [('XATTR', None)]
	)
else:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['elf'],
		undef_macros = ['XATTR']
	)

setup(
	name = 'PaxPython',
	version = '1.0',
	author = 'Anthony G. Basile',
	author_email = 'blueness@gentoo.org',
	url = 'http://dev.gentoo.org/~blueness/elfix',
	description = 'This is bindings between paxctl and python',
	license = 'GPL-2',
	ext_modules = [module1]
)
