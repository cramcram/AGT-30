#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include "adageMath.h"
#include "diskLister.h"

FILE *inputStream = NULL;
FILE *outputStream = NULL;

#if 0
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
int c;
int tapeBlockSizeWords = 0;   // Size of tape block in words
int tapeWords = 0;
int optind = 0;
#endif

int i;
uint32_t *pInputOrigin = NULL;   // Saved readFile() malloc
uint32_t *pInput = NULL;   // Pointer to current inputFile block

int diskSize = 0;   // Size of "disk" in words

char packName[64] = "";

typedef struct {
	uint32_t lastFirstId;
	uint32_t numFirstCyl;
} vdIdEntry;

typedef struct {
	vdIdEntry vId[NUM_VOL_IDS];	// 0 - 63
	uint32_t notUsed_1;			// 64
	uint32_t fdSectorEntry[31];	// 65 - 95
	uint32_t lastAvailPlusOne;	// 96
	uint32_t firstAvail;		// 97
	uint32_t notUsed_2;			// 98
	uint32_t creationDate;		// 99
	uint32_t amosPackName[4];	// 100 - 103
} vd;

vd *pVolDir = NULL;

typedef struct {
	uint32_t prevNextAddr;
	uint32_t fileInfo;
	uint32_t amosTitle;
	uint32_t tvrDate;
	uint32_t numSecWords;
} fd;

typedef struct {
	uint32_t firstNext;
	fd fdEntry[NUM_FD_ENTRIES_PER_SECTOR];
	uint32_t notUsed[3];
} fdSec;

fdSec (*pFdSectors)[NUM_FD_SECTORS] = NULL;

typedef struct {
	uint32_t prevNextFileId;
	uint32_t info;
	uint32_t name;
	uint32_t tvrDate;
	uint32_t secWordFile;
	uint32_t zero;   // Not shown in Adage docs, but observed on disk image
} fEntry;

fEntry *pFE = NULL;
fEntry *pTE = NULL;
fEntry *pLE = NULL;

int vol;
int firstIdCSW;
int lastIdCSW;
int thisIdCSW;
int startIdCyl;
int startIdSec;
int startIdWord;
int lastIdCyl;
int lastIdSec;
int lastIdWord;
int firstCyl;
int numCyl;
char fileName[64] = "";

static struct option longopt[] =
{
   {"input", required_argument,    0, 'i'},
   {"help",        no_argument,    0, '?'},
   {NULL   ,                 0, NULL,   0}
};

#define GOT_HERE {fprintf(stderr, "%d: Got here!\n", __LINE__);}

void usage(char *myname)
{
	fprintf(stderr, "Usage: %s [-i input_file_name]\n", myname);
	fprintf(stderr, "           -? Print this help message\n");
	fprintf(stderr, "           -i Specify input tape image, else stdin\n");

	return;
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
		   case '?':
			   usage(argv[0]);
			   return(0);

		   case 'i':
			   if ((inputStream = fopen(optarg, "r")) == NULL)
			   {
				   fprintf(stderr, "Couldn't open %s for reading\n", optarg);

				   return(1);
			   }
	   }
	}

	pInput = pInputOrigin = readFile(inputStream, &diskSize);

	if (inputStream != stdin)
	{
		fclose(inputStream);
	}

	pVolDir = (void *)pInput;
	pInput += sizeof(vd);

	pFdSectors = (void *)pInput;
	pInput += sizeof(fdSec[NUM_FD_SECTORS]);

	for (i = 0; i < 4; i++)
	{
		amosName2Ascii(pVolDir->amosPackName[i], &packName[i * 5]);
	}

	fprintf(outputStream, "Pack Name: %s\n", packName);

	// Note that index 0 -> Vol 1 ID

	for (i = 0; i < 32; i++)
	{
		if ((pVolDir->vId[i].lastFirstId) || (pVolDir->vId[i].numFirstCyl))
		{
			vol = i + 1;
			thisIdCSW = firstIdCSW = bf(pVolDir->vId[i].lastFirstId, 15, 29);
			lastIdCSW = bf(pVolDir->vId[i].lastFirstId, 0, 14);

			startIdCyl = bf(pVolDir->vId[i].lastFirstId, 15, 18);
			startIdSec = bf(pVolDir->vId[i].lastFirstId, 19, 22);
			startIdWord = bf(pVolDir->vId[i].lastFirstId, 23, 29);
			lastIdCyl = bf(pVolDir->vId[i].lastFirstId, 0, 3);
			lastIdSec = bf(pVolDir->vId[i].lastFirstId, 4, 7);
			lastIdWord = bf(pVolDir->vId[i].lastFirstId, 8, 14);
			firstCyl = bf(pVolDir->vId[i].numFirstCyl, 15, 29) >> 4;
			numCyl = bf(pVolDir->vId[i].numFirstCyl, 0, 14) >> 4;

#if 0
			fprintf(outputStream, "<%010o:%010o> ",
				pVolDir->vId[i].lastFirstId, pVolDir->vId[i].numFirstCyl);
#endif

			fprintf(outputStream,
"Volume %02o: <%010o %010o> ID = %d/%d/%d:%d to ID = %d/%d/%d:%d, Cyl = %d for %d Cyls\n",
				vol,
pVolDir->vId[i].lastFirstId,pVolDir->vId[i].numFirstCyl,
				startIdCyl, startIdSec, startIdWord, cswToOffset(firstIdCSW),
				lastIdCyl, lastIdSec, lastIdWord, cswToOffset(lastIdCSW),
				firstCyl, numCyl);

			thisIdCSW = bf(pVolDir->vId[i].lastFirstId, 15, 29);
			lastIdCSW = bf(pVolDir->vId[i].lastFirstId, 0, 14);

#define p2Id(csw) ((fEntry *)&(pInputOrigin[cswToOffset(csw)]))

			do {
				pTE = (fEntry *)&(pInputOrigin[cswToOffset(thisIdCSW)]);
				//amosName2Ascii(p2Id(thisIdCSW)->name, fileName);
				amosName2Ascii(pTE->name, fileName);
				fprintf(outputStream, "Name is %s <%010o>\n", fileName, pTE->name);

				thisIdCSW = bf(pTE->prevNextFileId, 15, 29);
			} while (thisIdCSW != lastIdCSW);


		}
	}

	fprintf(outputStream, "Done\n");

#if 0
	fprintf(outputStream, " FILE  RECORDS    NAME   ORIGIN  LENGTH   TYPE   " \
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
			fprintf(outputStream, 
				"%5o    %5o   %5s   %6d  %6d  %5s  %5d %5d  %2d/%3s/%4d\n",
				tapeFileNum, tapeRecordNum, fileName, origin, totalFileWords,
				typeName[typeNum], version, revision, day, monthNames[month],
				year);

			totalFileWords = 0;
		}

		tapeRecordNum = (pHeader->recFile >> 15) & 077777;
		tapeFileNum = pHeader->recFile & 077777;
	}

	fprintf(outputStream, "Done, %d blocks\n", blockNum);
#endif
}
