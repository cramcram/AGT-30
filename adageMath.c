#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "adageMath.h"

char amos2Ascii[3][128] = {{" %]!&*:_"
							"+t?\"'r()"
							"01234567"
							"89;=,-./"
							" ABCDEFG"
							"HIJKLMNO"
							"PQRSTUVW"
							"XYZ$#@^b"},

						   {"n%]!&*:_"
							"+t?\"'r()"
							"01234567"
							"89;=,-./"
							" ABCDEFG"
							"HIJKLMNO"
							"PQRSTUVW"
							"XYZ$#@^b"},

						   {"[%]!&*:_"
							"+\t?\"'\n()"
							"01234567"
							"89;=,-./"
							" ABCDEFG"
							"HIJKLMNO"
							"PQRSTUVW"
							"XYZ$#@^b" }};

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

#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}
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

int outputAsciiFromAmosWord(FILE *outStream, uint32_t amosWord, int amosTable,
							int posn)
{
	int i;
	char asciiChar;

	for (i = 0; i < 5; i++)
	{
		asciiChar = amos2Ascii[amosTable][(amosWord >> shiftStops[i]) & 077];
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
									int amosTable, int posn)
{
	int i;
	static int nextTabStop = 0;
	static int tabIndex = 0;
	char asciiChar =0;

	// Setup tab info if start of output line

	if (posn <= 1)
	{
		posn = 1;
		tabIndex = 0;
		nextTabStop = tabStops[tabIndex];
		nextTabStop = (nextTabStop < 0) ? abs(nextTabStop) : nextTabStop;
	}

	for (i = 0; i < 5; i++)
	{
		asciiChar = amos2Ascii[amosTable][(amosWord >> shiftStops[i]) & 077];

		if (asciiChar == '\t')
		{
			while (posn < nextTabStop)
			{
				fputc(' ', outStream);
				posn++;
			}

			if (tabStops[tabIndex + 1] < 0)
			{
				nextTabStop += 8;
			}
			else
			{
				nextTabStop = tabStops[++tabIndex];
			}
		}
		else
		{
			fputc(asciiChar, outStream);
			posn++;

			if (asciiChar == '\n')
			{
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

	outstr[0] = amos2Ascii[AMOS_SPACE][(amosNameWord >> 24) & 077];
	outstr[1] = amos2Ascii[AMOS_SPACE][(amosNameWord >> 18) & 077];
	outstr[2] = amos2Ascii[AMOS_SPACE][(amosNameWord >> 12) & 077];
	outstr[3] = amos2Ascii[AMOS_SPACE][(amosNameWord >>  6) & 077];
	outstr[4] = amos2Ascii[AMOS_SPACE][(amosNameWord >>  0) & 077];
	outstr[5] = '\0';

	return(outstr);
}

char *amosName2AsciiNoNull(uint32_t amosNameWord, char *asciiStr)
{
	static char lastStr[8];
	char *outstr = (asciiStr == NULL) ? lastStr : asciiStr;

	outstr[0] = amos2Ascii[AMOS_RAW][(amosNameWord >> 24) & 077];
	outstr[1] = amos2Ascii[AMOS_RAW][(amosNameWord >> 18) & 077];
	outstr[2] = amos2Ascii[AMOS_RAW][(amosNameWord >> 12) & 077];
	outstr[3] = amos2Ascii[AMOS_RAW][(amosNameWord >>  6) & 077];
	outstr[4] = amos2Ascii[AMOS_RAW][(amosNameWord >>  0) & 077];
	outstr[5] = '\0';

	return(outstr);
}

char *amosString2Ascii(uint32_t *amosString, char *asciiStr)
{
	static char lastStr[8];
	char *outstr = (asciiStr == NULL) ? lastStr : asciiStr;

	return(outstr);
}
