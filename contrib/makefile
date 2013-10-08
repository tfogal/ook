WARN=-Wall -Wextra -Werror
VIPS_CF:=$(shell pkg-config --cflags vips-7.28)
VIPS_LD:=$(shell pkg-config --libs vips-7.28)
CFLAGS=-std=c99 -ggdb $(WARN) -I../ $(VIPS_CF)
LIBS:=../libook.so $(VIPS_LD)
LDFLAGS:=
OBJ:=carr.o chain2.o cp.o debugio.o imgio.o minmax.o stack.o

all: $(OBJ) ../libook.so icopy iminmax

icopy: carr.o cp.o imgio.o stack.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

iminmax: chain2.o debugio.o minmax.o imgio.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

../libook.so:
	$(MAKE) -C ../

clean:
	rm -f $(OBJ) icopy iminmax
