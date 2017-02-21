#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
int main( int argc, const char* argv[] ){
	FILE* file = fopen("404_slides_pg79.bin", "w+b");
	unsigned char chardata = 0;
	unsigned short shortdata = sizeof(unsigned int);
	unsigned long longdata = 12;


	unsigned long empty = 0;

	//////////////////*			HEADER			*//////////////////////
	fwrite(&chardata, sizeof(unsigned char), 1, file); //Write Revision
	fwrite(&shortdata, sizeof(unsigned short), 1, file); //Write keySize
	fwrite(&shortdata, sizeof(unsigned short), 1, file); //Write valueSize
	shortdata = 2;
	fwrite(&shortdata, sizeof(unsigned short), 1, file); //Write leafCount
	fwrite(&longdata, sizeof(unsigned long), 1, file); //Write recordCount
	longdata = 3;
	fwrite(&longdata, sizeof(unsigned long), 1, file); //Write depth


	//////////////////*			KEYS			*//////////////////////
	fseek(file, 32, SEEK_SET);
	unsigned int intdata = 50;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);

	intdata =10;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =30;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =70;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =90;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	intdata =4;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =8;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =10;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =22;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =30;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =45;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =50;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =69;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =70;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =89;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =90;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =99;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	//////////////////*			VALUES			*//////////////////////

	intdata =4;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =8;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =10;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =22;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =30;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =45;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =50;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =69;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =70;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =89;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =90;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	intdata =99;
	fwrite(&intdata, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fwrite(&empty, sizeof(unsigned int), 1, file);
	fclose(file);
	free(file);
	
}
