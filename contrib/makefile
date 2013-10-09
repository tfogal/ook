WARN=-Wall -Wextra -Werror
VIPS_CF:=$(shell pkg-config --cflags vips-7.28)
VIPS_LD:=$(shell pkg-config --libs vips-7.28)
CFLAGS=-std=c99 -ggdb $(WARN) -I../ $(VIPS_CF)
LIBS:=-L../ -look $(VIPS_LD)
LDFLAGS:=
OBJ:=carr.o cast8.o chain2.o cp.o debugio.o imgio.o mask.o minmax.o stack.o

all: $(OBJ) ../libook.so ocopy ocast8 omask ominmax

ocopy: carr.o cp.o imgio.o stack.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

ominmax: chain2.o debugio.o minmax.o imgio.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

omask: mask.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

ocast8: cast8.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

../libook.so:
	$(MAKE) -C ../

clean:
	rm -f $(OBJ) ocast8 ocopy omask ominmax
