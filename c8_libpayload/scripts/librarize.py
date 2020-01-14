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

    for binarg in sys.argv[1:-1]:
        if os.path.isdir(binarg):
            bin_folder = os.path.abspath(binarg)
            for bin_fname in os.listdir(bin_folder):
                bin_names.append(bin_folder + '/' + bin_fname)
        else:
            bin_names.append(os.path.abspath(binarg))

    source_lines = defaultdict(list)
    for n in bin_names:
        payload_name = os.path.basename(n).split('.')[0]
        with open(n, 'rb') as fbin:
            fbytes = fbin.read()

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

    for sname, lines in source_lines.items():
        sfname = lib_dir + '/' + sname + '.c'

        if os.path.exists(sfname):
            with open(sfname, 'r') as f:
                old_lines = f.readlines()

            if all(map(eq, lines, old_lines)):
                continue

        with open(sfname, 'w+') as f:
            f.writelines(lines)
