#!/usr/bin/python3

import sys
import ctypes

LZSS_SO_FILE = "./lzss.so"

if len(sys.argv) != 4:
    print ("Usage: lzss.py --[encode|decode] sketch.bin sketch.lzss")
    sys.exit()

lzss_functions = ctypes.CDLL(LZSS_SO_FILE)

mode   = sys.argv[1]
ifile  = sys.argv[2]
ofile  = sys.argv[3]

b_ifile = ifile.encode('utf-8')
b_ofile = ofile.encode('utf-8')

if mode == "--encode":
    lzss_functions.encode_file.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lzss_functions.encode_file(b_ifile, b_ofile)
elif mode == "--decode":
    lzss_functions.decode_file.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lzss_functions.decode_file(b_ifile, b_ofile)
else:
    print ("Error, invalid mode parameter, use --encode or --decode")
