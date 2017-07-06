/******************************************************************************/
/**
@file		ion_file.h
@author		Graeme Douglas
@brief		A file API for the ionDB.
@todo		Include support for other file systems (Arduino, Contiki).
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

#if !defined(ION_FILE_H_)
#define ION_FILE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../key_value/kv_system.h"

typedef long ion_file_offset_t;

#define ION_FILE_START	SEEK_SET
#define ION_FILE_END	SEEK_END

#if defined(ARDUINO)

#include "sd_stdio_c_iface.h"

#define ION_NOFILE \
	((ion_file_handle_t) { NULL, -1 } \
	)

typedef struct file_handle {
	SD_FILE *file;
} ion_file_handle_t;

#else /* Clause ARDUINO */

#include "stdio.h"
#include "unistd.h"

typedef FILE *ion_file_handle_t;

#define ION_NOFILE ((ion_file_handle_t) (NULL))

#endif /* Clause ARDUINO */

#define ION_FILE_NULL -1

ion_boolean_t
ion_fexists(
	char *name
);

ion_file_handle_t
ion_fopen(
	char *name
);

ion_err_t
ion_fclose(
	ion_file_handle_t file
);

ion_err_t
ion_fremove(
	char *name
);

ion_err_t
ion_fseek(
	ion_file_handle_t	file,
	ion_file_offset_t	seek_to,
	int					origin
);

ion_file_offset_t
ion_ftell(
	ion_file_handle_t file
);

ion_file_offset_t
ion_fend(
	ion_file_handle_t file
);

ion_err_t
ion_fwrite(
	ion_file_handle_t	file,
	unsigned int		num_bytes,
	ion_byte_t			*to_write
);

ion_err_t
ion_fwrite_at(
	ion_file_handle_t	file,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*to_write
);

ion_err_t
ion_fread(
	ion_file_handle_t	file,
	unsigned int		num_bytes,
	ion_byte_t			*write_to
);

ion_err_t
ion_fread_at(
	ion_file_handle_t	file,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*write_to
);

#if defined(__cplusplus)
}
#endif

#endif
