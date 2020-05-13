#!/usr/bin/python3

import sys
import json
import base64

if len(sys.argv) != 3:
    print ("Usage: bin2json.py sketch-ota.bin sketch-ota.json")
    sys.exit()

ifile = sys.argv[1]
ofile = sys.argv[2]

# Read the binary file
in_file = open(ifile, "rb")
bin_data = in_file.read()
in_file.close()

# Perform the base64 conversion
base64_data = base64.b64encode(bin_data)

# Write to outfile
out_file = open(ofile, "w")
json.dump({'binary' : base64_data.decode("ascii")}, out_file)
out_file.close()
