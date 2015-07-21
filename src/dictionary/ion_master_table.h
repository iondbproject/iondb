/******************************************************************************/
/**
@file       ion_master_table.h
@author     Eric Huang, Graeme Douglas, Scott Fazackerley, Wade Penson
@brief      Master table handling.
@details    At compile time, the master table is either used, OR it is not included
            at all. The directive, ION_USING_MASTER_TABLE controls this.
            
            If the master table is used, the assumption is that the user does not
            bypass the master table and call dictionary_create directly.

            If the master table is not used, the user must provide IDs directly to
            dictionary creation and the onus is on the user to resolve conflicts,
            if they occur.

            Any files created by implementations have file
*/
/******************************************************************************/

#ifndef ION_MASTER_TABLE_H
#define ION_MASTER_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dictionary.h"

#if ION_USING_MASTER_TABLE


#define ION_MASTER_TABLE_FILENAME "ion_master_table.tbl"

/**
@brief ID of the master table.
*/
#define ION_MASTER_TABLE_ID 0

/**
@brief Master table resposible for managing instances. 
*/
extern ion_dictionary_id_t ion_master_table_next_id;

extern FILE                *ion_master_table_file;

/**
@brief      Opens the master table.
@details    Can be safely called multiple times without closing.
*/
err_t
ion_init_master_table(
  void
);

/**
@brief Closes the master table.
*/
err_t
ion_close_master_table(
  void
);

/**
@brief Deletes the master table.
*/
err_t
ion_delete_master_table(
  void
);

/**
@brief Creates a dictionary through use of the master table.
*/
err_t
ion_master_table_create_dictionary(
    dictionary_handler_t    *handler,
    dictionary_t            *dictionary,
    key_type_t              key_type,
    int                     key_size,
    int                     value_size,
    int                     dictionary_size
);

/**
@brief Adds the given dictionary to the master table.
*/
err_t
ion_add_to_master_table(
    dictionary_t    *dictionary,
    int             dictionary_size
);

/**
@brief Looks up the config of the given id.
*/
err_t
ion_lookup_in_master_table(
    ion_dictionary_id_t             id,
    ion_dictionary_config_info_t    *config
);

/**
@brief Deletes a dictionary from the master table.
*/
err_t
ion_delete_from_master_table(
    dictionary_t        *dictionary
);

/**
@brief Finds the target dictionary and opens it.
*/
err_t
ion_open_dictionary(
    dictionary_handler_t    *handler,
    dictionary_t            *dictionary,
    ion_dictionary_id_t     id
);

/**
@brief Closes a given dictionary.
*/
err_t
ion_close_dictionary(
    dictionary_t        *dictionary
);

#ifdef __cplusplus
}
#endif /* C++ Guard */

#endif /* ION_USING_MASTER_TABLE */

#endif /* ION_MASTER_TABLE_H */