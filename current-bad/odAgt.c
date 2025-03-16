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
char octalStrBuf[8][256];

uint32_t *pInputOrigin = NULL;   // Saved readFile() malloc
uint32_t *pInput = NULL;   // Pointer to current inputFile block
uint32_t *pInputLimit = NULL;   // Pointer to last input Byte
uint32_t octalWords[8];

int i;
int c;
int indx;
int inputSize;
int optind = 0;

char *cptr;
char outputName[8] = "";

static struct option longopt[] =
{
   {"output", required_argument,    NULL, 'o'},
   {"input" , required_argument,    NULL, 'i'},
   {NULL    , 0,                    NULL, 0}
};

//#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}

void usage(char *myname)
{
	fprintf(stderr, "Usage: %s [-i input_file_name] [-o output_file_name]\n",
		myname);
	fprintf(stderr, "           -i Specify input file name, else stdin\n");
	fprintf(stderr, "           -o Specify output file name, else stdout\n");

	return;
}

char inbuf[128];
char outbuf[128];
int result = 0;

int main(int argc, char **argv)
{
	int c;

	inputStream = stdin;
	outputStream = stdout;

	strcpy(inputFileName, "<stdin>");
	strcpy(outputFileName, "<stdout>");

	while ((c = getopt_long(argc, argv, "?i:o:", longopt, &optind)) >= 0)
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

fprintf(stderr, "inputSize = %d\n", inputSize);

	for (i = 0; i < ((inputSize + 3) % 4); i++)
	{
		if (i < inputSize)
		{
			octalWords[i % 4] = *pInput++;
		}

		if ((i % 4) == 3)
		{
			fprintf(outputStream, "% 5d: %010o %010o %010o %010o %s%s%s%s\n",
				4 * (i / 4),
				octalWords[0], octalWords[1],
				octalWords[2], octalWords[3],
				amosWord2AsciiTable[octalWords[0], octalStrBuf[0],
					amosWord2Ascii),
				amosWord2AsciiTable[octalWords[1], octalStrBuf[1],
					amosWord2Ascii),
				amosWord2AsciiTable[octalWords[2], octalStrBuf[2],
					amosWord2Ascii),
				amosWord2AsciiTable[octalWords[3], octalStrBuf[3]),
					amosWord2Ascii);
		}
	}

	if ((i % 4) != 0)
	{
		memset(&outputString[4 + (4 - ((i % 4) * 11))], ' ', 1);
		strcpy(&outputString[52 + (4 - ((i % 4) * 5))], "\n");

		fprintf(outputStream, outputString);
	}
}
