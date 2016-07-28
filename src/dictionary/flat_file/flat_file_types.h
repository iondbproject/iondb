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

/**
@brief		This type describes the status flag within a flat file row.
*/
typedef ion_byte_t flat_file_row_status_t;

/**
@brief		Metadata container that holds flat file specific information.
*/
typedef struct {
	ion_dictionary_parent_t super;	/**< Parent structure that holds dictionary level information. */
	ion_boolean_t			sorted_mode;/**< Flag to toggle whether or not to activate "sorted mode" for storage. */
	ion_fpos_t				start_of_data;	/**< This signifies where the actual record data starts, in case we want to
											write some metadata at the beginning of the flat file's file. */
	ion_dictionary_size_t	num_buffered;	/**< This comes from the given dictionary size, and signifies how many
											 records we want to buffer at a time. This is a trade-off between
											 better performance and increased memory usage. */
	FILE					*data_file;	/**< The file descriptor of the file this flat file instance operates on. */
	size_t					row_size;	/**< This value expresses the size of one row inside the @p data_file. A row is defined
										as a record + metadata. */
} ion_flatfile_t;

#if defined(__cplusplus)
}
#endif

#endif
