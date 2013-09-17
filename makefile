WARN=-Wall -Wextra -Werror
CFLAGS=-std=c99 -ggdb $(WARN)
LIBS:=-lm
LDFLAGS:=
OBJ:=sample.o ook.o stdcio.o

all: $(OBJ) ooksample

ooksample: sample.o ook.o stdcio.o
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJ)
