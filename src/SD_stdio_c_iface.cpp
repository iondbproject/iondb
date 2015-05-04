/*
 * SD_stdio_c_iface.c
 *
 */

#include "SD_stdio_c_iface.h"

#include <SD.h>

/**
@brief		A structure that translates a file object to a C-compatible
		struct.
*/
struct _SD_File
{
	File 			f;		/**< The Arduino SD File object we to use. */
	int8_t 			eof;
};


int
sd_fclose(
	SD_FILE 	*stream
)
{
	if (stream) stream->f.close();
	delete stream;
	return 0;
}

int
sd_feof(
	SD_FILE 	*stream
)
{
	if ( stream->eof == 1)
	{
#ifdef DEBUG
		Serial.println("EOF");
#endif
		return -1;	//end of file has been reached
	}else
	{
#ifdef DEBUG
		Serial.println("NOT EOF");
#endif
		return 0;	//end of file has not been reached
	}
}


int
sd_fflush(
	SD_FILE 	*stream
)
{
	stream->f.flush();
	return 0;
}


int sd_fsetpos(
	SD_FILE 	*stream, 
	fpos_t 		*pos
)
{
	return (stream) ? !(stream->f.seek(*pos)) : 1;
}

int
sd_fgetpos(
	SD_FILE 	*stream,
	fpos_t 		*pos)
{
	*pos = (stream) ? stream->f.position() : 0;
	return 0;
}



//Opens the filename pointed to by filename using the given mode.
/** @todo update to handle other modes */
SD_FILE
*sd_fopen(
	char 	*filename,
	char 	*mode
)
{
	uint8_t operation;
#ifdef DEBUG
	Serial.print("Target mode: ");
	Serial.write((uint8_t*)mode,2);
	Serial.println();
#endif	
	if ( strcmp(mode,"r") == 0) //	Open a file for reading. The file must exist.
	{
		//check to see if file exists
#ifdef DEBUG
		Serial.println("checking for file");		
#endif		
		if (SD.exists(filename) == false)
		{
#ifdef DEBUG	
			Serial.println("File does not exist");
#endif
			return NULL;
		}		
#ifdef DEBUG
		Serial.println("file exists");
#endif
		operation = FILE_READ;
	}
	else if (strcmp(mode, "w") == 0) 	// Create an empty file for writing. 
						// If a file with the same name already exists 
						// its content is erased and the file is 
						// considered as a new empty file.
	{
#ifdef DEBUG
		Serial.println("opening file");
#endif		
		if (SD.exists(filename))
		{
			SD.remove(filename);
		}
		operation = FILE_WRITE;
	}else if (strcmp(mode, "r+") == 0) //Open a file for update both reading and writing. The file must exist.
	{
		if (!SD.exists(filename))
		{
			return NULL;
		}
		operation = FILE_WRITE;

	}else if (strcmp(mode, "w+") == 0) //Create an empty file for both reading and writing.
	{
#ifdef DEBUG	
		Serial.println("opening file");
#endif
		if (SD.exists(filename) == true)
		{
#ifdef DEBUG
			Serial.println("removing file");
#endif			
			SD.remove(filename);
		}
		operation = FILE_WRITE;
	}else if (strcmp(mode, "a+") == 0)
	{
		operation = FILE_WRITE;
	}
	else
	{
		return 0;		/** incorrect args */
	}	
		_SD_File *file 	 = new struct _SD_File();
		(file)->f = SD.open(filename, operation);
		if (!((file)->f))
		{
			return 0;
		}
		return file;
}


size_t
sd_fread(
	void 		*ptr,
	size_t 		size,
	size_t 		nmemb,
	SD_FILE 	*stream
)
{
	//read is the size of bytes * num of size-bytes
	int num_bytes = stream->f.read((char *)ptr, size * nmemb);
#ifdef DEBUG	
	Serial.print("Bytes read : ");
	Serial.println(num_bytes);
#endif	
	if (num_bytes < size * nmemb)
	{
#ifdef DEBUG
		Serial.println("End of file");
#endif	
		stream->eof = 1;
	}	
	return  num_bytes / size;
}



int
 sd_fseek(
	SD_FILE 	*stream,
	long int 	offset,
	int 		whence
)
{
	unsigned long cur_pos 	= stream->f.position();
	unsigned long cur_end 	= stream->f.size();
#ifdef DEBUG	
	Serial.print("Current pos ");
	Serial.println(cur_pos);
	Serial.print("Current end ");
	Serial.println(cur_end);
	Serial.print("Offset ");
	Serial.println(offset);
#endif
	stream->eof 		= 0;

	switch (whence)
	{
		case SEEK_SET:  		//seek from current position
		{
#ifdef DEBUG
			Serial.println("Seek from start");
#endif			
			if (offset < 0) return -1;		//can't seek before file
			
			return (stream) ? !(stream->f.seek(offset)) : 1;
			break;
		}
		case SEEK_CUR:
		{	
#ifdef DEBUG
			Serial.println("Seek from cur");
#endif
			if (offset+cur_pos > cur_end)
			{
				stream->eof = 1;
				return -1;
			}
			if (offset - cur_pos < 0) return -1;		// too far
			return (stream) ? !(stream->f.seek(cur_pos + offset)) : 1;
			break;
		}
		case SEEK_END:
		{
#ifdef DEBUG	
			Serial.println("Seek from end");
#endif	
			if (offset > 0) 
			{
				stream->eof = 1;
				return -1;
			}//can't seek past end of tile
			return (stream) ? !(stream->f.seek(cur_end + offset)) : 1;
			break;
		}
		default:
			return 1;
	}
}


long int
sd_ftell(
	SD_FILE 	*stream
)
{
	long int pos =  (stream) ? stream->f.position() : 0;
#ifdef DEBUG
	Serial.print("cur pos: ");
	Serial.println(pos);
#endif	
	return pos;
}


size_t
sd_fwrite(
	void 		*ptr,
	size_t 		size,
	size_t 		nmemb,
	SD_FILE 	*stream
)
{
	size_t idx;
	size_t bytes_written = 0;
	size_t total_count = 0;

	//for each element, continue to write
	for (idx = 0; idx < nmemb; idx ++)
	{
		bytes_written = stream->f.write((uint8_t *)ptr, size);
		if (bytes_written != size)
			return total_count;
		total_count += 1;
	}
	if (total_count != nmemb)
		return -1;
	else
		return total_count;
}


int
sd_remove(
	char 	*filename
)
{
	return SD.remove(filename);
}


void sd_rewind(
	SD_FILE 	*stream
)
{
	stream->eof  	= 0;
	stream->f.seek(0);
}


int SD_File_Begin(uint8_t csPin)
{
	return SD.begin(csPin);
}


void
sd_printint(int i)
{
	Serial.println(i);
}

