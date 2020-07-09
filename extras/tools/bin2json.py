#!/usr/bin/python3

import sys
import json
import base64

CHUNK_SIZE           = 256 # This is the chunk size of how the binary is split on the server side for not overloading the embedded device receive buffers.
INTER_CHUNK_DELAY_MS = 500 # This is delay between 2 consecutive chunks so as to not over load the embedded device.

if len(sys.argv) != 3:
    print ("Usage: bin2json.py sketch.ota sketch.json")
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
json.dump({'binary' : base64_data.decode("ascii"), 'chunk_size' : CHUNK_SIZE, 'delay' : INTER_CHUNK_DELAY_MS}, out_file)
out_file.close()
