WARN=-Wall -Wextra -Werror
CFLAGS=-std=c99 -ggdb $(WARN) -fPIC
LIBS:=-lm
LDFLAGS:=
OBJ:=sample.o ook.o stdcio.o threshold.o copy.o

all: $(OBJ) libook.so ookthreshold ooksample ookcopy

ooksample: sample.o libook.so
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

ookthreshold: threshold.o libook.so
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

ookcopy: copy.o libook.so
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

libook.so: ook.o stdcio.o
	$(CC) -fPIC -shared -Wl,--version-script=symbols.map $^ -o $@ $(LIBS)
	@#$(CC) -fPIC -shared $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJ) libook.so ookcopy ooksample ookthreshold
