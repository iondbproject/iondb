/******************************************************************************/
/**
@file		ion_file.h
@author		Graeme Douglas
@brief		A file API for the ionDB.
@todo		Include support for other file systems (Arduino, Contiki).
*/
/******************************************************************************/

#ifndef ION_FILE_H
#define ION_FILE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "./../kv_system.h"

#define FS_TARGET_CFILE		0
#define FS_TARGET_ARDUINO	1
#define FS_TARGET 		FS_TARGET_CFILE

typedef long		file_offset_t;

#if FS_TARGET == FS_TARGET_ARDUINO

#include "SD_c_iface.h"

typedef struct file_handle
{
	SD_File*	file;
	file_offset_t	end;
} file_handle_t;

#define ION_FILE_START	0
#define ION_FILE_END	1

#else

#include "stdio.h"
#include "unistd.h"

typedef FILE*		file_handle_t;

#define ION_FILE_START	SEEK_SET
#define ION_FILE_END	SEEK_END

#endif

#define FILE_NULL	-1

boolean_t
ion_fexists(
	char		*name
);

file_handle_t
ion_fopen(
	char		*name
);

err_t
ion_fclose(
	file_handle_t	file
);

err_t
ion_fremove(
	char		*name
);

err_t
ion_fseek(
	file_handle_t	file,
	file_offset_t	seek_to,
	int		origin
);

file_offset_t
ion_ftell(
	file_handle_t	file
);

file_offset_t
ion_fend(
	file_handle_t	file
);

err_t
ion_fwrite(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte*		to_write
);

err_t
ion_fwrite_at(
	file_handle_t	file,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*to_write
);

err_t
ion_fappend(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte*		to_write
);

err_t
ion_fread(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte		*write_to
);

err_t
ion_fread_at(
	file_handle_t	file,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte*		write_to
);

#ifdef  __cplusplus
}
#endif

#endif
