/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation specific type definitions for the flat file store.
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

#if !defined(FLAT_FILE_TYPES_H)
#define FLAT_FILE_TYPES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary.h"
#include "../../file/SD_stdio_c_iface.h"

#if defined(ARDUINO)
#include "../../serial/serial_c_iface.h"
#endif

/**
@brief		This type describes the status flag within a flat file row.
*/
typedef ion_byte_t ion_flat_file_row_status_t;

/**
@brief		Signifies that this row in the flat file is currently occupied and should not be overwritten.
*/
#define ION_FLAT_FILE_STATUS_OCCUPIED	1
/**
@brief		Signifies that this row in the flat file is currently empty and is okay to be overwritten.
*/
#define ION_FLAT_FILE_STATUS_EMPTY		0

/**
@brief		Signals to @ref flat_file_scan to scan in a forward direction.
*/
#define ION_FLAT_FILE_SCAN_FORWARDS		1
/**
@brief		Signals to @ref flat_file_scan to scan in a backward direction.
*/
#define ION_FLAT_FILE_SCAN_BACKWARDS	0

/**
@brief		Metadata container that holds flat file specific information.
*/
typedef struct {
	/**> Parent structure that holds dictionary level information. */
	ion_dictionary_parent_t super;
	/**> Flag to toggle whether or not to activate "sorted mode" for storage. */
	ion_boolean_t			sorted_mode;
	/**> This signifies where the actual record data starts, in case we want to
		 write some metadata at the beginning of the flat file's file. */
	ion_fpos_t				start_of_data;
	/**> This marks the eof position within the file, so that we can efficiently find it. */
	ion_fpos_t				eof_position;
	/**> This comes from the given dictionary size, and signifies how many
		 records we want to buffer at a time. This is a trade-off between
		 better performance and increased memory usage. */
	ion_dictionary_size_t	num_buffered;
	/**> Memory buffer capable of holding @p row_size number of rows. This is used
		 for many purposes throughout the flat file. */
	ion_byte_t				*buffer;
	/**> The file descriptor of the file this flat file instance operates on. */
	FILE					*data_file;
	/**> This value expresses the size of one row inside the @p data_file. A row is defined
		 as a record + metadata. Change this if @ref ion_flat_file_row_t changes!*/
	size_t					row_size;
	/**> When a scan is performed, a region (defined as @p num_in_buffer number of records) is loaded into
		 memory. We can utilize this fact to do efficient cached reads as long as the buffer is intact.
		 This is expressed as an index that points to the first record in the region. @p num_in_buffer-1 would
		 be the last index in the region. */
	ion_fpos_t	current_loaded_region;
	/**> Expresses how many valid records are currently in the buffer. */
	size_t		num_in_buffer;
} ion_flat_file_t;

/**
@brief		Container for the rows written in the flat file data file.
*/
typedef struct {
	/**> A flag indicating the status of the row. */
	ion_flat_file_row_status_t	row_status;
	/**> The key stored in this row. */
	ion_key_t					key;
	/**> The value stored in this row. */
	ion_value_t					value;
} ion_flat_file_row_t;

/**
@brief		The function signature of a flat file predicate, used in searches.
*/
typedef ion_boolean_t (*ion_flat_file_predicate_t)(
	ion_flat_file_t *,
	ion_flat_file_row_t *,
	va_list *args
);

/**
@brief		Implementation cursor type for the flat file store cursor.
*/
typedef struct {
	/**> Supertype of the dictionary cursor. */
	ion_dict_cursor_t	super;
	/**> Holds the index of the current location in our search. */
	ion_fpos_t			current_location;
} ion_flat_file_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif
