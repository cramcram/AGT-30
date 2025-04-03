#include <stdio.h>

#define AMOS_SPACE		0
#define AMOS_RAW		1
#define AMOS_BRACKET	2

#define AMOS_NUM_DEFS	3

// NOTE: high and low are in (30 bit)BE[00:29] == (32 bit)LE[29:00] format!!!

#define BF(x, high, low) \
	(((x) >> (29 - (high))) & (((1 << ((low) - (high))) << 1) - 1))

extern char amos2Ascii[3][128];
extern char amos2AsciiNoBracket[65];
extern char *typeName[16];
extern char *monthNames[16];

extern char *amosName2Ascii(uint32_t, char *);
extern char *amosName2AsciiNoNull(uint32_t, char *);
extern void* readFile(FILE *, int *);

//extern char *amosString2Ascii[65];

extern uint32_t cswToOffset(uint32_t);
extern int outputAsciiFromAmosWord(FILE *, uint32_t, int, int);
extern int outputAsciiFromAmosWordWithTabs(FILE *, uint32_t, int, int);
extern uint32_t add30Bit(uint32_t a, uint32_t b);
extern uint32_t bf(uint32_t, int, int);

extern char tabStops[256];

#define NUM_TYPES (16)

