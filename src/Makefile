CC = gcc

CFLAGS = -O3 -mtune=native -march=native
override CFLAGS += -Ilib
override LDFLAGS += -lm

OUT = ryzen_monitor

SRC = ryzen_monitor.c
SRC += pm_tables.c
SRC += readinfo.c
SRC += lib/libsmu.c

OBJ = $(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUT) $(OBJ)

clean:
	rm -rf *.o lib/*.o
