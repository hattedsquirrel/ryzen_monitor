CC = gcc
STRIP = strip

CFLAGS = -O3 -mtune=native -march=native
CFLAGS += -Ilib
LDFLAGS = -lm

SFLAGS = --strip-all

OUT = ryzen_monitor

SRC = ryzen_monitor.c
SRC += pm_tables.c
SRC += readinfo.c
SRC += lib/libsmu.c

OBJ = $(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUT) $(OBJ)
	$(STRIP) $(SFLAGS) $(OUT)

clean:
	rm -rf *.o lib/*.o
