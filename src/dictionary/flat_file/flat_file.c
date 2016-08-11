/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation specific definitions for the flat file store.
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

#include "flat_file.h"
#include "flat_file_types.h"

ion_err_t
flat_file_initialize(
	ion_flat_file_t			*flat_file,
	ion_dictionary_id_t		id,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	if (dictionary_size <= 0) {
		/* Clamp the dictionary size since we always need at least 1 row to buffer */
		dictionary_size = 1;
	}

	flat_file->super.key_type			= key_type;
	flat_file->super.record.key_size	= key_size;
	flat_file->super.record.value_size	= value_size;

	char	filename[ION_MAX_FILENAME_LENGTH];
	int		actual_filename_length = dictionary_get_filename(id, "ffs", filename);

	if (actual_filename_length >= ION_MAX_FILENAME_LENGTH) {
		return err_dictionary_initialization_failed;
	}

	flat_file->sorted_mode	= boolean_false;/* By default, we don't use sorted mode */
	flat_file->num_buffered = dictionary_size;	/* TODO: Sorted mode needs to be written out as a header? */

	flat_file->data_file	= fopen(filename, "r+b");

	if (NULL == flat_file->data_file) {
		/* The file did not exist - lets open to write */
		flat_file->data_file = fopen(filename, "w+b");

		if (NULL == flat_file->data_file) {
			/* Failed to open, even to create */
			return err_file_open_error;
		}
	}

	flat_file->start_of_data = ftell(flat_file->data_file);	/* For now, we don't have any header information. */

	if (-1 == flat_file->start_of_data) {
		return err_file_read_error;
	}

	/* A record is laid out as: | STATUS |	  KEY	 |	   VALUE	  | */
	/*				   Bytes:	(1)	 (key_size)   (value_size)	*/
	flat_file->row_size = sizeof(ion_flat_file_row_status_t) + key_size + value_size;
	flat_file->buffer	= calloc(flat_file->num_buffered, flat_file->row_size);

	return err_ok;
}

ion_err_t
flat_file_destroy(
	ion_flat_file_t *flat_file
) {
	ion_err_t err = flat_file_close(flat_file);

	if (err_ok != err) {
		return err;
	}

	char filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(flat_file->super.id, "ffs", filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	flat_file->data_file = NULL;

	return err_ok;
}

ion_err_t
flat_file_scan(
	ion_flat_file_t				*flat_file,
	ion_fpos_t					start_location,
	ion_fpos_t					*location,
	ion_flat_file_row_t			*row,
	ion_boolean_t				scan_forwards,
	ion_flat_file_predicate_t	predicate,
	...
) {
	ion_fpos_t eof_pos = 0;

	if (0 != fseek(flat_file->data_file, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	if (-1 == (eof_pos = ftell(flat_file->data_file))) {
		return err_file_read_error;
	}

	ion_fpos_t	cur_offset	= start_location * flat_file->row_size;
	ion_fpos_t	end_offset	= scan_forwards ? eof_pos : flat_file->start_of_data;

	if (-1 == start_location) {
		if (scan_forwards) {
			cur_offset = flat_file->start_of_data;
		}
		else {
			cur_offset = eof_pos;
		}
	}

	if ((cur_offset > eof_pos) || (cur_offset < flat_file->start_of_data)) {
		return err_out_of_bounds;
	}

	/* This line is likely not needed, as long as we're careful to only read good data */
	/* memset(read_buffer, 0, flat_file->row_size * flat_file->num_buffered); */

	while (cur_offset != end_offset) {
		if (0 != fseek(flat_file->data_file, cur_offset, SEEK_SET)) {
			return err_file_bad_seek;
		}

		/* We set cur_offset to be the next block to read after this next code segment, so */
		/* we need t save what block we're currently reading now for location calculation purposes */
		ion_fpos_t	prev_offset				= cur_offset;
		size_t		num_records_to_process	= flat_file->num_buffered;

		if (scan_forwards) {
			/* It's possible for this to do a partial read (if you're close to EOF) */
			/* so we just check that it doesn't read nothing */
			if (0 == (num_records_to_process = fread(flat_file->buffer, flat_file->row_size, flat_file->num_buffered, flat_file->data_file))) {
				return err_file_incomplete_read;
			}

			if (-1 == (cur_offset = ftell(flat_file->data_file))) {
				return err_file_read_error;
			}
		}
		else {
			/* Move the offset pointer to the next read location, clamp it at start_of_file if we go too far */
			cur_offset -= flat_file->row_size * flat_file->num_buffered;

			if (cur_offset < flat_file->start_of_data) {
				/* We know how many rows we went past the start of file, calculate it so we don't fread too much */
				num_records_to_process	= flat_file->num_buffered - (flat_file->start_of_data - cur_offset) / flat_file->row_size;
				cur_offset				= flat_file->start_of_data;
			}

			if (0 != fseek(flat_file->data_file, cur_offset, SEEK_SET)) {
				return err_file_bad_seek;
			}

			if (num_records_to_process != fread(flat_file->buffer, flat_file->row_size, num_records_to_process, flat_file->data_file)) {
				return err_file_incomplete_read;
			}

			/* In this case, the prev_offset is actually the cur_offset. */
			prev_offset = cur_offset;
		}

		size_t i;

		for (i = 0; i < num_records_to_process; i++) {
			size_t cur_rec = i * flat_file->row_size;

			/* This cast is done because it's possible for the status change in type */
			row->row_status = *((ion_flat_file_row_status_t *) &flat_file->buffer[cur_rec]);
			row->key		= &flat_file->buffer[cur_rec + sizeof(ion_flat_file_row_status_t)];
			row->value		= &flat_file->buffer[cur_rec + sizeof(ion_flat_file_row_status_t) + flat_file->super.record.key_size];

			va_list predicate_arguments;

			va_start(predicate_arguments, predicate);

			ion_boolean_t predicate_test = predicate(flat_file, row, &predicate_arguments);

			va_end(predicate_arguments);

			if (predicate_test) {
				*location = (prev_offset - flat_file->start_of_data) / flat_file->row_size + i;
				return err_ok;
			}
		}
	}

	/* If we reach this point, then no row matched the predicate. */
	*location = (eof_pos - flat_file->start_of_data) / flat_file->row_size;
	return err_file_hit_eof;
}

/**
@brief		Predicate function to return any row that is empty or deleted.
@see		ion_flat_file_predicate_t
*/
ion_boolean_t
flat_file_predicate_empty(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
) {
	UNUSED(flat_file);
	UNUSED(args);

	return FLAT_FILE_STATUS_EMPTY == row->row_status;
}

ion_boolean_t
flat_file_predicate_not_empty(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
) {
	UNUSED(flat_file);
	UNUSED(args);

	return FLAT_FILE_STATUS_OCCUPIED == row->row_status;
}

ion_boolean_t
flat_file_predicate_key_match(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
) {
	ion_key_t target_key = va_arg(*args, ion_key_t);

	return FLAT_FILE_STATUS_OCCUPIED == row->row_status && 0 == flat_file->super.compare(target_key, row->key, flat_file->super.record.key_size);
}

ion_boolean_t
flat_file_predicate_within_bounds(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
) {
	ion_key_t	lower_bound = va_arg(*args, ion_key_t);
	ion_key_t	upper_bound = va_arg(*args, ion_key_t);

	return FLAT_FILE_STATUS_OCCUPIED == row->row_status && flat_file->super.compare(row->key, lower_bound, flat_file->super.record.key_size) >= 0 && flat_file->super.compare(row->key, upper_bound, flat_file->super.record.key_size) <= 0;
}

/**
@brief		Writes the given row out to the data file.
@details	If the key or value is given as @p NULL, then no write will be performed
			for that @p NULL key/value. This can be used to perform a status-only write
			by passing in @p NULL for both the key and value. **NOTE:** The alignment
			of the write is dependent on the occurence of the writes that come before
			it. This means that the @p key cannot be @p NULL while the value is not @p NULL.
@param[in]	flat_file
				Which flat file instance to write to.
@param[in]	location
				Which row index to write to. This function will compute
				the file offset of the row index.
@param[in]	row
				Given row to write out at the destined @p location.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
flat_file_write_row(
	ion_flat_file_t		*flat_file,
	ion_fpos_t			location,
	ion_flat_file_row_t *row
) {
	if (0 != fseek(flat_file->data_file, location * flat_file->row_size, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fwrite(&row->row_status, sizeof(row->row_status), 1, flat_file->data_file)) {
		return err_file_incomplete_write;
	}

	if ((NULL != row->key) && (1 != fwrite(row->key, flat_file->super.record.key_size, 1, flat_file->data_file))) {
		return err_file_incomplete_write;
	}

	if ((NULL != row->value) && (1 != fwrite(row->value, flat_file->super.record.value_size, 1, flat_file->data_file))) {
		return err_file_incomplete_write;
	}

	return err_ok;
}

ion_err_t
flat_file_read_row(
	ion_flat_file_t		*flat_file,
	ion_fpos_t			location,
	ion_flat_file_row_t *row
) {
	if (0 != fseek(flat_file->data_file, location * flat_file->row_size, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fread(flat_file->buffer, sizeof(row->row_status), 1, flat_file->data_file)) {
		return err_file_incomplete_write;
	}

	if (1 != fread(flat_file->buffer + sizeof(row->row_status), flat_file->super.record.key_size, 1, flat_file->data_file)) {
		return err_file_incomplete_write;
	}

	if (1 != fread(flat_file->buffer + sizeof(row->row_status) + flat_file->super.record.key_size, flat_file->super.record.value_size, 1, flat_file->data_file)) {
		return err_file_incomplete_write;
	}

	row->row_status = *((ion_flat_file_row_status_t *) flat_file->buffer);
	row->key		= flat_file->buffer + sizeof(ion_flat_file_row_status_t);
	row->value		= flat_file->buffer + sizeof(ion_flat_file_row_status_t) + flat_file->super.record.key_size;

	return err_ok;
}

ion_status_t
flat_file_insert(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t		status		= ION_STATUS_INITIALIZE;
	ion_fpos_t			insert_loc	= -1;
	ion_flat_file_row_t row;
	ion_err_t			err			= flat_file_scan(flat_file, -1, &insert_loc, &row, boolean_true, flat_file_predicate_empty);

	if ((err_ok != err) && (err_file_hit_eof != err)) {
		status.error = err;
		return status;
	}

	err = flat_file_write_row(flat_file, insert_loc, &(ion_flat_file_row_t) { FLAT_FILE_STATUS_OCCUPIED, key, value });

	if (err_ok != err) {
		status.error = err;
		return status;
	}

	if (flat_file->sorted_mode) {
		/* TODO: Do the thing */
	}

	status.error	= err_ok;
	status.count	= 1;
	return status;
}

ion_status_t
flat_file_get(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t status = ION_STATUS_INITIALIZE;

	if (!flat_file->sorted_mode) {
		ion_fpos_t			found_loc	= -1;
		ion_flat_file_row_t row;
		ion_err_t			err			= flat_file_scan(flat_file, -1, &found_loc, &row, boolean_true, flat_file_predicate_key_match, key);

		if (err_ok != err) {
			status.error = err;

			if (err_file_hit_eof == err) {
				/* Alias the error since in this case, since hitting the EOF signifies */
				/* that we didn't find what we were looking for */
				status.error = err_item_not_found;
			}

			return status;
		}

		memcpy(value, row.value, flat_file->super.record.value_size);
		status.error	= err_ok;
		status.count	= 1;
	}
	else {
		/* TODO: Do the thing */
	}

	return status;
}

ion_status_t
flat_file_delete(
	ion_flat_file_t *flat_file,
	ion_key_t		key
) {
	ion_status_t status = ION_STATUS_INITIALIZE;

	if (!flat_file->sorted_mode) {
		ion_fpos_t			loc = -1;
		ion_flat_file_row_t row;
		ion_err_t			err;

		while (err_ok == (err = flat_file_scan(flat_file, loc, &loc, &row, boolean_true, flat_file_predicate_key_match, key))) {
			flat_file_write_row(flat_file, loc, &(ion_flat_file_row_t) { FLAT_FILE_STATUS_EMPTY, NULL, NULL });
			status.count++;
			/* Move the loc one-forwards so that we skip the one we just deleted */
			loc++;
		}

		status.error = err_ok;

		if ((err == err_file_hit_eof) && (status.count == 0)) {
			status.error = err_item_not_found;
		}
		else if (err != err_file_hit_eof) {
			status.error = err;
		}

		return status;
	}
	else {
		/* TODO: Do the thing */
	}

	return status;
}

ion_status_t
flat_file_update(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t status = ION_STATUS_INITIALIZE;

	if (!flat_file->sorted_mode) {
		ion_fpos_t			loc = -1;
		ion_flat_file_row_t row;
		ion_err_t			err;

		while (err_ok == (err = flat_file_scan(flat_file, loc, &loc, &row, boolean_true, flat_file_predicate_key_match, key))) {
			flat_file_write_row(flat_file, loc, &(ion_flat_file_row_t) { FLAT_FILE_STATUS_OCCUPIED, key, value });
			status.count++;
			/* Move one-forwards to skip the one we just updated */
			loc++;
		}

		status.error = err_ok;

		if ((err == err_file_hit_eof) && (status.count == 0)) {
			/* If this is the case, then we had nothing to update. Do an upsert instead */
			return flat_file_insert(flat_file, key, value);
		}
		else if (err != err_file_hit_eof) {
			status.error = err;
		}

		return status;
	}
	else {
		/* TODO: Do the thing */
	}

	return status;
}

ion_err_t
flat_file_close(
	ion_flat_file_t *flat_file
) {
	free(flat_file->buffer);
	flat_file->buffer = NULL;

	if (0 != fclose(flat_file->data_file)) {
		return err_file_close_error;
	}

	return err_ok;
}
