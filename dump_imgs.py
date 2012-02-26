#!/usr/bin/python
from os.path import exists, isdir, join
from commands import getstatusoutput 
import sys
import os

if len(sys.argv) != 2:
	print "Usage: %s target_directory" % sys.argv[0]
	sys.exit(2)

target_dir = sys.argv[1]

if exists(target_dir) and not isdir(target_dir):
	print "%s was found, you must supply a directory name" % target_dir
	sys.exit(3)
if not exists(target_dir):
	os.makedirs(target_dir)

for part_name in ('misc','kernel','boot','recovery', 'system', 'backup'):
	rc = os.system('./img-manager.py dump %s %s.img' \
		% (part_name, join(target_dir, part_name)))
	if rc <> 0:
		break
	
print "Img files dumped into", target_dir
