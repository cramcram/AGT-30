# Makefile for Adage file tools

WARN	= -Wall
DEBUG	= -g
CC		= gcc
BIN		= ./bin

HEADERS += adageMath.h adageTape.h diskLister.h

OBJECTS	= adageMath.o

CFLAGS	+= -fdiagnostics-color=always $(WARN) -Wformat -Wall -lc

all:	$(OBJECTS) $(BIN)/adageTape $(BIN)/tapeLister $(BIN)/extractFile \
			$(BIN)/extractFile $(BIN)/extractFile $(BIN)/odAgt \
			$(BIN)/diskLister

$.o:	%.c
		$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

$(BIN)/diskLister:	diskLister.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o $(BIN)/diskLister adageMath.o \
			diskLister.c

$(BIN)/odAgt:	odAgt.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) adageMath.o -o $@ $< 

$(BIN)/adageTape:	adageTape.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) adageMath.o -o $@ $<

$(BIN)/tapeLister:	tapeLister.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o $(BIN)/tapeLister adageMath.o \
				tapeLister.c

$(BIN)/extractFile:	extractFile.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o $(BIN)/extractFile adageMath.o \
				extractFile.c

adageMath.o: adageMath.c $(HEADERS)
			$(CC) $(CFLAGS) $(DEBUG) -c adageMath.c

