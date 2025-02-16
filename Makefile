# Makefile for Adage file tools

WARN	= -Wall
DEBUG	= -g
CC		= gcc
BIN		= ./bin

HEADERS += adageMath.h adageTape.h

OBJECTS	= adageMath.o

CFLAGS	+= -fdiagnostics-color=always $(WARN) -Wformat -Wall -lc

all:	$(OBJECTS) $(BIN)/adageTape $(BIN)/tapeLister $(BIN)/extractFile

$.o:	%.c
		$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

$(BIN)/adageTape:	adageTape.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o $(BIN)/adageTape adageMath.o adageTape.c

$(BIN)/tapeLister:	tapeLister.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o $(BIN)/tapeLister adageMath.o tapeLister.c

$(BIN)/extractFile:	extractFile.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o $(BIN)/extractFile adageMath.o extractFile.c

adageMath.o: adageMath.c $(HEADERS)
			$(CC) $(CFLAGS) $(DEBUG) -c adageMath.c

