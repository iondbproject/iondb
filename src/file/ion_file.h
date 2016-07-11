/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		A file API for the ionDB.
@todo		Include support for other file systems (Arduino, Contiki).
*/
/******************************************************************************/

#if !defined(ION_FILE_H_)
#define ION_FILE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../key_value/kv_system.h"

typedef long file_offset_t;

#define ION_FILE_START	SEEK_SET
#define ION_FILE_END	SEEK_END

#if defined(ARDUINO)

#include "SD_stdio_c_iface.h"

#define ION_NOFILE \
	((file_handle_t) { NULL, -1 } \
	)

typedef struct file_handle {
	SD_FILE *file;
} file_handle_t;

#else /* Clause ARDUINO */

#include "stdio.h"
#include "unistd.h"

typedef FILE *file_handle_t;

#define ION_NOFILE ((file_handle_t) (NULL))

#endif /* Clause ARDUINO */

#define FILE_NULL -1

boolean_t
ion_fexists(
	char *name
);

file_handle_t
ion_fopen(
	char *name
);

err_t
ion_fclose(
	file_handle_t file
);

err_t
ion_fremove(
	char *name
);

err_t
ion_fseek(
	file_handle_t	file,
	file_offset_t	seek_to,
	int				origin
);

file_offset_t
ion_ftell(
	file_handle_t file
);

file_offset_t
ion_fend(
	file_handle_t file
);

err_t
ion_fwrite(
	file_handle_t	file,
	unsigned int	num_bytes,
	ion_byte_t		*to_write
);

err_t
ion_fwrite_at(
	file_handle_t	file,
	file_offset_t	offset,
	unsigned int	num_bytes,
	ion_byte_t		*to_write
);

err_t
ion_fappend(
	file_handle_t	file,
	unsigned int	num_bytes,
	ion_byte_t		*to_write
);

err_t
ion_fread(
	file_handle_t	file,
	unsigned int	num_bytes,
	ion_byte_t		*write_to
);

err_t
ion_fread_at(
	file_handle_t	file,
	file_offset_t	offset,
	unsigned int	num_bytes,
	ion_byte_t		*write_to
);

#if defined(__cplusplus)
}
#endif

#endif
