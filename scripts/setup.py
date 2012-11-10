#!/usr/bin/env python

import os
from distutils.core import setup, Extension

ptpax = os.getenv('PTPAX')
xtpax = os.getenv('XTPAX')

if ptpax != None and xtpax == None:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['elf'],
		undef_macros = ['XTPAX'],
		define_macros = [('PTPAX', 1)]
	)

elif ptpax == None and xtpax != None:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['attr'],
		undef_macros = ['PTPAX'],
		define_macros = [('PTPAX', 1)]
	)

if ptpax != None and xtpax != None:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['elf', 'attr'],
		define_macros = [('PTPAX', 1), ('XTPAX', 1)]
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
