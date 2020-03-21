import gdb

def val_from_sym(name):
    print 'getting value for %s' % name

    try:
        if name == 'wzr' or name == 'xzr':
            return '0'

        elif name[0] == '#':
            return name

        elif name[0] in ['x', 'w']:
            return '0x{:x}'.format(int(gdb.selected_frame().read_register(name)))

        else:
            return None

    except:
        return 'val?'


class Profile(gdb.Command):
    def __init__(self):
        super(Profile, self).__init__("profile", gdb.COMMAND_USER)

    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)
        if len(argv) != 2:
            raise gdb.GdbError("Usage: profile [fname] [end_addr]")

        arch = gdb.selected_frame().architecture()
        instr_type = gdb.lookup_type("unsigned int").pointer()

        next_dest = None
        stack = []

        outfile = open(argv[0], "a+")
        end_addr = int(argv[1], 16)

        while True:
            if next_dest is not None:
                outfile.write('\tdest %s\n' % val_from_sym(next_dest))
                next_dest = None

            addr = gdb.selected_frame().read_register("pc")
            if int(str(addr), 16) == end_addr:
                print 'goodbye!'
                break

            instr = arch.disassemble(int(str(addr), 16))[0]['asm']
            if instr == '.inst\t0x00000000 ; undefined':
                break

            instr_spl = instr.split()

            mnem = instr_spl[0]
            dest = None
            arg1 = None
            arg2 = None

            outfile.write('\n%s\t%s\n' % (addr, instr))
            if len(instr_spl) > 1:
                dest = instr_spl[1].strip(',')

            if len(instr_spl) > 2:
                arg1 = instr_spl[2].strip(',')

            if len(instr_spl) > 3:
                arg2 = instr_spl[3].strip(',')

            if mnem == 'bl' or mnem == 'blr':
                outfile.write('\tentering %s\n' % dest)
                stack.append(dest)

                outfile.write('\targs: [')
                for reg in ['x0', 'x1', 'x2', 'x3', 'x4', 'x5', 'x6', 'x7']:
                    outfile.write('%s, ' % val_from_sym(reg))
                outfile.write(']\n')

            elif mnem == 'ret':
                if len(stack) > 0:
                    outfile.write('\tfinished %s\n' % stack.pop())
                else:
                    outfile.write('\tfinished ??\n')

                outfile.write('\tretval %s\n' % val_from_sym('x0'))

            elif mnem == 'ldr' or mnem == 'ldp':
                outfile.write('\tdest %s\n' % val_from_sym(dest))
                if mnem == 'ldp':
                    outfile.write('\tdest %s\n' % val_from_sym(arg1))

            elif mnem == 'str' or mnem == 'stp':
                outfile.write('\targ1 %s\n' % val_from_sym(dest))
                if mnem == 'stp':
                    outfile.write('\targ2 %s\n' % val_from_sym(arg1))

            else:
                if dest is not None and dest[0] in ['x', 'w']:
                    next_dest = dest

                if arg1 is not None:
                    val = val_from_sym(arg1)
                    if val is not None:
                        outfile.write('\targ1 %s\n' % val)

                if arg2 is not None:
                    val = val_from_sym(arg2)
                    if val is not None:
                        outfile.write('\targ2 %s\n' % val)

            gdb.execute("stepi", to_string=False)
        outfile.close()

Profile()
