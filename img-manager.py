#!/usr/bin/python
from commands import getoutput, getstatusoutput
from os.path import exists
import sys
import os
import re

class Partition:
	def __init__(self, offset, size):
		self.offset = offset
		self.size = size
	def __repr__(self):
		return "offset=%s,size=%s" % (self.offset, self.size)

def partition_info(device_info):
	partinfo = {}
	parts = re.findall("CMDLINE: .*mtdparts=(.*)", device_info)[0].split(',')
	for part in parts:
		part = part.split(':')[-1] # remove rk29xxnand prefix
		size, offset, name = re.findall("(.*)@(.*)\((.*)\)", part)[0]
		partinfo[name]= Partition(offset, size)		
	return partinfo

data = getoutput("./rkflashtool r 0x0 0x400 2>/dev/null")
device_info = data[:1024].strip('\x00')
if device_info:
	print "Device information:"
	print device_info
else:
	print "No device found, please make sure your device is set to flash mode."
	print "On some systems this can be done with:"
	print "    - Shutdown the device"
	print "    - Press the volume '-' while plugging the USB cable"

if len(sys.argv) < 3 or sys.argv[1] not in 'dump':
	print "Usage: %s " % sys.argv[0]
	print "            dump misc|kernel|boot|recovery|system"
	sys.exit(2)

cmd = sys.argv[1]
part_name = sys.argv[2]
part_info = partition_info(device_info)
if cmd == "dump":
	partition = part_info.get(part_name, None)
	if partition is None:
		print "%s not found, available partitions: %s" % (part_name, ','.join(part_info))
		sys.exit(3)
	cmd = "./rkflashtool r %s %s > %s" % (partition.offset, partition.size, part_name+".img.tmp")
	print "Executing", cmd
	rc, output = getstatusoutput(cmd)
	if rc != 0:
		print "An error ocurred while reading image file"
	else:
		if exists(part_name+".img"):
			os.unlink(part_name+".img")
		os.rename(part_name+".img.tmp", part_name+".img")
		print "Image sucesffully dumped to", part_name+".img"
	
