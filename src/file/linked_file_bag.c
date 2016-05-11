
#include "linked_file_bag.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

err_t
lfb_put(
	lfb_t		*bag,
	byte		*to_write,
	unsigned int	num_bytes,
	file_offset_t	next,
	file_offset_t	*wrote_at
)
{
	file_offset_t	next_empty;
	err_t		error;
	
	next_empty		= LFB_NULL;
	
	if (LFB_NULL != bag->next_empty)
	{
		error		= ion_fread_at(
					bag->file_handle,
					bag->next_empty,
					sizeof(file_offset_t),
					(byte *)&next_empty
				);
		
		if (err_ok != error)
		{
			return error;
		}
		
		*wrote_at	= bag->next_empty;
	}
	else
	{
		*wrote_at	= ion_fend(bag->file_handle);
	}	
	
	error		= ion_fwrite_at(
				bag->file_handle,
				*wrote_at,
				sizeof(file_offset_t),
				(byte *)&next
			);
	
	if (err_ok != error)
	{
		return error;
	}
	
	error		= ion_fwrite_at(
				bag->file_handle,
				*wrote_at + sizeof(file_offset_t),
				num_bytes,
				to_write
			);
	
	if (err_ok != error)
	{
		return error;
	}
	
	bag->next_empty	= next_empty;

	return err_ok;
}

err_t
lfb_get(
	lfb_t		*bag,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*write_to,
	file_offset_t	*next
)
{
	err_t		error;
	
	error	= ion_fread_at(
			bag->file_handle,
			offset,
			sizeof(file_offset_t),
			(byte *)next
		);
	
	if (err_ok != error)
	{
		return error;
	}

	error	= ion_fread_at(
			bag->file_handle,
			offset+sizeof(file_offset_t),
			num_bytes,
			write_to
		);
	
	return error;
}

err_t
lfb_update_next(
	lfb_t		*bag,
	file_offset_t	offset,
	file_offset_t	next
)
{
	err_t		error;
	
	error		= ion_fwrite_at(
				bag->file_handle,
				offset,
				sizeof(file_offset_t),
				(byte *)&(next)
			);
	
	if (err_ok == error)
	{
		bag->next_empty	= offset;
	}
	
	return error;
}

err_t
lfb_delete(
	lfb_t		*bag,
	file_offset_t	offset
)
{
	return lfb_update_next(bag, offset, bag->next_empty);
}

err_t
lfb_delete_all(
	lfb_t		*bag,
	file_offset_t	offset
)
{
	err_t		error;
	file_offset_t	next;
	
	while (LFB_NULL != offset)
	{
		error	= ion_fread_at(
				bag->file_handle,
				offset,
				sizeof(file_offset_t),
				(byte *)&next
			);
		
		if (err_ok != error)
		{
			return error;
		}
		
		error	= lfb_delete(bag, offset);
		
		if (err_ok != error)
		{
			return error;
		}
		
		offset	= next;
	}
	
	return err_ok;
}

err_t
lfb_update(
	lfb_t		*bag,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*to_write,
	file_offset_t	*next
)
{
	err_t	error;
	
	if (NULL != next)
	{
		error = lfb_update_next(bag, offset, *next);
		
		if (err_ok != error)
			return error;
	}
	
	error		= ion_fwrite_at(
				bag->file_handle,
				offset + sizeof(file_offset_t),
				num_bytes,
				to_write
			);
	
	return error;
}

err_t
lfb_update_all(
	lfb_t		*bag,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*to_write
)
{
	err_t		error;
	file_offset_t	next;
	
	while (LFB_NULL != offset)
	{
		error	= ion_fread_at(
				bag->file_handle,
				offset,
				sizeof(file_offset_t),
				(byte *)&next
			);
		
		if (err_ok != error)
		{
			return error;
		}
		
		error	= lfb_update(bag, offset, num_bytes, to_write, NULL);
		
		if (err_ok != error)
		{
			return error;
		}
		
		offset	= next;
	}
	
	return err_ok;
}
