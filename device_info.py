#!/usr/bin/python
from commands import getoutput
import struct

data = getoutput("./rkflashtool r 0x0 1 2>/dev/null")
header = data[:4]
if header != "PARM":
	print "NAND content does not start with a parameters file!"
	sys.exit()
params_size = struct.unpack('<I', data[4:8])
params_size = params_size[0]
print "Device parameters:"
print data[8:8+params_size]

next_piece=data[8+params_size:8+params_size+4]
params_size = struct.unpack('<I', next_piece)

print "Next piece", next_piece
