CC = gcc
STRIP = strip

CFLAGS = -O3 -mtune=native -march=native
LDFLAGS = -lm

SFLAGS = --strip-all

PATHS = -I"lib"

OUT = ryzen_monitor

SRC = ryzen_monitor.c
SRC += pm_tables.c
SRC += "lib/libsmu.c"

all: ryzen_monitor.c pm_tables.c lib/libsmu.c
	$(CC) $(PATHS) $(CFLAGS) $(LDFLAGS) -o $(OUT) $(SRC)
	$(STRIP) $(SFLAGS) $(OUT)

clean:
	rm -rf *.o lib/*.o
