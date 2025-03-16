#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "adageMath.h"

char amos2AsciiNullSpace[65] =	" %]!&*:_"
								"+t?\"'r()"
								"01234567"
								"89;=,-./"
								" ABCDEFG"
								"HIJKLMNO"
								"PQRSTUVW"
								"XYZ$#@^b";

char amos2AsciiRawNoNull[65] =	"n%]!&*:_"
								"+t?\"'r()"
								"01234567"
								"89;=,-./"
								" ABCDEFG"
								"HIJKLMNO"
								"PQRSTUVW"
								"XYZ$#@^b";

char amos2Ascii[65]	=			"[%]!&*:_"
								"+\t?\"'\n()"
								"01234567"
								"89;=,-./"
								" ABCDEFG"
								"HIJKLMNO"
								"PQRSTUVW"
								"XYZ$#@^b";

//	Adage tab settings -> 13, 27, 43, -8

char tabStops[256] = {13, 27, 43, -8};

char *typeName[16] = {
	"SYMS", "DATA", "RELOC", "ASCII", "TEXT", "RLSYM", "PRNTR", "ATEXT",
	"BIN", "DUMP", "DISK", "MACRO", "TYPE5", "TYPE6", "TYPE7", "TYPE8"
};

char *monthNames[16] = {
	"***", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL",
	"AUG", "SEP", "OCT", "NOV", "DEC", "***", "***", "***"
};

int shiftStops[5] = {24, 18, 12, 6, 0};

//#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}
//#define GOT_HERE {fprintf(stderr, "%s:%s:%d: Got here!\n", 
//		__FILE__,__func__,__LINE__);}

// Do a 30 bit + 30 bit 1's complement add

uint32_t add30Bit(uint32_t a, uint32_t b)
{
	uint32_t sum;

	a &= 0x3fffffff;
	b &= 0x3fffffff;

	sum = a + b;

	if (sum & 0x40000000)
	{
		sum++;
		sum &= 0x3fffffff;
	}

	return(sum);
}

int outputAsciiFromAmosWord(FILE *outStream, uint32_t amosWord, int posn)
{
	int i;
	char asciiChar;

	for (i = 0; i < 5; i++)
	{
		asciiChar = amos2Ascii[(amosWord >> shiftStops[i]) & 077];
		fputc(asciiChar, outStream);
		posn++;

		if (asciiChar == '\n')
		{
			return(-posn);
			break;
		}
	}

	return(posn);
}

int outputAsciiFromAmosWordWithTabs(FILE *outStream, uint32_t amosWord,
	int posn)
{
	int i;
	static int nextTabStop = 0;
	static int tabIndex = 0;
	char asciiChar =0;
fprintf(stderr, "amosWord %011o\n", amosWord);

	// Setup tab info if start of output line

	if (posn <= 1)
	{
GOT_HERE
		posn = 1;
		tabIndex = 0;
		nextTabStop = tabStops[tabIndex];
		nextTabStop = (nextTabStop < 0) ? abs(nextTabStop) : nextTabStop;
	}
GOT_HERE

	for (i = 0; i < 5; i++)
	{
GOT_HERE
		asciiChar = amos2Ascii[(amosWord >> shiftStops[i]) & 077];

		if (asciiChar == '\t')
		{
GOT_HERE
			while (posn < nextTabStop)
			{
GOT_HERE
				fputc(' ', outStream);
				posn++;
			}

			if (tabStops[tabIndex + 1] < 0)
			{
GOT_HERE
				nextTabStop += 8;
			}
			else
			{
GOT_HERE
				nextTabStop = tabStops[++tabIndex];
			}
		}
		else
		{
GOT_HERE
			fputc(asciiChar, outStream);
			posn++;

			if (asciiChar == '\n')
			{
GOT_HERE
				return(-posn);
				break;
			}
		}
	}

	return(posn);
}

char *amosName2Ascii(uint32_t amosNameWord, char *asciiStr)
{
	static char lastStr[8];
	char *outstr = (asciiStr == NULL) ? lastStr : asciiStr;

	outstr[0] = amos2AsciiNullSpace[(amosNameWord >> 24) & 077];
	outstr[1] = amos2AsciiNullSpace[(amosNameWord >> 18) & 077];
	outstr[2] = amos2AsciiNullSpace[(amosNameWord >> 12) & 077];
	outstr[3] = amos2AsciiNullSpace[(amosNameWord >>  6) & 077];
	outstr[4] = amos2AsciiNullSpace[(amosNameWord >>  0) & 077];
	outstr[5] = '\0';

	return(outstr);
}

char *amosName2AsciiTable(uint32_t amosNameWord, char *asciiStr)
{
	static char lastStr[8];
	char *outstr = (asciiStr == NULL) ? lastStr : asciiStr;

	outstr[0] = amos2Ascii[(amosNameWord >> 24) & 077];
	outstr[1] = amos2Ascii[(amosNameWord >> 18) & 077];
	outstr[2] = amos2Ascii[(amosNameWord >> 12) & 077];
	outstr[3] = amos2Ascii[(amosNameWord >>  6) & 077];
	outstr[4] = amos2Ascii[(amosNameWord >>  0) & 077];
	outstr[5] = '\0';

	return(outstr);
}

char *amosWord2AsciiTable(uint32_t amosNameWord, char *asciiStrBuf,
	char *amosAsciiTable)
{
	asciiStrBuf[0] = amos2Ascii[(amosNameWord >> 24) & 077];
	asciiStrBuf[1] = amos2Ascii[(amosNameWord >> 18) & 077];
	asciiStrBuf[2] = amos2Ascii[(amosNameWord >> 12) & 077];
	asciiStrBuf[3] = amos2Ascii[(amosNameWord >>  6) & 077];
	asciiStrBuf[4] = amos2Ascii[(amosNameWord >>  0) & 077];
	asciiStrBuf[5] = '\0';

	return(asciiStrBuf);
}

char *amosString2Ascii(uint32_t *amosString, char *asciiStr)
{
	static char lastStr[8];
	char *outstr = (asciiStr == NULL) ? lastStr : asciiStr;

	return(outstr);
}

int createOctalAscii(uint32_t *words, int numWords, int offset, char *outBuf)
{
	char* bufp = outBuf;
	int len = 0;
	int i;
	int j;

	for (i = 0; i < numWords; i++)
	{
		bufp += len = sprintf(bufp, "%010o ", words[i]);
	}

	for ( ; i < numWords; i++)
	{
		bufp += len = sprintf(bufp, "           ");
	}

	for (j = 0; j < numWords; j++)
	{
		bufp += len = sprintf(bufp, "%s", amosName2Ascii(words[j], bufp));
	}

	for ( ; j < numWords; j++)
	{
		bufp += len = sprintf(bufp, "     ");
	}

	return(len);
}

void* readFile(FILE *fp, int *fileSize)
{
    size_t capacity = 1024; // Initial capacity (in uint32_t units)
    size_t size = 0;
    uint32_t *buffer = malloc(capacity * sizeof(uint32_t));
    uint32_t value;

    if (!fp)
	{
		fp = stdin;
	}

    if (!buffer)
	{
        perror("malloc failed");
        return NULL;
    }
    
    while (fread(&value, sizeof(uint32_t), 1, fp) == 1)
	{
		if (feof(fp) || ferror(fp))
		{
			*fileSize = 0;
			return(NULL);
		}

        if (size >= capacity)
		{
            capacity *= 0x10000;
            uint32_t *temp = realloc(buffer, capacity * sizeof(uint32_t));

            if (!temp) {
                perror("realloc failed");
                free(buffer);
                return(NULL);
            }

            buffer = temp;
        }

        buffer[size++] = value;
    }
    
fprintf(stderr, "size = %d\n", (int)size);
    if (fileSize)
	{
        *fileSize = (int)size;
    }

    return buffer;
}

