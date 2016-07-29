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
#include "../../key_value/kv_system.h"

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
@param[in]	  flat_file
					Which flat file instance to scan.
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
*/
ion_err_t
flat_file_scan(
	ion_flat_file_t				*flat_file,
	ion_fpos_t					*location,
	ion_boolean_t				scan_forwards,
	ion_flat_file_predicate_t	predicate
) {
	return err_not_implemented;
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
	ion_value_t					value
) {
	UNUSED(flat_file);
	UNUSED(key);
	UNUSED(value);

	return FLAT_FILE_STATUS_EMPTY == row_status;
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
	if (0 != fseek(flat_file->data_file, location, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fwrite(&row_status, sizeof(row_status), 1, flat_file->data_file)) {
		return err_file_write_error;
	}

	if (NULL != key) {
		if (1 != fwrite(key, flat_file->super.record.key_size, 1, flat_file->data_file)) {
			return err_file_write_error;
		}
	}

	if (NULL != value) {
		if (1 != fwrite(value, flat_file->super.record.value_size, 1, flat_file->data_file)) {
			return err_file_write_error;
		}
	}

	return err_ok;
}

ion_status_t
flat_file_insert(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t status		= ION_STATUS_INITIALIZE;
	/* TODO: Need to factor in empty spots (overwrite them), sorted order insert, and reading buffer size rows at a time */
	ion_fpos_t	insert_loc	= -1;
	ion_err_t	err			= flat_file_scan(flat_file, &insert_loc, boolean_true, flat_file_predicate_empty);

	if (err_ok != err) {
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

	return status;
}
