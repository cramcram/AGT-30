#include <stdio.h>

#if 1
#define GOT_HERE {fprintf(stderr, "%s:%d: Got here!\n", __func__,__LINE__);}
#else
#define GOT_HERE {fprintf(stderr, "%s:%s:%d: Got here!\n", \
		__FILE__,__func__,__LINE__);}
#endif

extern char amos2Ascii[65];
extern char amos2AsciiRaw[65];
extern char amos2AsciiRawNullSpace[65];
extern char *typeName[16];
extern char *monthNames[16];

extern char *amosName2Ascii(uint32_t, char *);
extern char *amosName2AsciiNoNull(uint32_t, char *);
extern void* readFile(FILE *fp, int *fileSize);

//extern char *amosString2Ascii[65];

extern int createOctalAscii(uint32_t *, int, int, char *);
extern int outputAsciiFromAmosWord(FILE *, uint32_t, int);
extern int outputAsciiFromAmosWordWithTabs(FILE *, uint32_t, int);
extern char *amosWord2Ascii(uint32_t, char *, char *);
extern char *amosWord2AsciiTable(uint32_t, char *, char *);
extern uint32_t add30Bit(uint32_t a, uint32_t b);
extern char amos2AsciiNullSpace[65];

extern char tabStops[256];

#define NUM_TYPES (16)

