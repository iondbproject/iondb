/******************************************************************************/
/**
@file		ion_master_table.h
@author		Eric Huang, Graeme Douglas, Scott Fazackerley, Wade Penson
@brief		Master table API.
@details	At compile time, the master table is either used, OR it is not
			included at all. The directive, ION_USING_MASTER_TABLE controls
			this.

			If the master table is used, the assumption is that the user does
			not bypass the master table and call dictionary_create directly.

			If the master table is not used, the user must provide IDs directly
			to dictionary creation and the onus is on the user to resolve
			conflicts, if they occur.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see @ref AUTHORS.md)
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

#if !defined(ION_MASTER_TABLE_H_)
#define ION_MASTER_TABLE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "dictionary.h"
#include "../file/SD_stdio_c_iface.h"
#include "../file/kv_stdio_intercept.h"

#if ION_USING_MASTER_TABLE

/**
@brief		File name for IonDB master table.
@details	Needs to be relatively short for devices.
*/
#define ION_MASTER_TABLE_FILENAME	"ion_mt.tbl"

/**
@brief		ID of the master table.
*/
#define ION_MASTER_TABLE_ID			0

/**
@brief		Flag used when searching master table; search for first instance
			matching criteria.
*/
#define ION_MASTER_TABLE_FIND_FIRST 1

/**
@brief		Flag used when searching master table; search for last instance
			matching criteria.
*/
#define ION_MASTER_TABLE_FIND_LAST	-1

/**
@brief		Master table resposible for managing instances.
*/
extern ion_dictionary_id_t ion_master_table_next_id;

/**
@brief		Master table file.
*/
extern FILE *ion_master_table_file;

/**
@brief	  Opens the master table.
@details	Can be safely called multiple times without closing.
*/
err_t
ion_init_master_table(
	void
);

/**
@brief		Closes the master table.
*/
err_t
ion_close_master_table(
	void
);

/**
@brief		Deletes the master table.
*/
err_t
ion_delete_master_table(
	void
);

/**
@brief		Creates a dictionary through use of the master table.
@param		handler
				A pointer to an allocated and initialized dictionary handler
				object that contains all implementation specific data
				and function pointers.
@param		dictionary
				A pointer to an allocated dictionary object, which will be
				written into when opened.
@param		key_type
				The type of key to be used with this dictionary, which
				determines the key comparison operator.
@param		key_size
				The size of the key type to be used with this dictionary.
@param		value_size
				The size of the value type to be used with this dictionary.
@param		dictionary_size
				The dictionary implementation specific dictionary size
				parameter.
@returns	An error code describing the result of the operation.
*/
err_t
ion_master_table_create_dictionary(
	dictionary_handler_t	*handler,
	dictionary_t			*dictionary,
	key_type_t				key_type,
	int						key_size,
	int						value_size,
	int						dictionary_size
);

/**
@brief		Adds the given dictionary to the master table.
@param		dictionary
				A pointer to the dictionary object to add to the master table.
@param		dictionary_size
				The implementation specific size parameter used when
				creating the dictionary.
*/
err_t
ion_add_to_master_table(
	dictionary_t	*dictionary,
	int				dictionary_size
);

/**
@brief		Looks up the config of the given id.
@param		id
				The identifier identifying the dictionary metadata in the
				master table which is to be looked up.
@param		config
				A pointer to an already allocated configuration object
				that will be read into from the master table.
*/
err_t
ion_lookup_in_master_table(
	ion_dictionary_id_t				id,
	ion_dictionary_config_info_t	*config
);

/**
@brief		Find first or last dictionary in master table with a given use.
@param		config
				A pointer to an already allocated configuration object
				that will be used to open the found dictionary.
@param		use_type
				The integral usage type for the dictionary. This is
				user defined.
@param		whence
				Whether to find the first or the last dictionary
				having @p use_type. This can take exactly two values,
				@ref ION_MASTER_TABLE_FIND_FIRST or
				@ref ION_MASTER_TABLE_FIND_LAST.
@returns	@c err_ok if a dictionary having usage type @p use_type
			is found, otherwise an error code properly describing
			the situation and outcome.
*/
err_t
ion_find_by_use_master_table(
	ion_dictionary_config_info_t	*config,
	ion_dict_use_t					use_type,
	char							whence
);

/**
@brief		Deletes a dictionary from the master table.
@param		dictionary
				A pointer to the dictionary object to delete (it contains its
				own identifier info for the master table).
@returns	An error code describing the result of the operation.
*/
err_t
ion_delete_from_master_table(
	dictionary_t *dictionary
);

/**
@brief		Finds the target dictionary and opens it.
@param		handler
				A pointer to an initialized dictionary handler object
				containing the implementation specific data and function
				pointers for the dictionary to open.
@param		dictionary
				A pointer to the dictionary object to open.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	An error code describing the result of the operation.
*/
err_t
ion_open_dictionary(
	dictionary_handler_t	*handler,
	dictionary_t			*dictionary,
	ion_dictionary_id_t		id
);

/**
@brief		Closes a given dictionary.
@param		dictionary
				A pointer to the dictionary object to close.
*/
err_t
ion_close_dictionary(
	dictionary_t *dictionary
);

#if defined(__cplusplus)
}
#endif /* C++ Guard */

#endif /* ION_USING_MASTER_TABLE */

#endif /* ION_MASTER_TABLE_H_ */
