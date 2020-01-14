import sys
import os

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: headerize.py [lib names ...] [header dir]')
        exit(1)

    lib_names = []
    hdr_dir = os.path.abspath(sys.argv[-1])

    for libarg in sys.argv[1:-1]:
        if os.path.isdir(libarg):
            lib_folder = os.path.abspath(libarg)
            for lib_fname in os.listdir(lib_folder):
                lib_names.append(lib_folder + '/' + lib_fname)
        else:
            lib_names.append(os.path.abspath(libarg))

    header_lines = ['#ifndef CHECKM8_TOOL_LIBPAYLOAD_H\n',
                    '#define CHECKM8_TOOL_LIBPAYLOAD_H\n',
                    '\n']

    name_lines = []
    size_lines = []

    for n in lib_names:
        with open(n, 'r') as f:
            line = f.readline() # looks like "const unsigned char PAYLOAD_NAME[PAYLOAD_SIZE] = "
            name = line.split(' ')[3].split('[')[0]
            size = line.split(' ')[3].split('[')[1][:-1]

            name_lines.append('extern const unsigned char %s[%s_SZ];\n' % (name, name.upper()))
            size_lines.append('#define %s_SZ %s\n' % (name.upper(), size))

    header_lines.extend(size_lines)
    header_lines.append('\n')
    header_lines.extend(name_lines)

    header_lines.append('\n')
    header_lines.append('#endif //CHECKM8_TOOL_LIBPAYLOAD_H\n')

    with open(hdr_dir + '/libpayload.h', 'w+') as f:
        f.writelines(header_lines)