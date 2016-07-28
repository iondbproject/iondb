/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation specific declarations for the flat file store.
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

#if !defined(FLAT_FILE_H)
#define FLAT_FILE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "flat_file_types.h"

/**
@brief		Initializes the flat file implementation and creates all necessary files.
@details	A check is done to see if this is actually an attempt to open a previously existing
			flat file instance. This (should) only happen when this is called from an open context
			instead of an initialize.
@param[in]	flatfile
				Given instance of a flat file struct to initialize. This must be allocated **heap** memory,
				as destruction will assume that it needs to be freed.
@param[in]	id
				The assigned ID of this dictionary.
@param[in]	key_type
				Key category to use for this instance.
@param[in]	key_size
				Key size, in bytes used for this instance.
@param[in]	value_size
				Value size, in bytes used for this instance.
@param[in]	dictionary_size
				Dictionary size is interpreted as how many records (key value pairs) are buffered. This should be given
				as somewhere between 1 (minimum) and the page size of the device you are working on.
@return		The status of initialization.
 */
ion_err_t
flat_file_initialize(
	ion_flatfile_t			*flatfile,
	ion_dictionary_id_t		id,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
);

#if defined(__cplusplus)
}
#endif

#endif
