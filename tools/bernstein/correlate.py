import sys
import os
import numpy as np

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('usage: correlate [data path]')
        exit(1)

    # gather data
    count = []
    ttotal = []

    t = []
    tsq = []
    tnum = []

    for datafile in os.listdir(sys.argv[2]):
        t_new = np.empty((16, 256), dtype=np.uint64)
        tsq_new = np.empty((16, 256), dtype=np.uint64)
        tnum_new = np.empty((16, 256), dtype=np.uint64)

        with open(datafile, 'r') as f:
            split = f.readline().strip().split()
            count.append(int(split[0]))
            ttotal.append(int(split[0]))

            f.readline() # empty
            for line in f.readlines():
                split = list(map(int, line.strip().split()))

                t_new[split[0]][split[1]] = split[2]
                tsq_new[split[0]][split[1]] = split[3]
                tnum_new[split[0]][split[1]] = split[4]

        t.append(t_new)
        tsq.append(tsq_new)
        tnum.append(tnum_new)

    # divide along some axis
    # todo

    # reduce
    count = sum(count)
    ttotal = sum(ttotal)

    t = sum(t)
    tsq = sum(tsq)
    tnum = sum(tnum)