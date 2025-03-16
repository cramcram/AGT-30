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

char inputFileName[64] = "";
char outputFileName[64] = "";
char tapeFileName[64] = "";
char adageFileName[64] = "";
char adageFileNameWord[8] = "";
char adageFileTypeName[64] = "";

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
int tapeFileNameLen;
int tapeRecordNum;
int recordBodyWords;
int totalFileWords;
int blockErrors = 0;
int errorRecord = 0;
int blockNum;
int i;
int c;
int indx;
int tapeSize = 0;   // Size of "tape" in words (i.e., ignore inter-record words)
int tapeBlockSizeWords = 0;   // Size of tape block in words
int tapeWords = 0;
int optind = 0;
int adageFileTypeNum = 0;
int adageFileNameLen = 0;
int extractThisFile;

char *cptr;
char outputName[8] = "";
char outputTypeName[8] = "";
int outputType = -1;
int asciiOut = 0;

static struct option longopt[] =
{
   {"output", required_argument,    NULL, 'o'},
   {"input" , required_argument,    NULL, 'i'},
   {"ASCII" , no_argument,          NULL, 'A'},
   {NULL    , 0,                    NULL, 0}
};

enum inputFormat
{
	BIN, ATEXT, ASCII, DISK, UNKNOWN
};

struct FTYPES
{
	int typeNum;
	char typeName[8];
	enum inputFormat type; 
};

struct FTYPES typeInfo[16] =
{
	{000, "SYMS", BIN},
	{001, "DATA", BIN},
	{002, "RELOC", BIN},
	{003, "ASCII", ASCII},
	{004, "TEXT", ATEXT},
	{005, "RLSYM", BIN},
	{006, "PRNTR", BIN},
	{007, "ATEXT", ATEXT},
	{010, "BIN", BIN},
	{011, "DUMP", BIN},
	{012, "DISK", DISK},
	{013, "MACRO", BIN},
	{014, "TYPE5", UNKNOWN},
	{015, "TYPE6", UNKNOWN},
	{016, "TYPE7", UNKNOWN},
	{017, "TYPE8", UNKNOWN}
};

#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}

void* readFile(FILE *fp, int *fileSize)
{
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

void usage(char *myname)
{
	fprintf(stderr, "Usage: %s [-i input_file_name] [-o output_file_name] "\
							"FILE.TYPE\n", myname);
	fprintf(stderr, "           -i Specify input file name, else stdin\n");
	fprintf(stderr, "           -o Specify output file name, else stdout\n");
	fprintf(stderr, "           -A Interpret binary words as ASCII chars\n");
	fprintf(stderr, "           FILE.TYPE - Name of file to extract\n");
	fprintf(stderr, "                      *Must be all upper case!\n");

	return;
}

char inbuf[128];
char outbuf[128];
int result = 0;

int main(int argc, char **argv)
{
	int c;
	int posn;

	inputStream = NULL;
	outputStream = NULL;

	strcpy(inputFileName, "<stdin>");
	strcpy(outputFileName, "<stdout>");

//	Adage tab settings -> 13, 27, 43, -8

	while ((c = getopt_long(argc, argv, "?Ai:o:", longopt, &optind)) >= 0)
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

			case 'A':
			   asciiOut = 1;

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

	if (argv[optind] == NULL)
	{
		fprintf(stderr, "Missing FILE.TYPE argument\n");
		return(1);
	}

	strcpy(adageFileName, argv[optind]);
		
	if ((cptr = strchr(adageFileName, '.')) == NULL)
	{
		fprintf(stderr, "Invalid FILE.TYPE argument (no '.' present)\n");
		return(1);
	}

	*cptr++ = '\0';
	strcpy(adageFileTypeName, cptr);

	if ((strlen(adageFileName) < 1) || (strlen(adageFileName) > 5))
	{
		fprintf(stderr, "Invalid FILE argument (too short/long)\n");
		return(1);
	}

	if ((strlen(adageFileTypeName) < 1) && (strlen(adageFileTypeName) > 5))
	{
		fprintf(stderr, "Invalid TYPE argument (too short/long)\n");
		return(1);
	}

#if 0
	fprintf(stderr, "Using %s.%s as FILE.TYPE to extract from %s\n",
		adageFileName, adageFileTypeName, inputFileName);
#endif

	strcpy(adageFileNameWord, adageFileName);

	while ((c = strlen(adageFileNameWord)) < 5)
	{
		adageFileNameWord[c++] = ' ';
		adageFileNameWord[c] = '\0';
	}

	for (i = 0; i < NUM_TYPES; i++)
	{
		if (strcmp(adageFileTypeName, typeInfo[i].typeName))
		{
			continue;
		}

		adageFileTypeNum = typeInfo[i].typeNum;
	}

	if (outputType >= NUM_TYPES)
	{
		fprintf(stderr, "Invalid file type given <%s>\n", outputTypeName);
		return(1);
	}

#if 0
	fprintf(stderr, "Looking for file %s, type %s (%02o)\n", adageFileName,
		adageFileTypeName, adageFileTypeNum);
#endif

	pInput = pInputOrigin = readFile(inputStream, &tapeSize);

	if (inputStream != stdin)
	{
		fclose(inputStream);
	}

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

		amosName2Ascii(pHeader->fileName, tapeFileName);
		tapeFileNameLen = strlen(tapeFileName);

		day = (pHeader->dateSize >> (0 + 15)) & 037;
		month = (pHeader->dateSize >> (5 + 15)) & 017;
		year = (((pHeader->dateSize >> (9 + 15))) & 037) + 1964;
		recordBodyWords = pHeader->dateSize & 077777;
		totalFileWords += recordBodyWords;

		revision = (pHeader->tvsOrigin >> (0 + 15)) & 077;
		version = (pHeader->tvsOrigin >> (6 + 15)) & 037;
		typeNum = (pHeader->tvsOrigin >> (11 + 15)) & 017;
		origin = pHeader->tvsOrigin & 077777;

		extractThisFile =
			((!strncmp(tapeFileName, adageFileNameWord,
			strlen(adageFileNameWord))) && (typeNum == adageFileTypeNum));

		if (extractThisFile)
		{
			if (tapeRecordNum == 1)
			{
				fprintf(stderr,
					"Found the file %s.%s as File %03o, Record %03o\n",
					adageFileNameWord, adageFileTypeName, tapeFileNum,
					tapeRecordNum);
			}

			posn = 1;
		}

		for (i = 0, computedChecksum = 07777777777; i < recordBodyWords;
			i++)
		{
			computedChecksum = add30Bit(computedChecksum, pRecordData[i]);

			if (i >= (recordBodyWords - 1))
			{
				continue;
			}

			if (extractThisFile)
			{
				if (asciiOut)
				{
					if ((posn = outputAsciiFromAmosWordWithTabs(outputStream,
						pRecordData[i], AMOS_BRACKET, posn)) < 0)
					{
						posn = 1;
					}
				}
				else
				{
					fwrite((pRecordData + i), sizeof(uint32_t), 1,outputStream);
				}
			}
		}

		expectedChecksum = pRecordData[i];

		pInput += tapeBlockSizeWords + 1;
		tapeWords += tapeBlockSizeWords;
		interRecordWord = *pInput;
		pHeader = (tapeHeader *)(pInput + 1);

		if ((IS_EOT(interRecordWord)) ||
			((pHeader->recFile & 077777) != tapeFileNum))
		{
			totalFileWords = 0;
		}

		tapeRecordNum = (pHeader->recFile >> 15) & 077777;
		tapeFileNum = pHeader->recFile & 077777;
	}
}
