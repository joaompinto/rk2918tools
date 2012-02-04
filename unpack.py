#!/usr/bin/python
import sys
import os
import magic
from os.path import dirname, join, exists, split ,splitext

if len(sys.argv) != 2:
	print "Usage: %s file.img" % sys.argv[0]
	sys.exit(1)
base_dir = dirname(sys.argv[0])
img_file = sys.argv[1]
header = open(img_file).read(4)
m = magic.open(magic.MAGIC_NONE)
m.load()
type = m.file(img_file)
if header == "KRNL":
	raw_file=img_file+"-raw"
	rc = os.system(join(base_dir,"rkunpack ")+img_file)
	if rc <> 0 or not exists(raw_file):
		print "Unable to unpack file"
		sys.exit(2)
	print raw_file
	type = m.file(raw_file)
	if 'gzip compressed data' in type:
		x_file = raw_file+".x"
		rc = os.system("gzip -dc %s > %s" % (raw_file, x_file))
		if rc <> 0 or not exists(x_file):
			print "Unable to extract file"
			sys.exit(3)
		type = m.file(x_file)
		if "cpio archive" in type:			
			output_dir = splitext(split(raw_file)[1])[0]
			if exists(output_dir):
				print "%s already exists, remove/rename it first!!" % output_dir
				sys.exit(4)
			os.mkdir(output_dir)
			os.chdir(output_dir)
			os.system("cpio -i --preserve-modification-time --make-directories < ../"+x_file)
			os.chdir("..")
			print "Image extracted into %s/" % output_dir
elif 'Linux Compressed ROM File System' in type:
	output_dir = splitext(split(img_file)[1])[0]
	if exists(output_dir):
		print "%s already exists, remove/rename it first!!" % output_dir
		sys.exit(5)
	os.system("cramfsck -x %s %s" % (output_dir, img_file))
	print "Image extracted into %s/" % output_dir
