CC      = gcc
CFLAGS  = -std=c99 -Wall -Werror -g
LDFLAGS =

SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = cortex

.PHONY: doc

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo ld $^
	@$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	@echo cc $<
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo clean
	@rm -f $(OUTPUT) $(OBJECTS)

doc:
	@echo building docs..
	@doxygen
