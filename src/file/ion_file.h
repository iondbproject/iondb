/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		A file API for the ionDB.
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
