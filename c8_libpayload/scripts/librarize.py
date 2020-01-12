import sys
import os

from collections import defaultdict
from operator import eq

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: librarize.py [bin names ...] [lib dir]')
        exit(1)

    bin_names = []
    lib_dir = os.path.abspath(sys.argv[-1])

    if os.path.isdir(sys.argv[1]):
        bin_folder = os.path.abspath(sys.argv[1])
        for bin_fname in os.listdir(bin_folder):
            bin_names.append(bin_folder + '/' + bin_fname)
    else:
        for n in sys.argv[1:-1]:
            bin_names.append(os.path.abspath(n))

    source_lines = defaultdict(list)
    header_lines = ['#ifndef CHECKM8_TOOL_LIBPAYLOAD_H\n',
                    '#define CHECKM8_TOOL_LIBPAYLOAD_H\n',
                    '\n']

    for n in bin_names:
        payload_name = os.path.basename(n).split('.')[0]
        with open(n, 'rb') as fbin:
                fbytes = fbin.read()

        header_lines.append('extern const unsigned char %s[%i];\n' % (payload_name, len(fbytes)))

        source_lines[payload_name].append('#include "libpayload.h"\n')
        source_lines[payload_name].append('\n')
        source_lines[payload_name].append('const unsigned char %s[%i] =\n' % (payload_name, len(fbytes)))
        source_lines[payload_name].append('\t{')

        for i, b in enumerate(fbytes):
            if i % 16 == 0:
                source_lines[payload_name][-1] += '\n'
                source_lines[payload_name].append('\t\t')

            source_lines[payload_name][-1] += '0x%02x, ' % b
            if i == len(fbytes) - 1:
                source_lines[payload_name][-1] += '\n'

        source_lines[payload_name].append('\t};\n')

    header_lines.append('\n')
    header_lines.append('#endif //CHECKM8_TOOL_LIBPAYLOAD_H\n')

    files_updated = False
    for sname, lines in source_lines.items():
        sfname = lib_dir + '/' + sname + '.c'

        if os.path.exists(sfname):
            with open(sfname, 'r') as f:
                old_lines = f.readlines()

            if all(map(eq, lines, old_lines)):
                continue

        with open(sfname, 'w+') as f:
            files_updated = True
            f.writelines(lines)

    if files_updated:
        with open(lib_dir + '/libpayload.h', 'w+') as f:
            f.writelines(header_lines)