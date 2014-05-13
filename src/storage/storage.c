/******************************************************************************/
/**
@file		storage.c
@author		Graeme Douglas
@brief		Storage handling implementation.
@see		For more information, reference @ref storage.h.
*/
/******************************************************************************/

#include "storage.h"

status_t
chunk_read(
	chunk_handler_t	*handler,
	byte		*data,
	offset_t	idx
)
{
	switch(handler->type) {
	case CHUNKTYPE_INMEMORY:
		return memchunk_read((memchunk_handler_t*)handler, data, idx);
	default:
		return (status_t){ERROR_UNDEFINEDCHUNKTYPE, 0};
	}
}

status_t
chunk_write(
	chunk_handler_t	*handler,
	byte		*data,
	offset_t	idx
)
{
	switch(handler->type) {
	case CHUNKTYPE_INMEMORY:
		return memchunk_write((memchunk_handler_t*)handler, data, idx);
	default:
		return (status_t){ERROR_UNDEFINEDCHUNKTYPE, 0};
	}
}

status_t
chunk_append(
	chunk_handler_t	*handler,
	byte		*data
)
{
	switch(handler->type) {
	case CHUNKTYPE_INMEMORY:
		return (status_t){ERROR_UNSUPPORTED};
	default:
		return (status_t){ERROR_UNDEFINEDCHUNKTYPE, 0};
	}
}

#if 0
status_t
chunk_create(
	char		*name,
	chunk_type_t	type,
	my_size_t	numitems,
	my_size_t	itemsize,
)
{
	status_t status;
	void referencep;
	
	if (CHUNKTYPE_INMEMORY == type)
	{
		status = memchunk_create(name, numitems, itemsize, segmentp);
	}
	else
	{
		status = {ERROR_UNSUPPORTED, 0};
	}
	
	return status;
}
#endif
