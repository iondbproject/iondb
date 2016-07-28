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
@param[in,out]	str
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
