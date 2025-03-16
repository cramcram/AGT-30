#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "adageTape.h"

FILE *inputStream = NULL;
FILE *outputStream = NULL;

char currentLine[256] = "";
char dataText[256] = "";
char tapeName[256] = "";
char octalData[256] = "";
char blockSizeText[16] = "";
char *lineToken = "";
char *target = NULL;

uint32_t octalWords[8];
uint8_t octalBytes[64];

int blockErrors = 0;
int errorRecord = 0;
int blockWarnings = 0;
int currentLineLen = 0;
int errorBlock = 0;
int tokenLen = 0;
int tokenID = -1;
int blockSize = 0;
int i;
int j;
int c;
int matches;
int lineLen;
int recordNum;
int fileNum;
int inputLineNum;
int numDataBlocks = 0;
int numTotalBytes = 0;
int numThousandBytes = 0;
int numSubBytes = 0;
int numTapeMarks = 0;
int numCharsRead = 0;
int expectedRecordSize = 0;
uint32_t outputWord;

int optind = 0;
static struct option longopt[] =
{
   { "input", required_argument,    0, 'i'},
   {"output", required_argument,    0, 'o'},
   {    NULL,                 0, NULL,  0}
};

#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}

int main(int argc, char **argv)
{
	int c;

	inputStream = stdin;
	outputStream = stdout;

	while ((c = getopt_long(argc, argv, "i:o:", longopt, &optind)) >= 0)
	{
	   switch (c)
	   {
		   case 'o':
			   if ((outputStream = fopen(optarg, "w")) == NULL)
			   {
				   fprintf(stderr, "Couldn't open %s for writing\n", optarg);
				   return(1);
			   }
			   break;

		   case 'i':
			   if ((inputStream = fopen(optarg, "r")) == NULL)
			   {
				   fprintf(stderr, "Couldn't open %s for reading\n", optarg);

				   return(1);
			   }

			   break;
	   }
	}

	for (inputLineNum = 0;
		fgets(currentLine, sizeof(currentLine) - 1, inputStream) != NULL; )
	{
		inputLineNum++;
		lineLen = strlen(currentLine);

		if (lineLen == 1)
		{
			// Blank line ?

			continue;
		}

		// file: line ?

		target = "file: ";

		if (!strncmp(currentLine, target, strlen(target)))
		{
			strcpy(tapeName, &currentLine[strlen(target)]);
			fprintf(stderr, "Tape Name: %s", tapeName);

			continue;
		}

		// options: line ?

		target = "options: ";

		if (!strncmp(currentLine, target, strlen(target)))
		{
			fprintf(stderr, currentLine);

			continue;
		}

		// end of file line ?

		target = "end of file";

		if (!strncmp(currentLine, target, strlen(target)))
		{
			continue;
		}

		// File size info ?

		target = "tapemarks";

		if (strstr(currentLine, target) != NULL)
		{
			matches = sscanf(currentLine,
				"there were %d data blocks with %d,%d bytes, and %'d tapemarks",
				&numDataBlocks, &numThousandBytes, &numSubBytes, &numTapeMarks);

			numTotalBytes = (numThousandBytes * 1000) + numSubBytes;

			if (matches == 4)
			{
				continue;
			}
			else
			{
				fprintf(stderr,
			"Line %d: Error parsing block/bytes/marks data (only %d matches)\n",
					inputLineNum, matches);
			}
		}

		// Error info ?

		target = "errors";

		if (strstr(currentLine, target) != NULL)
		{
			if (!strncmp(currentLine, "no", strlen("no")))
			{
				blockErrors = 0;
			}
			else
			{
				if (sscanf(currentLine, "%'d", &blockErrors) != 1)
				{
					fprintf(stderr,
						"Line %d: Error parsing block errors line.\n",
						inputLineNum);

					continue;
				}
			}

			continue;
		}
			
		// Warning info ?

		target = "warnings";

		if (strstr(currentLine, target) != NULL)
		{
			if (!strncmp(currentLine, "no", strlen("no")))
			{
				blockWarnings = 0;
			}
			else
			{
				if (sscanf(currentLine, "%'d", &blockWarnings) != 1)
				{
					fprintf(stderr,
						"Line %d: Error parsing block warnings line.\n",
						inputLineNum);

					continue;
				}
			}

			fprintf(stderr,
		"%d blocks, %d total bytes, %d tapemarks, %d errors and %d warnings\n",
				numDataBlocks, numTotalBytes, numTapeMarks, blockErrors,
				blockWarnings);

			// Flag EOF with a 32 bit all ones..

			outputWord = 0xffffffff;
			fwrite(&outputWord, sizeof(outputWord), 1, outputStream);
			fprintf(stderr, "\nDone\n");

			if (outputStream != NULL)
			{
				fclose(outputStream);
				outputStream = NULL;
			}

			return(0);
		}

		// From here on we assume a data line, possible a new
		// record with possible errors.

		// See if record start line...

		if (currentLine[5] == ':')
		{
			errorRecord = 0;
			numCharsRead = 0;

			// OK, see if an error record...

			if (currentLine[0] == '!')
			{
				errorRecord = 1;
				currentLine[0] = ' ';
			}

			// Get expected record size...

			if ((matches =
				sscanf(currentLine, "%'d", &expectedRecordSize) != 1))
			{
				fprintf(stderr, "Line %d: Error reading record line size\n",
					inputLineNum);

				if (outputStream != NULL)
				{
					fclose(outputStream);
					outputStream = NULL;
				}

				return(1);
			}

			// Output inter-record flag word...

			outputWord = expectedRecordSize | TAPE_INTER_RECORD_FLAG |
				(errorRecord ? TAPE_INTER_RECORD_ERROR : 0);

			if (outputStream != NULL)
			{
				fwrite(&outputWord, sizeof(outputWord), 1, outputStream);
			}
		}

		// Grab octal data only

		strncpy(octalData, &currentLine[7], 7 + (11 * 8) + 1);
		matches = sscanf(octalData, "%o %o %o %o %o %o %o %o",
			&octalWords[0], &octalWords[1], &octalWords[2], &octalWords[3],
			&octalWords[4], &octalWords[5], &octalWords[6], &octalWords[7]);

		// We always round up to next word if non-multiple of 5 chars
		// originally read due to tape read error.

		for (i = 0; (numCharsRead < expectedRecordSize) && (i < 8); i++)
		{
			if (outputStream != NULL)
			{
				fwrite(&octalWords[i], sizeof(octalWords[0]), 1, outputStream);
			}

			numCharsRead += 5;
		}
	}
}

