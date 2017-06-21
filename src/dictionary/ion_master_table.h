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

#if !defined(ION_MASTER_TABLE_H_)
#define ION_MASTER_TABLE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "dictionary.h"
#include "../file/sd_stdio_c_iface.h"
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
@brief		Write a record to the master table.
@details	Automatically, this call will reposition the file position
			back to where it was once the call is complete.
@param[in]	config
				A pointer to a previously allocated config object to write from.
@param[in]	where
				An integral value representing where to write to in the file.
				This file offset is byte-aligned, not record aligned, in general.

				Two special flags can be passed in here:
					- @c ION_MASTER_TABLE_CALCULATE_POS
						Calculate the position based on the passed-in config id.
					- @c ION_MASTER_TABLE_WRITE_FROM_END
						Write the record at the end of the file.
@returns	An error code describing the result of the call.
*/
ion_err_t
ion_master_table_write(
	ion_dictionary_config_info_t	*config,
	long							where
);

/**
@brief	  Opens the master table.
@details	Can be safely called multiple times without closing.
*/
ion_err_t
ion_init_master_table(
	void
);

/**
@brief		Closes the master table.
*/
ion_err_t
ion_close_master_table(
	void
);

/**
@brief		Deletes the master table.
*/
ion_err_t
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
ion_err_t
ion_master_table_create_dictionary(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size
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
ion_err_t
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
ion_err_t
ion_find_by_use_master_table(
	ion_dictionary_config_info_t	*config,
	ion_dict_use_t					use_type,
	char							whence
);

/**
@brief		Deletes a dictionary from the master table.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
ion_delete_from_master_table(
	ion_dictionary_id_t id
);

/**
@brief		Finds the target dictionary and opens it.
@param		handler
				A pointer to the handler object to be initialized.
@param		dictionary
				A pointer to the dictionary object to open.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
ion_open_dictionary(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary,
	ion_dictionary_id_t			id
);

/**
@brief		Closes a given dictionary.
@param		dictionary
				A pointer to the dictionary object to close.
*/
ion_err_t
ion_close_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief		Deletes a given dictionary instance and deletes it from the master
			table.
@param		dictionary
				A pointer to the dictionary object to delete.
@param		id
				The identifier identifying the dictionary metadata in the
				master table. Required to delete a closed dictionary without
				reopening it.
*/
ion_err_t
ion_delete_dictionary(
	ion_dictionary_t	*dictionary,
	ion_dictionary_id_t id
);

/**
@brief		Retrieves the type of dictionary stored under a particular id in the
			master table.
@param		type
				The type of dictionary instance to initialize the handler to.
@param		handler
				A pointer to the handler to be set.
@returns	An error code describing the result of the operation.
*/
ion_err_t
ion_switch_handler(
	ion_dictionary_type_t		type,
	ion_dictionary_handler_t	*handler
);

/**
@brief		Returns the next dictionary ID, then increments.
@param		id
				An identifier pointer to be written into.
@returns	An error code describing the result of the operation.
*/
ion_err_t
ion_master_table_get_next_id(
	ion_dictionary_id_t *id
);

/**
@brief		Retrieves the type of dictionary stored under a particular id in the
			master table.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	The type of dictionary implementation corresponding to the id.
*/
ion_dictionary_type_t
ion_get_dictionary_type(
	ion_dictionary_id_t id
);

#if defined(__cplusplus)
}
#endif /* C++ Guard */

#endif /* ION_USING_MASTER_TABLE */

#endif /* ION_MASTER_TABLE_H_ */
