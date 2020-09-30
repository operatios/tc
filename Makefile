CC=clang
CFLAGS=-std=c99 -pedantic -Wall -Wextra -Werror -Wwrite-strings

SOURCES = main.c tc.c
OBJECTS = $(SOURCES:.c=.o)


all: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o tc.exe

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	del *.o *.exe
