/*-
 * Copyright (c) 2011 FUKAUMI Naoki.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "rkcrc.h"

#define BUFSIZE	0x800U
static uint8_t buf[BUFSIZE], header[BUFSIZE];

static uint32_t ioff;
static uint8_t *p, *selfp;
static int count;

static void
make_header(const char *arg)
{
	off_t fsize;
	uint32_t isize, noff, nsize;
	const char *sep;
	char name[0x20], fpath[0x3c];
	int fd;

	if ((sep = strchr(arg, ':')) == NULL)
		errx(EXIT_FAILURE, "invalid argument %s", arg);

	if (strncasecmp(arg, "FIRMWARE_VER", sep - arg) == 0) {
		arg = sep + 1;

		if ((sep = strchr(arg, '.')) == NULL)
			errx(EXIT_FAILURE, "invalid argument %s", arg);
		header[0x87] = strtol(arg, NULL, 10) & 0xff;
		arg = sep + 1;

		if ((sep = strchr(arg, '.')) == NULL)
			errx(EXIT_FAILURE, "invalid argument %s", arg);
		header[0x86] = strtol(arg, NULL, 10) & 0xff;
		arg = sep + 1;

		if ((sep = strchr(arg, '.')) != NULL)
			errx(EXIT_FAILURE, "invalid argument %s", arg);
		header[0x84] = strtol(arg, NULL, 10) & 0xff;

	} else if (strncasecmp(arg, "MACHINE_MODEL", sep - arg) == 0)
		strcpy((char *)&header[0x08], sep + 1);
	else if (strncasecmp(arg, "MACHINE_ID", sep - arg) == 0)
		strcpy((char *)&header[0x2a], sep + 1);
	else if (strncasecmp(arg, "MANUFACTURER", sep - arg) == 0)
		strcpy((char *)&header[0x48], sep + 1);
	else {
		memcpy(name, arg, sep - arg);
		name[sep - arg] = '\0';
		arg = sep + 1;

		if ((sep = strchr(arg, ':')) == NULL) {
			strcpy(fpath, arg);
			nsize = 0;
			noff = ~0U;
		} else {
			memcpy(fpath, arg, sep - arg);
			fpath[sep - arg] = '\0';
			arg = sep + 1;

			if ((sep = strchr(arg, '@')) == NULL)
				errx(EXIT_FAILURE, "invalid argument %s", arg);

			nsize = strtoul(arg, NULL, 16);
			arg = sep + 1;

			noff = strtoul(arg, NULL, 16);
		}

		if (memcmp(fpath, "SELF", 4) == 0) {
			selfp = p;
			fsize = isize = 0;
		} else {
			if ((fd = open(fpath, O_RDONLY)) == -1)
				err(EXIT_FAILURE, "%s", fpath);
			if ((fsize = lseek(fd, 0, SEEK_END)) == -1)
				err(EXIT_FAILURE, "%s", fpath);
			close(fd);

			if (memcmp(name, "parameter", 9) == 0)
				fsize += 12;

			isize = (fsize + (BUFSIZE - 1)) & ~(BUFSIZE - 1);
		}

		strcpy((char *)p, name);
		strcpy((char *)&p[0x20], fpath);
		p[0x5c] = (nsize >> 0) & 0xff;
		p[0x5d] = (nsize >> 8) & 0xff;
		p[0x5e] = (nsize >> 16) & 0xff;
		p[0x5f] = (nsize >> 24) & 0xff;
		if (memcmp(fpath, "SELF", 4) != 0) {
			p[0x60] = (ioff >> 0) & 0xff;
			p[0x61] = (ioff >> 8) & 0xff;
			p[0x62] = (ioff >> 16) & 0xff;
			p[0x63] = (ioff >> 24) & 0xff;
		}
		p[0x64] = (noff >> 0) & 0xff;
		p[0x65] = (noff >> 8) & 0xff;
		p[0x66] = (noff >> 16) & 0xff;
		p[0x67] = (noff >> 24) & 0xff;
		p[0x68] = (isize >> 0) & 0xff;
		p[0x69] = (isize >> 8) & 0xff;
		p[0x6a] = (isize >> 16) & 0xff;
		p[0x6b] = (isize >> 24) & 0xff;
		p[0x6c] = (fsize >> 0) & 0xff;
		p[0x6d] = (fsize >> 8) & 0xff;
		p[0x6e] = (fsize >> 16) & 0xff;
		p[0x6f] = (fsize >> 24) & 0xff;

		ioff += isize;
		p += 0x70;
		count++;
	}
}

int
main(int argc, char *argv[])
{
	ssize_t nr;
	uint32_t crc, fsize, pcrc;
	int fd, i;

	if (argc < 2) {
		fprintf(stderr,
		    "usage: %s [parameter:value] name:file[@offset] ...\n",
		    argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(header, 0, sizeof(header));
	memcpy(header, "RKAF", 4);
	ioff = BUFSIZE;
	p = &header[0x8c];
	selfp = NULL;
	count = 0;

	for (i = 1; i < argc; i++)
		make_header(argv[i]);

	header[0x04] = (ioff >> 0) & 0xff;
	header[0x05] = (ioff >> 8) & 0xff;
	header[0x06] = (ioff >> 16) & 0xff;
	header[0x07] = (ioff >> 24) & 0xff;
	header[0x88] = count & 0xff;

	if (selfp != NULL) {
		selfp[0x68] = ((ioff + 0x200) >> 0) & 0xff;
		selfp[0x69] = ((ioff + 0x200) >> 8) & 0xff;
		selfp[0x6a] = ((ioff + 0x200) >> 16) & 0xff;
		selfp[0x6b] = ((ioff + 0x200) >> 24) & 0xff;
		selfp[0x6c] = ((ioff + 4) >> 0) & 0xff;
		selfp[0x6d] = ((ioff + 4) >> 8) & 0xff;
		selfp[0x6e] = ((ioff + 4) >> 16) & 0xff;
		selfp[0x6f] = ((ioff + 4) >> 24) & 0xff;
	}

	crc = 0;
	RKCRC(crc, header, BUFSIZE);
	write(STDOUT_FILENO, header, BUFSIZE);

	for (p = &header[0x8c]; count > 0; p += 0x70, count--) {
		if (memcmp(&p[0x20], "SELF", 4) == 0)
			continue;

		fd = open((const char *)&p[0x20], O_RDONLY);

		if (memcmp(p, "parameter", 9) == 0) {
			memcpy(buf, "PARM", 4);

			fsize = (p[0x6c] | p[0x6d] << 8 | p[0x6e] << 16 |
			    p[0x6f] << 24) - 12;
			buf[4] = (fsize >> 0) & 0xff;
			buf[5] = (fsize >> 8) & 0xff;
			buf[6] = (fsize >> 16) & 0xff;
			buf[7] = (fsize >> 24) & 0xff;

			nr = read(fd, &buf[8], BUFSIZE - 8);

			pcrc = 0;
			RKCRC(pcrc, &buf[8], nr);
			buf[8 + nr + 0] = (pcrc >> 0) & 0xff;
			buf[8 + nr + 1] = (pcrc >> 8) & 0xff;
			buf[8 + nr + 2] = (pcrc >> 16) & 0xff;
			buf[8 + nr + 3] = (pcrc >> 24) & 0xff;

			memset(&buf[8 + nr + 4], 0, BUFSIZE - 8 - nr - 4);

			RKCRC(crc, buf, BUFSIZE);
			write(STDOUT_FILENO, buf, BUFSIZE);
		} else {
			while ((nr = read(fd, buf, BUFSIZE)) != -1 && nr != 0) {
				if (nr != BUFSIZE)
					memset(&buf[nr], 0, BUFSIZE - nr);
				RKCRC(crc, buf, BUFSIZE);
				write(STDOUT_FILENO, buf, BUFSIZE);
			}
		}

		close(fd);
	}

	buf[0] = (crc >> 0) & 0xff;
	buf[1] = (crc >> 8) & 0xff;
	buf[2] = (crc >> 16) & 0xff;
	buf[3] = (crc >> 24) & 0xff;

	write(STDOUT_FILENO, buf, 4);

	return EXIT_SUCCESS;
}
