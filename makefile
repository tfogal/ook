WARN=-Wall -Wextra -Werror
CFLAGS=-std=c99 -ggdb $(WARN) -fPIC
LIBS:=-lm
LDFLAGS:=
OBJ:=sample.o ook.o stdcio.o threshold.o copy.o

library:=libook.so
os:=$(shell uname -s)
ifeq ($(os), Darwin)
	library:=libook.dylib
endif

all: $(OBJ) $(library) ookthreshold ooksample ookcopy

ooksample: sample.o $(library)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

ookthreshold: threshold.o $(library)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

ookcopy: copy.o $(library)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

libook.so: ook.o stdcio.o
	$(CC) -fPIC -shared -Wl,--version-script=symbols.map $^ -o $@ $(LIBS)
	@#$(CC) -fPIC -shared $^ -o $@ $(LIBS)

libook.dylib: ook.o stdcio.o
	$(CC) -fPIC -shared -Wl $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJ) $(library) ookcopy ooksample ookthreshold
