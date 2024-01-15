#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PATH 1024

void writeAAVal(unsigned char val, FILE *fp)
{
	fputc(0xaa | (val >> 1), fp);
	fputc(0xaa | val, fp);
}

char *PathFindExtension(const char *path)
{
	char *p;

	for (p = (char *)path; *p != 0x00; p++);

	for (; *p != '.'; p--);

	return p;
}

void PathRemoveExtension(char *path)
{
	char *p;

	for (p = path; *p != 0x00; p++);

	for (; *p != '.'; p--);

	*p = 0x00;
}

void conv_dsk2nic(const char *name)
{
	static const unsigned char encTable[] = {
		0x96, 0x97, 0x9A, 0x9B, 0x9D, 0x9E, 0x9F, 0xA6,
		0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB2, 0xB3,
		0xB4, 0xB5, 0xB6, 0xB7, 0xB9, 0xBA, 0xBB, 0xBC,
		0xBD, 0xBE, 0xBF, 0xCB, 0xCD, 0xCE, 0xCF, 0xD3,
		0xD6, 0xD7, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE,
		0xDF, 0xE5, 0xE6, 0xE7, 0xE9, 0xEA, 0xEB, 0xEC,
		0xED, 0xEE, 0xEF, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,
		0xF7, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
	};
	static const unsigned char scramble[] = {
		0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15
	};
	static const unsigned char FlipBit1[4] = { 0, 2, 1, 3 };
	static const unsigned char FlipBit2[4] = { 0, 8, 4, 12 };
	static const unsigned char FlipBit3[4] = { 0, 32, 16, 48 };

	unsigned char src[256 + 2];
	int volume = 0xfe;
	int track, sector;

	if (!name || !name[0]) {
		return;
	}

	FILE *fp, *fpw;
	char path[MAX_PATH];

	if (strncasecmp(PathFindExtension(name), ".DSK", 4) != 0) {
		fprintf(stderr, "Use .DSK file.");
		return;
	}

	fp = fopen(name, "r");
	if (fp == (FILE *)NULL) {
		fprintf(stderr, "Can't open the DSK file %s.", name);
		return;
	}

	strcpy(path, name);
	PathRemoveExtension(path);
	strcat(path, ".NIC");

	fpw = fopen(path, "w");	
	if (fpw == (FILE *)NULL) {
		fprintf(stderr, "Can't write %s.\n", path);
		fclose(fp);
		return;
	}

	for (track = 0; track < 35; track++) {
		for (sector = 0; sector < 16; sector++) {
			int i;
			unsigned char x, ox = 0;

			if (fseek(fp, track * (256 * 16) + scramble[sector] * 256, SEEK_SET) != 0) {
				fprintf(stderr, "File error.");
				fclose(fp);
				fclose(fpw);
				exit(-1);
			}

			for (i = 0; i < 22; i++) {
				fputc(0xff, fpw);
			}

			fputc(0x03, fpw);
			fputc(0xfc, fpw);
			fputc(0xff, fpw);
			fputc(0x3f, fpw);
			fputc(0xcf, fpw);				
			fputc(0xf3, fpw);
			fputc(0xfc, fpw);
			fputc(0xff, fpw);
			fputc(0x3f, fpw);
			fputc(0xcf, fpw);
			fputc(0xf3, fpw);
			fputc(0xfc, fpw);

			fputc(0xd5, fpw);
			fputc(0xaa, fpw);
			fputc(0x96, fpw);
			writeAAVal(volume, fpw);
			writeAAVal(track, fpw);
			writeAAVal(sector, fpw);
			writeAAVal(volume ^ track ^ sector, fpw);
			fputc(0xde, fpw);
			fputc(0xaa, fpw);
			fputc(0xeb, fpw);
			for (i = 0; i < 5; i++) {
				fputc(0xff, fpw);
			}
			fputc(0xd5, fpw);
			fputc(0xaa, fpw);
			fputc(0xad, fpw);
			fread(src, 1, 256, fp);
			src[256] = src[257] = 0;
			for (i = 0; i < 86; i++) {
				x = (FlipBit1[src[i] & 3] | FlipBit2[src[i + 86] & 3] | FlipBit3[src[i + 172] & 3]);
				fputc(encTable[(x^ox)&0x3f], fpw);
				ox = x;
			}
			for (i = 0; i < 256; i++) {
				x = (src[i] >> 2);
				fputc(encTable[(x ^ ox) & 0x3f], fpw);
				ox = x;
			}
			fputc(encTable[ox & 0x3f], fpw);
			fputc(0xde, fpw);
			fputc(0xaa, fpw);
			fputc(0xeb, fpw);
			for (i = 0; i < 14; i++) {
				fputc(0xff, fpw);
			}
			for (i = 0; i < (512 - 416); i++) {
				fputc(0x00, fpw);
			}
		}
	}

	//fprintf(stderr, "the NIC image created.");
	fclose(fpw);
	fclose(fp);
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		int i;
		for (i = 1; i < argc; i++) {
			conv_dsk2nic(argv[i]);
		}
	}
	return 0;
}

