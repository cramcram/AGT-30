#include <stdio.h>

extern char amos2Ascii[65];
extern char amos2AsciiNoBracket[65];
extern char *typeName[16];
extern char *monthNames[16];

extern char *amosName2Ascii(uint32_t, char *);
extern char *amosName2AsciiNoNull(uint32_t, char *);

//extern char *amosString2Ascii[65];

extern int outputAsciiFromAmosWord(FILE *, uint32_t, int);
extern int outputAsciiFromAmosWordWithTabs(FILE *, uint32_t, int);
extern uint32_t add30Bit(uint32_t a, uint32_t b);

extern char tabStops[256];

#define NUM_TYPES (16)

