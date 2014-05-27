#!/usr/bin/env python

import sys
import re, glob
from optparse import OptionParser

from elftools import __version__
from elftools.common.exceptions import ELFError
from elftools.common.py3compat import bytes2str
from elftools.elf.elffile import ELFFile
from elftools.elf.dynamic import DynamicSection

class ReadElf(object):
    def __init__(self, file):
        """ file: stream object with the ELF file to read
        """
        self.elffile = ELFFile(file)

    def display_dynamic_dt_needed(self):
        """ Display the dynamic DT_NEEDED contained in the file
        """
        for section in self.elffile.iter_sections():
            if not isinstance(section, DynamicSection):
                continue

            for tag in section.iter_tags():
                if tag.entry.d_tag == 'DT_NEEDED':
                    sys.stdout.write('\t%s\n' % bytes2str(tag.needed) )


def ldpath(ld_so_conf='/etc/ld.so.conf'):
    """ Generate paths to search for libraries from ld.so.conf.  Recursively
        parse included files.  We assume correct syntax and the ld.so.cache
        is in sync with ld.so.conf.
    """
    with open(ld_so_conf, 'r') as path_file:
        lines = path_file.read()
    lines = re.sub('#.*', '', lines)                   # kill comments
    lines = list(re.split(':+|\s+|\t+|\n+|,+', lines)) # man 8 ldconfig

    include_globs = []
    for l in lines:
        if l == '':
           lines.remove('')
        if l == 'include':
            f = lines[lines.index(l) + 1]
            lines.remove(l)
            lines.remove(f)
            include_globs.append(f)

    include_files = []
    for g in include_globs:
        include_files = include_files + glob.glob('/etc/' + g)
    for c in include_files:
        lines = lines + ldpath(c)

    return list(set(lines))


SCRIPT_DESCRIPTION = 'Print shared library dependencies'
VERSION_STRING = '%%prog: based on pyelftools %s' % __version__

def main():
    optparser = OptionParser(
        usage='usage: %prog <elf-file>',
        description=SCRIPT_DESCRIPTION,
        add_help_option=False, # -h is a real option of readelf
        prog='ldd.py',
        version=VERSION_STRING)
    optparser.add_option('-h', '--help',
        action='store_true', dest='help',
        help='Display this information')
    options, args = optparser.parse_args()

    #if options.help or len(args) == 0:
        #optparser.print_help()
        #sys.exit(0)

    for f in args:
        with open(f, 'rb') as file:
            try:
                readelf = ReadElf(file)
                if len(args) > 1:
                    sys.stdout.write('%s : \n' % f)
                readelf.display_dynamic_dt_needed()
            except ELFError as ex:
                sys.stderr.write('ELF error: %s\n' % ex)
                sys.exit(1)

    lines = ldpath()
    print(lines)


if __name__ == '__main__':
    main()
