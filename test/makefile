WARN=-Wall -Wextra -Werror
CFLAGS=-std=c99 -ggdb $(WARN) -I../
LIBS:=-pthread ../libook.so -lcheck -lm -lrt
LDFLAGS:=
OBJ:=bricksize.o check.o rwop.o ../libook.so

all: $(OBJ) ../libook.so suite

../libook.so:
	$(MAKE) -C ../

suite: bricksize.o check.o rwop.o
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJ) suite
