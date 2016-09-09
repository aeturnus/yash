CC = gcc
LD = ld
CFLAGS =-Wall -Wextra
SOURCES = $(wildcard *.c)
EXECUTABLE = yash


OBJECTS = $(SOURCES:.c=.o)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) 


all: $(EXECUTABLE)
	@echo "Program built"

clean:
	rm *.o $(EXECUTABLE)

test:
	./$(EXECUTABLE)

mem:
	valgrind --leak-check=yes ./$(EXECUTABLE)

debug:
	gdb ./$(EXECUTABLE)
