#include <stdio.h>
#include <stdint.h>

uint32_t recordStart = 020000000047; // + size in words
uint32_t recordLast = 037777777777;

// Defines 16 word body...

uint32_t header[4] = {04142434445, 04647505152, 05354555657, 0};
uint32_t csum = 01111111111;

uint32_t thisval = (00 << 24) | (01 << 18) | (02 << 12) | (03 << 6) | (04 << 0);
uint32_t nextval = (05 << 24) | (05 << 18) | (05 << 12) | (05 << 6) | (05 << 0);

int main(int argc, char *argv)
{
	uint32_t val;
	int i;

	fwrite(&recordStart, sizeof(uint32_t), 1, stdout);
	fwrite(header, sizeof(uint32_t), 4, stdout);

	for (i = 0 ; i < 16; i++)
	{
		fwrite(&thisval, sizeof(thisval), 1, stdout);
		thisval += nextval;
	}

	fwrite(&csum, sizeof(uint32_t), 1, stdout);
	fwrite(&recordLast, sizeof(uint32_t), 1, stdout);
}

