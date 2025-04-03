#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include "adageMath.h"
#include "adageTape.h"

FILE *inputStream = NULL;
FILE *outputStream = NULL;

char inputFileName[64] = "";
char outputFileName[64] = "";
char outputString[256] = "";
char buffer[256];
char octalStrBuf[16][32];

char fmt4[]="% 4d %s %s %s %s  >%s%s%s%s<\n";
char fmt8[]="% 4d %s %s %s %s %s %s %s %s  >%s%s%s%s%s%s%s%s<\n";

char fmt4_A[]="% 7d %s %s %s %s  >%s%s%s%s<\n";
char fmt8_A[]="% 7d %s %s %s %s %s %s %s %s  >%s%s%s%s%s%s%s%s<\n";

uint32_t *pInputOrigin = NULL;   // Saved readFile() malloc
uint32_t *pInput = NULL;   // Pointer to current inputFile block
uint32_t *pInputLimit = NULL;   // Pointer to last input Byte
uint32_t octalWords[16];

int i;
int c;
int indx;
int inputSize;
int optind = 0;
int cols = 4;
int colsm1 = 3;

char *cptr;
char outputName[8] = "";
char *addrModeStr="%07o";
char *dataModeStr="%010o";
int sectorInfo = 0;
int absMode = 0;
int wideMode = 0;
int by8Mode = 0;

char inbuf[128];
char outbuf[128];
int result = 0;

static struct option longopt[] =
{
   {"output"		, required_argument,    NULL, 'o'},
   {"input" 		, required_argument,    NULL, 'i'},
   {"disk"  		, no_argument,   		NULL, 'D'},
   {"absolute" 		, no_argument,   		NULL, 'A'},
   {"8words" 		, no_argument,   		NULL, '8'},
   {"wide"			, no_argument,   		NULL, 'w'},
   {NULL    		, 0,                    NULL,  0 }
};

void usage(char *myname)
{
	fprintf(stderr,
		"Usage: %s [-i] [-o] [-D] [-A] [-w] [-8]\n"
		"           --input  	-i Specify input file name, else stdin\n"
		"           --output 	-o Specify output file name, else stdout\n"
		"           --disk   	-D DISK file format listing sector addr\n"
		"           --absolute  -A DISK file format listing abs addr\n"
		"           --8words    -8 print 8 words/line\n"
		"           --wide      -w Data always printed with full 32 bits\n",
		myname);

	return;
}

int main(int argc, char **argv)
{
	int c;

	inputStream = stdin;
	outputStream = stdout;

	strcpy(inputFileName, "<stdin>");
	strcpy(outputFileName, "<stdout>");

	while ((c = getopt_long(argc, argv, "8w?i:o:DA", longopt, &optind)) >= 0)
	{
	   switch (c)
	   {
			case 'i':
			   if ((inputStream = fopen(optarg, "r")) == NULL)
			   {
				   fprintf(stderr, "Couldn't open %s for reading\n", optarg);

				   return(1);
			   }

			   strcpy(inputFileName, optarg);

			   break;

			case 'o':
			   if ((outputStream = fopen(optarg, "w")) == NULL)
			   {
				   fprintf(stderr, "Couldn't open %s for writing\n", optarg);

				   return(1);
			   }

			   break;

			case 'D':
			   addrModeStr="% 3d";
			   sectorInfo = 1;

			   break;

			case 'A':
			   addrModeStr="% 7d";
			   absMode = 1;

			   break;

			case '8':
			   by8Mode = 1;
			   cols = 8;
			   colsm1 = 7;

			   break;

			case 'w':
			   dataModeStr="%011o";
			   wideMode = 1;

			   break;

			case '?':
				usage(argv[0]);
				return(0);

			default:
				usage(argv[0]);
				return(1);
	   }
	}

	pInput = pInputOrigin = readFile(inputStream, &inputSize);
	pInputLimit = pInput + (inputSize - 1);

	if (inputStream != stdin)
	{
		fclose(inputStream);
	}

	//fprintf(stdout, "inputSize = %d\n", inputSize);

	for (i = 0; i < (((inputSize + colsm1) / cols) * cols); i++)
	{
		if (i < inputSize)
		{
			octalWords[i % cols] = *pInput++;
			sprintf(octalStrBuf[i % cols], (wideMode) ? "%011o" : "%010o",
				octalWords[i % cols]);
			amosName2Ascii(octalWords[i % cols],
				octalStrBuf[cols + (i % cols)]);
		}
		else
		{
			sprintf(octalStrBuf[i % cols], "%*c", (wideMode) ? 11 : 10, ' ');
			octalStrBuf[cols + (i % cols)][0] = '\0';
		}

		if (sectorInfo == 1)
		{
			if ((i % 104) == 0)
			{
				fprintf(outputStream, "# Cylinder %d Sector %d\n",
					i / (104 * 16), (i / 104) % 16);
			}

			if ((i % cols) == colsm1)
			{
				fprintf(outputStream, (cols == 8) ? fmt8 : fmt4,
					((i - colsm1) % 104),
					octalStrBuf[0], octalStrBuf[1],
					octalStrBuf[2], octalStrBuf[3],
					octalStrBuf[4], octalStrBuf[5],
					octalStrBuf[6], octalStrBuf[7],
					octalStrBuf[8], octalStrBuf[9],
					octalStrBuf[10], octalStrBuf[11],
					octalStrBuf[12], octalStrBuf[13],
					octalStrBuf[14], octalStrBuf[15]);
			}
		}
		else if (absMode == 1)
		{
			if ((i % cols) == colsm1)
			{
				fprintf(outputStream, (cols == 8) ? fmt8_A : fmt4_A,
					i - colsm1,
					octalStrBuf[0], octalStrBuf[1],
					octalStrBuf[2], octalStrBuf[3],
					octalStrBuf[4], octalStrBuf[5],
					octalStrBuf[6], octalStrBuf[7],
					octalStrBuf[8], octalStrBuf[9],
					octalStrBuf[10], octalStrBuf[11],
					octalStrBuf[12], octalStrBuf[13],
					octalStrBuf[14], octalStrBuf[15]);
			}
		}
		else
		{
			if ((i % cols) == colsm1)
			{
				fprintf(outputStream, (cols == 8) ? fmt8 : fmt4,
					cols * (i / cols),
					octalStrBuf[0], octalStrBuf[1],
					octalStrBuf[2], octalStrBuf[3],
					octalStrBuf[4], octalStrBuf[5],
					octalStrBuf[6], octalStrBuf[7],
					octalStrBuf[8], octalStrBuf[9],
					octalStrBuf[10], octalStrBuf[11],
					octalStrBuf[12], octalStrBuf[13],
					octalStrBuf[14], octalStrBuf[15]);
			}
		}
	}

	fprintf(stdout, "inputSize = %d, remainder = %d\n", inputSize,
			inputSize % cols);

	return(0);
}
