#!/usr/bin/env python
#
#	setup.py: this file is part of the elfix package
#	Copyright (C) 2011, 2012  Anthony G. Basile
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


import sys, os, re
from distutils.core import setup, Extension

ptpax = os.getenv('PTPAX')
xtpax = os.getenv('XTPAX')

# This is a bit hacky since we include gelf.h but
# the pax decls are in elf.h.  The stacking goes as
#	gelf.h
#		libelf.h
#			elf.h


# we want only XTPAX and so NEED_PAX_DECLS
if ptpax == None and xtpax != None:
	module1 = Extension(
		name='pax',
		sources = ['paxmodule.c'],
		libraries = ['attr'],
		undef_macros = ['PTPAX'],
		define_macros = [('XTPAX', 1), ('NEED_PAX_DECLS', 1)]
	)

# We want PTPAX but don't know if we NEED_PAX_DECLS
else:
	try:
		need_pax_decls = True
		f = open('/usr/include/elf.h', 'r')
		for line in f.readlines():
			if re.search('PF_PAGEEXEC', line):
				need_pax_decls = False
		f.close()

	except IOError as e:
		print("Can't find elf.h in the usual place!")
		sys.exit(1)

	# We NEED_PAX_DECLS
	if need_pax_decls:

		# We want PTPAX but not XTPAX
		if ptpax != None and xtpax == None:
			module1 = Extension(
				name='pax',
				sources = ['paxmodule.c'],
				libraries = ['elf'],
				undef_macros = ['XTPAX'],
				define_macros = [('PTPAX', 1), ('NEED_PAX_DECLS', 1)]
			)

		# We want both PTAPX and XTPAX
		elif ptpax != None and xtpax != None:
			module1 = Extension(
				name='pax',
				sources = ['paxmodule.c'],
				libraries = ['elf', 'attr'],
				define_macros = [('PTPAX', 1), ('XTPAX', 1), ('NEED_PAX_DECLS', 1)]
			)

	# We don't NEED_PAX_DECLS
	else:

		# We want both PTAPX and XTPAX
		if ptpax != None and xtpax == None:
			module1 = Extension(
				name='pax',
				sources = ['paxmodule.c'],
				libraries = ['elf'],
				undef_macros = ['XTPAX', 'NEED_PAX_DECLS'],
				define_macros = [('PTPAX', 1)]
			)

		# We want both PTAPX and XTPAX
		elif ptpax != None and xtpax != None:
			module1 = Extension(
				name='pax',
				sources = ['paxmodule.c'],
				libraries = ['elf', 'attr'],
				undef_macros = ['NEED_PAX_DECLS'],
				define_macros = [('PTPAX', 1), ('XTPAX', 1)]
			)


setup(
	name = 'PaxPython',
	version = '0.6.1',
	author = 'Anthony G. Basile',
	author_email = 'blueness@gentoo.org',
	url = 'http://dev.gentoo.org/~blueness/elfix',
	description = 'This is bindings between paxctl and python',
	license = 'GPL-2',
	ext_modules = [module1]
)
