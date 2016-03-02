/******************************************************************************/
/**
@file		linkedfilebag.h
@author		Graeme Douglas
@brief		API for a persistent bag. Items linked together in a singly
		linked list.
*/
/******************************************************************************/

#ifndef LINKEDFILEBAG_H
#define LINKEDFILEBAG_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "./../kv_system.h"
#include "ion_file.h"

#define LFB_NULL	FILE_NULL

typedef struct linkedfilebag
{
	file_handle_t	file_handle;
	file_offset_t	next_empty;
} lfb_t;

err_t
lfb_put(
	lfb_t		*bag,
	byte		*to_write,
	unsigned int	num_bytes,
	file_offset_t	next,
	file_offset_t	*wrote_at
);

err_t
lfb_get(
	lfb_t		*bag,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*write_to,
	file_offset_t	*next
);

err_t
lfb_update_next(
	lfb_t		*bag,
	file_offset_t	offset,
	file_offset_t	next
);

err_t
lfb_delete(
	lfb_t		*bag,
	file_offset_t	offset
);

err_t
lfb_delete_all(
	lfb_t		*bag,
	file_offset_t	offset
);

err_t
lfb_update(
	lfb_t		*bag,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*to_write,
	file_offset_t	*next
);

err_t
lfb_update_all(
	lfb_t		*bag,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*to_write
);

#ifdef  __cplusplus
}
#endif

#endif
