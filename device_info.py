#!/usr/bin/python
from commands import getoutput
data = getoutput("./rkflashtool r 0x0 1024 2>/dev/null")
device_info = data[:1024].strip('\x00').splitlines()
for line in device_info:
		print line
