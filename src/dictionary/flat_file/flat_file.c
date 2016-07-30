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

/**
@brief			Given the ID and a buffer to write to, writes back the formatted filename
				for this flat file instance to the given @p str.
@param[in]		id
					Given ID to use to generate a unique filename.
@param[out]		str
					Char buffer to write-back into. This must be allocated memory.
@return			How many characters would have been written. It is a good idea to check that this does not exceed
				@ref ION_MAX_FILENAME_LENGTH.
*/
int
flat_file_get_filename(
	ion_dictionary_id_t id,
	char				*str
) {
	return snprintf(str, ION_MAX_FILENAME_LENGTH, "%d.ffs", id);
}

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
		/* TODO: Should we instead just clamp at 1 instead of failing? */
		return err_invalid_initial_size;/* Can't have a negative dictionary size for the flat file */
	}

	flat_file->super.key_type			= key_type;
	flat_file->super.record.key_size	= key_size;
	flat_file->super.record.value_size	= value_size;

	char	filename[ION_MAX_FILENAME_LENGTH];
	int		actual_filename_length = flat_file_get_filename(id, filename);

	if (actual_filename_length >= ION_MAX_FILENAME_LENGTH) {
		return err_dictionary_initialization_failed;
	}

	flat_file->sorted_mode	= boolean_false;/* By default, we don't use sorted mode */
	flat_file->num_buffered = dictionary_size;

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

	return err_ok;
}

ion_err_t
flat_file_destroy(
	ion_flat_file_t *flat_file
) {
	if (0 != fclose(flat_file->data_file)) {
		return err_file_close_error;
	}

	char filename[ION_MAX_FILENAME_LENGTH];

	flat_file_get_filename(flat_file->super.id, filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	flat_file->data_file = NULL;

	return err_ok;
}

/**
@brief			Performs a linear scan of the flat file, going forwards
				if @p scan_forwards is true, writing the first location
				seen that satisfies the given @p predicate to @p location.
@details		If the scan falls through, then the location is written as
				the EOF position of the file. The variadic arguments accepted
				by this function are passed into the predicate additional parameters.
				This can be used to provide additional context to the predicate for use
				in determining whether or not the row is a match.
@param[in]		flat_file
					Which flat file instance to scan.
@param[in]		start_location
					Where to begin the scan. This is given as a row index. If
					given as -1, then it is assumed to be either the start of
					file or end of file, depending on the state of @p scan_forwards.
@param[out]		location
					Allocated memory location to write back the found location to.
					Is not changed in the event of a failure or error condition. This
					location is given back as a row index.
@param[in]		scan_forwards
					Scans front-to-back if @p true, else scans back-to-front.
@param[in]		predicate
					Given test function to check each row against. Once this function
					returns true, the scan is terminated and the found location is written.
@return			Resulting status of scan.
@todo			Try changing the predicate to be an enum-and-switch to eliminate the function
				call. Benchmark the performance gain and decide which strategy to use.
@todo			Consider changing to @p SEEK_CUR whenever possible. Benchmark this and see
				if the performance gain (if any) is worth it.
*/
ion_err_t
flat_file_scan(
	ion_flat_file_t				*flat_file,
	ion_fpos_t					start_location,
	ion_fpos_t					*location,
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

	ion_byte_t read_buffer[flat_file->row_size * flat_file->num_buffered];

	memset(read_buffer, 0, flat_file->row_size * flat_file->num_buffered);

	while (cur_offset != end_offset) {
		fseek(flat_file->data_file, cur_offset, SEEK_SET);

		/* We set cur_offset to be the next block to read after this next code segment, so */
		/* we need t save what block we're currently reading now for location calculation purposes */
		ion_fpos_t	prev_offset				= cur_offset;
		size_t		num_records_to_process	= flat_file->num_buffered;

		if (scan_forwards) {
			/* It's possible for this to do a partial read (if you're close to EOF) */
			/* so we just check that it doesn't read nothing */
			if (0 == (num_records_to_process = fread(read_buffer, flat_file->row_size, flat_file->num_buffered, flat_file->data_file))) {
				return err_file_read_error;
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

			if (num_records_to_process != fread(read_buffer, flat_file->row_size, num_records_to_process, flat_file->data_file)) {
				return err_file_read_error;
			}

			/* In this case, the prev_offset is actually the cur_offset. */
			prev_offset = cur_offset;
		}

		size_t i;

		for (i = 0; i < num_records_to_process; i++) {
			/* This cast is done because its possible for the status change in type */
			size_t						cur_rec		= i * flat_file->row_size;
			ion_flat_file_row_status_t	row_status	= *((ion_flat_file_row_status_t *) &read_buffer[cur_rec]);
			ion_key_t					key			= &read_buffer[cur_rec + sizeof(ion_flat_file_row_status_t)];
			ion_value_t					value		= &read_buffer[cur_rec + sizeof(ion_flat_file_row_status_t) + flat_file->super.record.key_size];

			va_list predicate_arguments;

			va_start(predicate_arguments, predicate);

			ion_boolean_t predicate_test = predicate(flat_file, row_status, key, value, &predicate_arguments);

			va_end(predicate_arguments);

			if (predicate_test) {
				*location = (prev_offset - flat_file->start_of_data) / flat_file->row_size + i;
				return err_ok;
			}
		}
	}

	/* If we reach this point, then no row matched the predicate */
	*location = (eof_pos - flat_file->start_of_data) / flat_file->row_size;
	return err_file_hit_eof;
}

/**
@brief		Predicate function to return any row that is empty or deleted.
@see		ion_flat_file_predicate_t
 */
ion_boolean_t
flat_file_predicate_empty(
	ion_flat_file_t				*flat_file,
	ion_flat_file_row_status_t	row_status,
	ion_key_t					key,
	ion_value_t					value,
	va_list						*args
) {
	UNUSED(flat_file);
	UNUSED(key);
	UNUSED(value);
	UNUSED(args);

	return FLAT_FILE_STATUS_EMPTY == row_status;
}

/**
@brief		Predicate function to return any row that has an exact match to the given target key.
@details	We expect one ion_key_t to be in @p args.
@see		ion_flat_file_predicate_t
 */
ion_boolean_t
flat_file_predicate_key_match(
	ion_flat_file_t				*flat_file,
	ion_flat_file_row_status_t	row_status,
	ion_key_t					key,
	ion_value_t					value,
	va_list						*args
) {
	UNUSED(value);

	ion_key_t target_key = va_arg(*args, ion_key_t);

	return FLAT_FILE_STATUS_OCCUPIED == row_status && 0 == flat_file->super.compare(target_key, key, flat_file->super.record.key_size);
}

/**
@brief		Writes the given row out to the data file.
@details	If the key or value is given as @p NULL, then no write will be performed
			for that @p NULL key/value. This can be used to perform a status-only write
			by passing in @p NULL for both the key and value.
@param[in]	flat_file
				Which flat file instance to write to.
@param[in]	location
				Which row index to write to. This function will compute
				the file offset of the row index.
@param[in]	row_status
				Given status to write into the row.
@param[in]	key
				Given key to write into the row.
@param[in]	value
				Given value to write into the row.
@return		Resulting status of the file operations.
*/
ion_err_t
flat_file_write_row(
	ion_flat_file_t				*flat_file,
	ion_fpos_t					location,
	ion_flat_file_row_status_t	row_status,
	ion_key_t					key,
	ion_value_t					value
) {
	if (0 != fseek(flat_file->data_file, location * flat_file->row_size, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fwrite(&row_status, sizeof(row_status), 1, flat_file->data_file)) {
		return err_file_write_error;
	}

	if ((NULL != key) && (1 != fwrite(key, flat_file->super.record.key_size, 1, flat_file->data_file))) {
		return err_file_write_error;
	}

	if ((NULL != value) && (1 != fwrite(value, flat_file->super.record.value_size, 1, flat_file->data_file))) {
		return err_file_write_error;
	}

	return err_ok;
}

ion_status_t
flat_file_insert(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
) {
	/* TODO: Need to factor in empty spots (overwrite them), sorted order insert, and reading buffer size rows at a time */
	ion_status_t	status		= ION_STATUS_INITIALIZE;
	ion_fpos_t		insert_loc	= -1;
	ion_err_t		err			= flat_file_scan(flat_file, -1, &insert_loc, boolean_true, flat_file_predicate_empty);

	if ((err_ok != err) && (err_file_hit_eof != err)) {
		status.error = err;
		return status;
	}

	err = flat_file_write_row(flat_file, insert_loc, FLAT_FILE_STATUS_OCCUPIED, key, value);

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
