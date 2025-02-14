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

struct headerStruct {   // Header words for tape files
	uint32_t recFile;
	uint32_t fileName;
	uint32_t dateSize;
	uint32_t tvsOrigin;
	uint32_t *data;
};

typedef struct headerStruct tapeHeader;
tapeHeader *pHeader = NULL;   // (== pInput[1])

char fileName[64] = "";

uint32_t computedChecksum;
uint32_t expectedChecksum;
uint32_t *pInputOrigin = NULL;   // Saved readFile() malloc
uint32_t *pInput = NULL;   // Pointer to current inputFile block
uint32_t interRecordWord;   // (== pInput[0])
uint32_t *pRecordData;   // (== pInput[5])

int origin;
int revision;
int version;
int typeNum;
int day;
int month;
int year;
int tapeFileNum;
int tapeRecordNum;
int recordBodyWords;
int totalFileWords;
int blockErrors = 0;
int errorRecord = 0;
int blockNum;
int i;
int c;
int tapeSize = 0;   // Size of "tape" in words (i.e., ignore inter-record words)
int tapeBlockSizeWords = 0;   // Size of tape block in words
int tapeWords = 0;
int optind = 0;

static struct option longopt[] =
{
   {"input", required_argument,    0, 'i'},
   {NULL   ,                 0, NULL,   0}
};

#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}

void* readFile(FILE *fp, int *fileSize) {
    
    size_t capacity = 1024; // Initial capacity (in uint32_t units)
    size_t size = 0;
    uint32_t *buffer = malloc(capacity * sizeof(uint32_t));
    uint32_t value;

    if (!fp) {
		return NULL;
	}

    if (!buffer) {
        perror("malloc failed");
        return NULL;
    }
    
    while (fread(&value, sizeof(uint32_t), 1, fp) == 1) {
        if (size >= capacity) {
            capacity *= 0x10000;
            uint32_t *temp = realloc(buffer, capacity * sizeof(uint32_t));
            if (!temp) {
                perror("realloc failed");
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }
        buffer[size++] = value;
    }
    
    if (fileSize) {
        *fileSize = size;
    }
    return buffer;
}

int main(int argc, char **argv)
{
	int c;

	inputStream = stdin;
	outputStream = stdout;

	while ((c = getopt_long(argc, argv, "i:", longopt, &optind)) >= 0)
	{
	   switch (c)
	   {
		   case 'i':
			   if ((inputStream = fopen(optarg, "r")) == NULL)
			   {
				   fprintf(stderr, "Couldn't open %s for reading\n", optarg);

				   return(1);
			   }
	   }
	}

	pInput = pInputOrigin = readFile(inputStream, &tapeSize);

	if (inputStream != stdin)
	{
		fclose(inputStream);
	}

	fprintf(stderr, " FILE  RECORDS    NAME   ORIGIN  LENGTH   TYPE   " \
		"VERS   REV  DA/MON/YEAR\n\n");

	for (blockNum = tapeWords = totalFileWords = 0;
		interRecordWord = *pInput, !IS_EOT(interRecordWord = *pInput);
		blockNum++)
	{
		if (!(IS_INTER_RECORD_WORD(interRecordWord)))
		{
			fprintf(stderr,
			"Tape word %d: expected inter-record word; read 0x%08x %010o\n",
				tapeWords, interRecordWord, interRecordWord);

			free(pInputOrigin);
			pInputOrigin = NULL;

			return(1);
		}

		tapeBlockSizeWords = TAPE_RECORD_LENGTH_WORDS(interRecordWord);

		pHeader = (tapeHeader *)(pInput + 1);
		pRecordData = pInput + 5;

		tapeRecordNum = (pHeader->recFile >> 15) & 077777;
		tapeFileNum = pHeader->recFile & 077777;

		amosName2Ascii(pHeader->fileName, fileName);

		day = (pHeader->dateSize >> (0 + 15)) & 037;
		month = (pHeader->dateSize >> (5 + 15)) & 017;
		year = (((pHeader->dateSize >> (9 + 15))) & 037) + 1964;
		recordBodyWords = pHeader->dateSize & 077777;
		totalFileWords += recordBodyWords;

		revision = (pHeader->tvsOrigin >> (0 + 15)) & 077;
		version = (pHeader->tvsOrigin >> (6 + 15)) & 037;
		typeNum = (pHeader->tvsOrigin >> (11 + 15)) & 017;
		origin = pHeader->tvsOrigin & 077777;

		for (i = 0, computedChecksum = 07777777777; i < recordBodyWords;
			i++)
		{
			computedChecksum = add30Bit(computedChecksum, pRecordData[i]);
		}

		expectedChecksum = pRecordData[i];

		pInput += tapeBlockSizeWords + 1;
		tapeWords += tapeBlockSizeWords;
		interRecordWord = *pInput;
		pHeader = (tapeHeader *)(pInput + 1);

		if ((IS_EOT(interRecordWord)) ||
			((pHeader->recFile & 077777) != tapeFileNum))
		{
			fprintf(stderr, 
				"%5o    %5o   %5s   %6d  %6d  %5s  %5d %5d  %2d/%3s/%4d\n",
				tapeFileNum, tapeRecordNum, fileName, origin, totalFileWords,
				typeName[typeNum], version, revision, day, monthNames[month],
				year);

			totalFileWords = 0;
		}

		tapeRecordNum = (pHeader->recFile >> 15) & 077777;
		tapeFileNum = pHeader->recFile & 077777;
	}

	fprintf(stderr, "Done, %d blocks\n", blockNum);
}
