/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		API for a persistent bag. Items are linked together in a singly
			linked list.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
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
