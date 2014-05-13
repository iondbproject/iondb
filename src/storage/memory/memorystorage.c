#include "memorystorage.h"

status_t
memchunk_read(
	memchunk_handler_t	*handler,
	byte			*data,
	offset_t		idx
)
{
	if (idx >= handler->super.numitems)
		return (status_t){ERROR_OUTSIDEOFCHUNK, 0};
	
	offset_t i = 0;
	
	for (i = 0; i < handler->super.itemsize; ++i)
	{
		data[i] = handler->segment[idx+i];
	}
	
	return (status_t){ERROR_NONE, handler->super.itemsize};
}

status_t
memchunk_write(
	memchunk_handler_t	*handler,
	byte			*data,
	offset_t		idx
)
{
	if (idx >= handler->super.numitems)
		return (status_t){ERROR_OUTSIDEOFCHUNK, 0};
	
	offset_t i = 0;
	
	for (i = 0; i < handler->super.itemsize; ++i)
	{
		handler->segment[idx+i] = data[i];
	}
	
	return (status_t){ERROR_NONE, handler->super.itemsize};
}

status_t
memchunk_create_segment(
	byte			**segment,
	my_size_t		numitems,
	my_size_t		itemsize
)
{
	*segment = malloc(numitems*itemsize);
	
	if (NULL == *segment)
		return (status_t){ERROR_CANNOTALLOCATEMEMORY, 0};
	
	return (status_t){ERROR_NONE, 0};
}

status_t
memchunk_init_handler(
	memchunk_handler_t	*handler,
	byte*			segment,
	my_size_t		numitems,
	my_size_t		itemsize
)
{
	handler->segment	= segment;
	
	handler->super.type	= CHUNKTYPE_INMEMORY;
	handler->super.numitems	= numitems;
	handler->super.itemsize	= itemsize;
	
	return (status_t){ERROR_NONE, 0};
}

status_t
memchunk_destroy_segment(
	byte			**segment
)
{
	if (NULL != segment)
		free(*segment);
	
	*segment = NULL;
	
	return (status_t){ERROR_NONE, 0};
}
