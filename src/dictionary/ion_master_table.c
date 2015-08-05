/******************************************************************************/
/**
@file       ion_master_table.c
@author     Eric Huang, Graeme Douglas, Scott Fazackerley, Wade Penson
@brief      Master table handling.
*/
/******************************************************************************/

#include "ion_master_table.h"

FILE *ion_master_table_file = NULL;
ion_dictionary_id_t ion_master_table_next_id = 1;

/* Returns the next dictionary ID, then increments. */
ion_dictionary_id_t
ion_master_table_get_next_id(
   void
)
{
    int oldpos = ftell(ion_master_table_file);
    fseek(ion_master_table_file, 0, SEEK_SET);
    /* Flush master row                           This writes the next ID to be used, so +1 */
    ion_dictionary_config_info_t master_config = {ion_master_table_next_id + 1, 0, 0, 0, 0};
    fwrite(&master_config, sizeof(master_config), 1, ion_master_table_file);
    fseek(ion_master_table_file, oldpos, SEEK_SET);

    return ion_master_table_next_id++;
}

err_t
ion_init_master_table(
  void
)
{
    /* If it's already open, then we don't do anything */
    if (NULL != ion_master_table_file) { return err_ok; }

    ion_master_table_file = fopen(ION_MASTER_TABLE_FILENAME, "r+b");
    /* File may not exist */
    if (NULL == ion_master_table_file)
    { 
        ion_master_table_file = fopen(ION_MASTER_TABLE_FILENAME, "w+b");
        if (NULL == ion_master_table_file) { return err_file_open_error; }

        /* Clean fresh file was opened */
        /* Write master row */
        ion_dictionary_config_info_t master_config = {ion_master_table_next_id, 0, 0, 0, 0 };
        fwrite(&master_config, sizeof(master_config), 1, ion_master_table_file);
    }
    else
    {
        /* Here we read an existing file */
        /* Find existing ID count */
        ion_dictionary_config_info_t master_config;
        fread(&master_config, sizeof(master_config), 1, ion_master_table_file);
        ion_master_table_next_id = master_config.id;
    }

    return err_ok;
}

err_t
ion_close_master_table(
  void
)
{
    if (NULL != ion_master_table_file)
    {
        if (0 != fclose(ion_master_table_file)) { return err_file_close_error; } 
    }

    ion_master_table_file = NULL;

    return err_ok;
}

err_t
ion_delete_master_table(
    void
)
{
    if (NULL != ion_master_table_file)
    {
        if (0 != fremove(ION_MASTER_TABLE_FILENAME)) { return err_file_delete_error; }
    }

    return err_ok;
}

err_t
ion_master_table_create_dictionary(
    dictionary_handler_t    *handler,
    dictionary_t            *dictionary,
    key_type_t              key_type,
    int                     key_size,
    int                     value_size,
    int                     dictionary_size
)
{
    err_t err;
    err = dictionary_create(
        handler,
        dictionary,
        ion_master_table_get_next_id(), 
        key_type,
        key_size,
        value_size,
        dictionary_size
    );

    if (err_ok != err) { return err; }

    err = ion_add_to_master_table(dictionary, dictionary_size);

    return err;
}

err_t
ion_add_to_master_table(
    dictionary_t    *dictionary,
    int             dictionary_size
)
{
    fseek(ion_master_table_file, 0, SEEK_END);

    ion_dictionary_config_info_t config =
    {
        dictionary->instance->id,
        dictionary->instance->key_type,
        dictionary->instance->record.key_size,
        dictionary->instance->record.value_size,
        dictionary_size
    };

    fwrite(&config, sizeof(config), 1, ion_master_table_file);

    return err_ok;
}

err_t
ion_lookup_in_master_table(
    ion_dictionary_id_t             id,
    ion_dictionary_config_info_t    *config
)
{
    fseek(ion_master_table_file, id * sizeof(ion_dictionary_config_info_t), SEEK_SET);
    fread(config, sizeof(*config), 1, ion_master_table_file);

    if (0 == config->id) { return err_item_not_found; }
    return err_ok;
}

err_t
ion_delete_from_master_table(
    dictionary_t        *dictionary
)
{
    fseek(ion_master_table_file, dictionary->instance->id * sizeof(ion_dictionary_config_info_t), SEEK_SET);
    ion_dictionary_config_info_t blank = {0,0,0,0,0};
    fwrite(&blank, sizeof(blank), 1, ion_master_table_file);

    return err_ok;
}

err_t
ion_open_dictionary(
    dictionary_handler_t    *handler, /* Initialized */
    dictionary_t            *dictionary, /* Empty, to be returned */
    ion_dictionary_id_t     id
)
{
    err_t err;

    ion_dictionary_config_info_t config;
    err = ion_lookup_in_master_table(id, &config);
    if (err_ok != err) { return err_dictionary_initialization_failed; } /* Lookup for id failed */

    err = dictionary_open(handler, dictionary, &config);
    return err;
}

err_t
ion_close_dictionary(
    dictionary_t        *dictionary
)
{
    err_t err;

    err = dictionary_close(dictionary);
    return err;
}