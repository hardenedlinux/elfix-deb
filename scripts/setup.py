#!/usr/bin/env python

import os
from distutils.core import setup, Extension

xattr = os.getenv('XTPAX')

if xattr != None:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['elf', 'attr'],
		define_macros = [('XTPAX', None)]
	)
else:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['elf'],
		undef_macros = ['XTPAX']
	)

setup(
	name = 'PaxPython',
	version = '2.0',
	author = 'Anthony G. Basile',
	author_email = 'blueness@gentoo.org',
	url = 'http://dev.gentoo.org/~blueness/elfix',
	description = 'This is bindings between paxctl and python',
	license = 'GPL-2',
	ext_modules = [module1]
)
