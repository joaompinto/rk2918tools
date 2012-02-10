#!/usr/bin/python
from commands import getoutput, getstatusoutput
from os.path import exists, dirname, join
import sys
import os
import re
import shutil

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

base_dir = dirname(sys.argv[0])
rkflash = join(base_dir, "rkflashtool")
data = getoutput(rkflash+" r 0x0 0x400 2>/dev/null")
device_info = data[:1024].strip('\x00')
if device_info:
	print "Device information:"
	print device_info
else:
	print "No device found, please make sure your device is set to flash mode."
	print "On some systems this can be done with:"
	print "    - Shutdown the device"
	print "    - Press the volume '-' while plugging the USB cable"
	sys.exit(2)

if len(sys.argv) < 3 or sys.argv[1] not in ["dump", "write"]:
	print "Usage: %s " % sys.argv[0]
	print "            dump misc|kernel|boot|recovery|system"
	sys.exit(2)

cmd = sys.argv[1]
part_name = sys.argv[2]
part_info = partition_info(device_info)
if cmd in ["dump", "write"]:
	partition = part_info.get(part_name, None)
	if partition is None:
		print "%s not found, available partitions: %s" % (part_name, ','.join(part_info))
		
		sys.exit(3)		
if cmd == "dump":
	if len(sys.argv) > 3:
		filename = sys.argv[3]	
	else:
		filename = part_name+".img"
	cmd = rkflash+" r %s %s > %s" % (partition.offset, partition.size, part_name+".img.tmp")
	print "Executing", cmd
	rc, output = getstatusoutput(cmd)
	if rc != 0:
		print "An error ocurred while reading image file"
	else:		
		if exists(filename):
			os.unlink(filename)
		shutil.move(part_name+".img.tmp", filename)
		print "Image sucesffully dumped to", part_name+".img"
elif cmd == "write":
	part_file = sys.argv[3]
	cmd = rkflash+" w %s %s < %s" % (partition.offset, partition.size, part_file)
	print "Executing", cmd
	rc, output = getstatusoutput(cmd)
	if rc != 0:
		print "An error ocurred while writing image file"
	else:
		print "Image sucesffully writen."
