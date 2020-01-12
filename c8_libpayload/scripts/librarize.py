import sys
from collections import defaultdict
import os

if __name__ == '__main__':
    print('ffffffffffffffffff')
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
                source_lines[payload_name].append('\n\t\t')

            source_lines[payload_name][-1] += '0x%02x, ' % b

        source_lines[payload_name].append('\n\t};\n')

    header_lines.append('\n')
    header_lines.append('#endif //CHECKM8_TOOL_LIBPAYLOAD_H\n')

    with open(lib_dir + '/libpayload.h', 'w+') as f:
        f.writelines(header_lines)

    for sname, lines in source_lines.items():
        with open(lib_dir + '/' + sname + '.c', 'w+') as f:
            f.writelines(lines)