# Makefile for Adage file tools

WARN	= -Wall
DEBUG	= -g
CC		= gcc

HEADERS += adageMath.h adageTape.h

OBJECTS	= adageMath.o

CFLAGS	+= -fdiagnostics-color=always $(WARN) -Wformat -Wall -lc

all:	$(OBJECTS) adageTape tapeLister extractFile

$.o:	%.c
		$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

adageTape:	adageTape.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o adageTape adageMath.o adageTape.c

tapeLister:	tapeLister.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o tapeLister adageMath.o tapeLister.c

extractFile:	extractFile.c adageMath.o $(HEADERS) Makefile
			$(CC) $(CFLAGS) $(DEBUG) -o extractFile adageMath.o extractFile.c

adageMath.o: adageMath.c $(HEADERS)
			$(CC) $(CFLAGS) $(DEBUG) -c adageMath.c

