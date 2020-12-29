import sys
import os

from operator import eq


def gen_payload_impl(name):
    source_lines = []
    payload_name = os.path.basename(name).split('.')[0].lower()
    if not payload_name.startswith('payload_'):
        payload_name = 'payload_' + payload_name
    
    with open(name, 'rb') as fbin:
        fbytes = fbin.read()

    source_lines.append('const unsigned char %s[%i] =\n' % (payload_name, len(fbytes)))
    source_lines.append('\t{')

    for i, b in enumerate(fbytes):
        if i % 16 == 0:
            source_lines[-1] += '\n'
            source_lines.append('\t\t')

        source_lines[-1] += '0x%02x, ' % b
        if i == len(fbytes) - 1:
            source_lines[-1] += '\n'

    source_lines.append('\t};\n')
    return payload_name, len(fbytes), source_lines


def gen_payload_getter(names, sizes):
    source_lines = \
        [
            '#include "libpayload_int.h"\n',
            '#include <stdlib.h>\n',
            '\n',
            'struct payload *get_payload(PAYLOAD_T p)\n',
            '{\n',
            '\tstruct payload *res;\n',
            '\tconst unsigned char *pl;\n',
            '\tint len;\n',
            '\n',
            '\tswitch(p)\n',
            '\t{\n'
        ]

    for name, size in zip(names, sizes):
        source_lines.append('\t\tcase %s:\n' % name.upper())
        source_lines.append('\t\t\tpl = %s;\n' % name)
        source_lines.append('\t\t\tlen = %i;\n' % int(size))
        source_lines.append('\t\t\tbreak;\n')
        source_lines.append('\n')

    footer = \
        [
            '\t\tdefault:\n',
            '\t\t\treturn NULL;\n',
            '\t}\n',
            '\n',
            '\tres = malloc(sizeof(struct payload));\n',
            '\tif(res == NULL) return NULL;\n',
            '\n',
            '\tres->type = p;\n',
            '\tres->len = len;\n',
            '\tres->data = pl;\n',
            '\tres->install_base = -1ull;\n',
            '\tres->intf = -1;\n',
            '\tres->next = NULL;\n',
            '\tres->prev = NULL;\n',
            '\treturn res;\n',
            '}'
        ]

    source_lines.extend(footer)
    return source_lines


def gen_libpayload_int(names, sizes):
    header_lines = \
        [
            '#ifndef CHECKM8_TOOL_LIBPAYLOAD_INT_H\n',
            '#define CHECKM8_TOOL_LIBPAYLOAD_INT_H\n',
            '\n',
            '#include "libpayload.h"\n',
            "\n",
            'struct payload\n',
            '{\n',
            '\tPAYLOAD_T type;\n',
            '\tconst unsigned char *data;\n',
            '\tint len;\n',
            '\n',
            '\tunsigned long long install_base;\n',
            '\tunsigned short intf;\n',
            '\n',
            '\tstruct payload *next;\n',
            '\tstruct payload *prev;\n',
            '};\n',
            '\n',
            'struct payload *get_payload(PAYLOAD_T p);\n'
        ]

    name_lines = []
    size_lines = []

    for name, size in zip(names, sizes):
        name_lines.append('extern const unsigned char %s[%i];\n' % (name, int(size)))

    header_lines.extend(size_lines)
    header_lines.append('\n')
    header_lines.extend(name_lines)

    header_lines.append('\n')
    header_lines.append('#endif //CHECKM8_TOOL_LIBPAYLOAD_INT_H\n')
    return header_lines


def gen_libpayload_pub(names):
    header_lines = \
        [
            '#ifndef CHECKM8_TOOL_LIBPAYLOAD_H\n',
            '#define CHECKM8_TOOL_LIBPAYLOAD_H\n',
            '\n',
            'typedef enum\n',
            '{\n',
            '\tPAYLOAD_BUILTIN = -1,\n'
        ]

    for name in names:
        header_lines.append('\t%s,\n' % name.upper())

    header_lines.append('} PAYLOAD_T;\n')
    header_lines.append('\n')
    header_lines.append('#endif //CHECKM8_TOOL_LIBPAYLOAD_H\n')
    return header_lines


def gen_and_update(fname, linefunc, args, force_lines=None):
    if force_lines is None:
        lines = linefunc(*args)
    else:
        lines = force_lines

    if os.path.exists(fname):
        with open(fname, 'r') as f:
            old_lines = f.readlines()

        if all(map(eq,
                   [l.strip() for l in lines],
                   [l.strip() for l in old_lines])):
            return

    with open(fname, 'w+') as f:
        f.writelines(lines)


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

    payload_names = []
    payload_sizes = []

    for name in bin_names:
        pname, size, pl_lines = gen_payload_impl(name)
        payload_names.append(pname)
        payload_sizes.append(size)

        gen_and_update(lib_dir + '/c/' + pname + '.c', None, None, force_lines=pl_lines)

    gen_and_update(lib_dir + '/c/get_payloads.c', gen_payload_getter, [payload_names, payload_sizes])
    gen_and_update(lib_dir + '/../include/libpayload_int.h', gen_libpayload_int, [payload_names, payload_sizes])
    gen_and_update(lib_dir + '/../include/libpayload.h', gen_libpayload_pub, [payload_names])
