import os
import sys
import numpy as np

def transform_upper(v):
    return v[:-2] + chr(min(ord(v[-2])+128, 255)) + v[-1]

def split_file(f_distr, f_data, f_txn, f_upper, n_data, n_txn, frac_full=0):
    with open(f_distr, "r") as distr, open(f_data, "w") as data, open(f_txn, "w") as txn, open(f_upper, "w") as upper:
        for l in distr:
            if n_txn + n_data == 0:
                break
            if np.random.random() < (n_txn)/(n_txn + n_data):
                n_txn -= 1
                txn.write(l)
                upper.write(transform_upper(l))
                if np.random.random() < frac_full and n_data > 0:
                    n_data -= 1
                    data.write(l)
            else:
                n_data -= 1
                data.write(l)
        assert n_data == 0, "n_data is too high"
        assert n_txn == 0, "n_txn is too high"

def sort_file(f_data):
    data = None
    with open(f_data, "r") as fd:
        data = fd.readlines()
    data = sorted(data)
    with open(f_data, "w") as fd:
        for l in data:
            fd.write(l)


if len(sys.argv) != 6:
    print("Usage: {} [email-list] [output-dir] [nkeys] [nqrys] [fraction of full queries]".format(sys.argv[0]))
    sys.exit()

f_distr = sys.argv[1]
basedir = sys.argv[2]

if not os.path.exists(basedir):
    os.makedirs(basedir)

f_data = basedir + '/data.txt'
f_txn = basedir + '/txn.txt'
f_upper = basedir + '/upper_bound.txt'

nkeys = int(sys.argv[3])
nqrys = int(sys.argv[4])
frac_sel = float(sys.argv[5])

split_file(f_distr, f_data, f_txn, f_upper, nkeys, nqrys, frac_sel)
sort_file(f_data)
