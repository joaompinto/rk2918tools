CFLAGS := -O2 -W -Wall -s
LDLIBS := -lusb-1.0

#CFLAGS := -DUSE_GCRYPT
#LDLIBS := -lgcrypt

TARGETS := rkcrc rkflashtool rkunpack rkafpack.c

all: ${TARGETS}

clean:
	rm ${TARGETS}

