CFLAGS := -O2 -W -Wall -s -DUSE_OPENSSL -g
LDLIBS := -lusb-1.0  -lssl -lcrypto

#CFLAGS := -DUSE_GCRYPT
#LDLIBS := -lgcrypt

TARGETS := rkcrc rkflashtool rkunpack rkafpack img_unpack img_maker afptool\
	to565

all: ${TARGETS}

clean:
	rm ${TARGETS}

