/******************************************************************************/
/**
@file		sd_stdio_c_iface.cpp
@author		Scott Fazackerley
@brief		This code contains implementations for stdio.h file functions
			using Arduino SD File libraries.
@details	Since the Arduino library doesn't have definitions for
			standard I/O file functions, we have to write our own.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "sd_stdio_c_iface.h"
#include <SD.h>

/**
@brief		A structure that translates a file object to a C-compatible
			struct.
*/
struct _SD_File {
	File	f;				/**< The Arduino SD File object we to use. */
	int8_t	eof;			/**< A position telling us where the end of the
								 file currently is. */
};

int
sd_fclose(
	SD_FILE *stream
) {
	if (stream) {
		stream->f.close();
	}

	delete stream;
	return 0;
}

int
sd_feof(
	SD_FILE *stream
) {
	if (stream->eof == 1) {
#if DEBUG
		Serial.println("EOF");
#endif
		return -1;	/* end of file has been reached */
	}
	else {
#if DEBUG
		Serial.println("NOT EOF");
#endif
		return 0;	/* end of file has not been reached */
	}
}

int
sd_fflush(
	SD_FILE *stream
) {
	stream->f.flush();
	return 0;
}

int
sd_fsetpos(
	SD_FILE		*stream,
	ion_fpos_t	*pos
) {
	return (stream) ? !(stream->f.seek(*pos)) : 1;
}

int
sd_fgetpos(
	SD_FILE		*stream,
	ion_fpos_t	*pos
) {
	*pos = (stream) ? stream->f.position() : 0;
	return 0;
}

SD_FILE *
sd_fopen(
	char	*filename,
	char	*mode
) {
	uint8_t			operation;
	ion_boolean_t	seek_start = boolean_false;

	if ((strcmp(mode, "r") == 0) || (strcmp(mode, "rb") == 0)) {
		/*	Open a file for reading. The file must exist. */
		/* check to see if file exists */

		if (!SD.exists(filename)) {
			return NULL;
		}

		operation = FILE_READ;
	}
	else if ((strcmp(mode, "w") == 0) || (strcmp(mode, "wb") == 0)) {
		/* Create an empty file for writing. */
		/* If a file with the same name already exists */
		/* its content is erased and the file is */
		/* considered as a new empty file. */

		if (SD.exists(filename)) {
			SD.remove(filename);
		}

		operation = (O_WRITE | O_TRUNC | O_CREAT);
		/* Open a file for update both reading and writing. The file must exist. */
	}
	else if (strstr(mode, "r+") != NULL) {
		if (!SD.exists(filename)) {
			return NULL;
		}

		operation	= O_RDWR;
		seek_start	= boolean_true;
	}
	/* Create an empty file for both reading and writing. */
	else if (strstr(mode, "w+") != NULL) {
		if (SD.exists(filename)) {
			SD.remove(filename);
		}

		operation = (O_RDWR | O_CREAT | O_TRUNC);
	}
	else if (strcmp(mode, "a+") == 0) {
		operation = (O_RDWR | O_APPEND | O_CREAT);
	}
	else {
		return 0;	/*incorrect args */
	}

	_SD_File *file = new struct _SD_File ();

	(file)->f = SD.open(filename, operation);

	if (!((file)->f)) {
		return 0;
	}

	if (seek_start) {
		file->f.seek(0);
	}

	return file;
}

size_t
sd_fread(
	void	*ptr,
	size_t	size,
	size_t	nmemb,
	SD_FILE *stream
) {
	/* read is the size of bytes * num of size-bytes */
	int num_bytes = stream->f.read((char *) ptr, size * nmemb);

#if DEBUG
	Serial.print("Bytes read : ");
	Serial.println(num_bytes);
#endif

	if (num_bytes < size * nmemb) {
#if DEBUG
		Serial.println("End of file");
#endif
		stream->eof = 1;
	}

	return num_bytes / size;
}

int
sd_fseek(
	SD_FILE		*stream,
	long int	offset,
	int			whence
) {
	if (NULL == stream) {
		return -1;
	}

	unsigned long	cur_pos = stream->f.position();
	unsigned long	cur_end = stream->f.size();

	stream->eof = 0;

	switch (whence) {
		case SEEK_SET:	/* seek from current position */
		{
			if (offset < 0) {
				return -1;	/* can't seek before file */
			}

			if (offset > cur_end) {
				if (!stream->f.seek(cur_end)) {
					return -1;
				}

				unsigned long	bytes_to_pad	= offset - cur_end;
				char			payload			= 0x0;
				size_t			num_written		= sd_fwrite(&payload, sizeof(payload), bytes_to_pad, stream);

				if (num_written != bytes_to_pad) {
					return -1;
				}

				/* The file-position indicator has been implicitly moved by the write - no seek needed to be done now */
				return 0;
			}

			return stream->f.seek(offset) ? 0 : -1;
			break;
		}

		case SEEK_CUR: {
			if (offset + cur_pos > cur_end) {
				if (!stream->f.seek(cur_end)) {
					return -1;
				}

				unsigned long	bytes_to_pad	= (offset + cur_pos) - cur_end;
				char			payload			= 0x0;
				size_t			num_written		= sd_fwrite(&payload, sizeof(payload), bytes_to_pad, stream);

				if (num_written != bytes_to_pad) {
					return -1;
				}

				/* The file-position indicator has been implicitly moved by the write - no seek needed to be done now */
				return 0;
			}

			if (offset - cur_pos < 0) {
				return -1;	/* too far past beginning of the file - assumes that offset is negative */
			}

			return stream->f.seek(cur_pos + offset) ? 0 : -1;
			break;
		}

		case SEEK_END: {
			if (offset > 0) {
				if (!stream->f.seek(cur_end)) {
					return -1;
				}

				unsigned long	bytes_to_pad	= offset;
				char			payload			= 0x0;
				size_t			num_written		= sd_fwrite(&payload, sizeof(payload), bytes_to_pad, stream);

				if (num_written != bytes_to_pad) {
					return -1;
				}

				/* The file-position indicator has been implicitly moved by the write - no seek needed to be done now */
				return 0;
			}

			return stream->f.seek(cur_end + offset) ? 0 : -1;
			break;
		}

		default:
			return -1;
	}
}

long int
sd_ftell(
	SD_FILE *stream
) {
	long int pos = (stream) ? stream->f.position() : -1;

#if DEBUG
	Serial.print("cur pos: ");
	Serial.println(pos);
#endif
	return pos;
}

size_t
sd_fwrite(
	void	*ptr,
	size_t	size,
	size_t	nmemb,
	SD_FILE *stream
) {
	size_t	idx;
	size_t	bytes_written	= 0;
	size_t	total_count		= 0;

	/* for each element, continue to write */
	for (idx = 0; idx < nmemb; idx++) {
		bytes_written = stream->f.write((uint8_t *) ptr, size);

		if (bytes_written != size) {
			return total_count;
		}

		total_count += 1;
	}

	if (total_count != nmemb) {
		return -1;
	}
	else {
		return total_count;
	}
}

int
sd_remove(
	char *filename
) {
	return SD.remove(filename) ? 0 : 1;
}

void
sd_rewind(
	SD_FILE *stream
) {
	stream->eof = 0;
	stream->f.seek(0);
}

int
SD_File_Begin(
	uint8_t csPin
) {
	return SD.begin(csPin);
}

void
sd_printint(
	int i
) {
	Serial.println(i);
}

int
SD_File_Exists(
	char *filepath
) {
	return (int) (SD.exists(filepath));
}

int
SD_File_Delete_All(
) {
	File root = SD.open("/");

	while (true) {
		File entry = root.openNextFile();

		if (!entry) {
			break;
		}

		entry.close();

		bool is_ok = SD.remove(entry.name());

		if (!is_ok) {
			return false;
		}
	}

	return true;
}
