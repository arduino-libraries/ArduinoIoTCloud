#!/usr/bin/python3

import sys
import base64

if len(sys.argv) != 3:
    print ("Usage: bin2base64.py sketch-ota.bin sketch-ota.base64")
    sys.exit()

ifile = sys.argv[1]
ofile = sys.argv[2]

# Read the binary file
in_file = open(ifile, "rb")
bin_data = bytearray(in_file.read())
in_file.close()

# Perform the base64 conversion
base64_data = base64.b64encode(bin_data)

# Write to outfile
out_file = open(ofile, "wb")
out_file.write(base64_data)
out_file.close()
