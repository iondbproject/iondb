/******************************************************************************/
/**
@file		linked_file_bag.c
@author		Graeme Douglas
@brief		API for a persistent bag. Items are linked together in a singly
			linked list.
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

#include "linked_file_bag.h"

#if !defined(ION_NULL)
#define ION_NULL ((void *) 0)
#endif

ion_err_t
lfb_put(
	ion_lfb_t			*bag,
	ion_byte_t			*to_write,
	unsigned int		num_bytes,
	ion_file_offset_t	next,
	ion_file_offset_t	*wrote_at
) {
	ion_file_offset_t	next_empty;
	ion_err_t			error;

	next_empty = ION_LFB_NULL;

	if (ION_LFB_NULL != bag->next_empty) {
		error = ion_fread_at(bag->file_handle, bag->next_empty, sizeof(ion_file_offset_t), (ion_byte_t *) &next_empty);

		if (err_ok != error) {
			return error;
		}

		*wrote_at = bag->next_empty;
	}
	else {
		*wrote_at = ion_fend(bag->file_handle);
	}

	error = ion_fwrite_at(bag->file_handle, *wrote_at, sizeof(ion_file_offset_t), (ion_byte_t *) &next);

	if (err_ok != error) {
		return error;
	}

	error = ion_fwrite_at(bag->file_handle, *wrote_at + sizeof(ion_file_offset_t), num_bytes, to_write);

	if (err_ok != error) {
		return error;
	}

	bag->next_empty = next_empty;

	return err_ok;
}

ion_err_t
lfb_get(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*write_to,
	ion_file_offset_t	*next
) {
	ion_err_t error;

	error = ion_fread_at(bag->file_handle, offset, sizeof(ion_file_offset_t), (ion_byte_t *) next);

	if (err_ok != error) {
		return error;
	}

	error = ion_fread_at(bag->file_handle, offset + sizeof(ion_file_offset_t), num_bytes, write_to);

	return error;
}

/**
@brief		Update the next offset for the record stored at @p offset.
@param		bag
				A pointer to the initialized handler for the linked file
				bag we wish to update.
@param		offset
				The offset of the record to set the next offset of.
@param		next
				The offset of the record to be referenced in the record
				stored starting at @p offset.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_update_next(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	ion_file_offset_t	next
) {
	ion_err_t error;

	error = ion_fwrite_at(bag->file_handle, offset, sizeof(ion_file_offset_t), (ion_byte_t *) &(next));

	if (err_ok == error) {
		bag->next_empty = offset;
	}

	return error;
}

ion_err_t
lfb_delete(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset
) {
	return lfb_update_next(bag, offset, bag->next_empty);
}

ion_err_t
lfb_delete_all(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	ion_result_count_t	*count
) {
	ion_err_t			error;
	ion_file_offset_t	next;

	while (ION_LFB_NULL != offset) {
		error = ion_fread_at(bag->file_handle, offset, sizeof(ion_file_offset_t), (ion_byte_t *) &next);

		if (err_ok != error) {
			return error;
		}

		error = lfb_delete(bag, offset);

		if (err_ok != error) {
			return error;
		}

		if (NULL != count) {
			(*count)++;
		}

		offset = next;
	}

	return err_ok;
}

ion_err_t
lfb_update(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*to_write,
	ion_file_offset_t	*next
) {
	ion_err_t error;

	if (NULL != next) {
		error = lfb_update_next(bag, offset, *next);

		if (err_ok != error) {
			return error;
		}
	}

	error = ion_fwrite_at(bag->file_handle, offset + sizeof(ion_file_offset_t), num_bytes, to_write);

	return error;
}

ion_err_t
lfb_update_all(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*to_write,
	ion_result_count_t	*count
) {
	ion_err_t			error;
	ion_file_offset_t	next;

	while (ION_LFB_NULL != offset) {
		error = ion_fread_at(bag->file_handle, offset, sizeof(ion_file_offset_t), (ion_byte_t *) &next);

		if (err_ok != error) {
			return error;
		}

		error = lfb_update(bag, offset, num_bytes, to_write, NULL);

		if (err_ok != error) {
			return error;
		}

		if (NULL != count) {
			(*count)++;
		}

		offset = next;
	}

	return err_ok;
}
