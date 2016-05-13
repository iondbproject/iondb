
#ifndef SD_STDIO_C_IFACE_H_
#define SD_STDIO_C_IFACE_H_

#include "./../kv_system.h"

#ifdef ARDUINO

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "kv_stdio_intercept.h"

/**
@brief		Wrapper around Arduino File type.
*/
typedef struct _SD_File SD_FILE;

/**
@brief		Wrapper around Arduino SD file close method.
@param		file	Pointer to C file struct type associated with an SD
			file object.
*/
int sd_fclose(SD_FILE *stream);

//Tests the end-of-file indicator for the given stream.
int sd_feof(SD_FILE *stream);

//Flushes the output buffer of a stream.
int sd_fflush(SD_FILE *stream); //file.flush;

/**
@brief		Wrapper around Arduino SD file position method.
@param		file	Poin,ter to C file struct type associated with an SD
			file object.
@returns	The position of the cursor within the file.
*/
//Gets the current file position of the stream and writes it to pos
int sd_fgetpos(SD_FILE *stream, fpos_t *pos); //position()

/**
@brief		Wrapper around Arduino SD file write method.
@param		file		Pointer to a pointer of the C file structure
				type associated with the C++ SD file object.
				The pointer pointed to by this pointer
				will be set to the newly allocated object.
@param		filepath	String containing path to file (basic filename).
@param		mode		What mode to open the file under.
@returns	A file for reading, or @c NULL if an error occurred.
*/
//Opens the filename pointed to by filename using the given mode.
SD_FILE *sd_fopen(char *filename, char *mode);


/**
@brief		Wrapper around Arduino SD file read method.
@param		file	Pointer to C file struct type associated with an SD
			file object.
@param		buf	Pointer to the memory segment to be read into.
@param		nbytes	The number of bytes to be read.
@returns	A read status.
		(?)
*/
//Reads data from the given stream into the array pointed to by ptr.
size_t sd_fread(void *ptr, size_t size, size_t nmemb, SD_FILE *stream);

/**
@brief		Wrapper around Arduino SD file read method.
@param		file	Pointer to C file struct type associated with an SD
			file object.
@param		pos	The position in the file to seek to.
			(from the beginning?)
@returns	@c 1 for success, @c 0 for failure.
*/
//Sets the file position of the stream to the given offset. The argument offset signifies the number of bytes to seek from the given whence position.
int sd_fseek(SD_FILE *stream, long int offset, int whence);

int sd_fsetpos(SD_FILE *stream, fpos_t *pos);
//Sets the file position of the given stream to the given position. The argument pos is a position given by the function fgetpos.

//void sd_println(uint8_t * ch);

void sd_printint(int i);

//Returns the current file position of the given stream.
long int sd_ftell(SD_FILE *stream);
/**
@brief		Wrapper around Arduino SD file write method.
@param		file	Pointer to C file struct type associated with an SD
			file object.
@param		buf	Pointer to the data that is to be written.
@param		size	The number of bytes to be written.
@returns	The number of bytes written.
*/
//Writes data from the array pointed to by ptr to the given stream.
size_t sd_fwrite(void *ptr, size_t size, size_t nmemb, SD_FILE *stream);


/**
@brief		Wrapper around Arduino SD file remove method.
@param		filepath	The string containing the path to the file.
@returns	@c 1 if the file was removed successfully, @c 0 otherwise.
*/
//Deletes the given filename so that it is no longer accessible.
int sd_remove(char *filename);

//Seek
//Sets the file position to the beginning of the file of the given stream.
void sd_rewind(SD_FILE *stream);



/**
@brief		Wrapper around Arduino SD file begin method.
@param		csPin	The pin connected to the chip select line of the SD
			card.
@todo		Is there a safe number to use as a signal to use default
		pin?
*/
int SD_File_Begin(uint8_t csPin);

/**
@brief		Check to see if an Arduino SD File exists.
@param		filepath	String containing path to file (basic filename).
@returns	@c 1 if the file exists, @c 0 otherwise.
*/
int SD_File_Exists(char *filepath);

#ifdef __cplusplus
}
#endif

#endif /* Clause ARDUINO */

#endif
