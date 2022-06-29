all: colorslide

CC=cc

LIBS=-lm
CFLAGS=-O3 -pipe -ansi -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-pointer-sign -Wno-incompatible-pointer-types
DEBUGCFLAGS=-Og -pipe -g

INPUT=colorslide.c
OUTPUT=colorslide

RM=/bin/rm

colorslide: $(INPUT)
	$(CC) $(INPUT) -o $(OUTPUT) $(LIBS) $(CFLAGS)
debug: $(INPUT)
	$(CC) $(INPUT) -o $(OUTPUT) $(LIBS) $(DEBUGCFLAGS)
clean: $(OUTPUT)
	if [ -e $(OUTPUT) ]; then $(RM) $(OUTPUT); fi
