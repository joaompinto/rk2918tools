#!/usr/bin/python
import sys
import os
import magic
from os.path import dirname, join, exists, split ,splitext

if len(sys.argv) != 2:
	print "Usage: %s dirname" % sys.argv[0]
	sys.exit(1)
base_dir = dirname(sys.argv[0])
img_dir = sys.argv[1]

if img_dir == "boot":
	os.system("sudo chown -R root:root boot")
	os.chdir("boot")
	os.system('sudo find . ! -name "."|sort|sudo cpio -oa -H newc|gzip -n >../newinitrd.gz')
	os.chdir("..")
	os.system(join(base_dir, "rkcrc")+" newinitrd.gz boot.img")
	print "Generated boot.img"


