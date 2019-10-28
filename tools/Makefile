CC = gcc
CCFLAGS = -Wall -Wno-unused-result -O3 -I../include

SOURCES = $(wildcard ../source/*.c) $(wildcard *.c)
OBJECTS = $(filter-out %/host.o,$(SOURCES:.c=.o))
TARGETS = printns

all: $(OBJECTS) $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGETS)
