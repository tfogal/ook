WARN=-Wall -Wextra -Werror
CFLAGS=-std=c99 -ggdb $(WARN) -fPIC
LIBS:=-lm
LDFLAGS:=
OBJ:=sample.o ook.o stdcio.o

all: $(OBJ) libook.so ooksample

ooksample: sample.o libook.so
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) libook.so

libook.so: ook.o stdcio.o
	#$(CC) -fPIC -shared -Wl,--version-script=symbols.map $^ -o $@ $(LIBS)
	$(CC) -fPIC -shared $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJ) libook.so ooksample
