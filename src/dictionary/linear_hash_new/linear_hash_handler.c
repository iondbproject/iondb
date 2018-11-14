/******************************************************************************/
/**
@file		linear_hash_handler.c
@author		Andrew Feltham, Spencer MacBeth
@brief		The handler for a linear hash .
@copyright	Copyright 2018
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

#include "linear_hash_handler.h"
#include "../dictionary.h"

void
ion_linear_hash_dict_init(
        ion_dictionary_handler_t *handler
) {
    handler->insert = ion_linear_hash_dict_insert;
    handler->get = ion_linear_hash_dict_get;
    handler->create_dictionary = ion_linear_hash_create_dictionary;
    handler->remove = ion_linear_hash_dict_delete;
    handler->delete_dictionary = ion_linear_hash_delete_dictionary;
    handler->destroy_dictionary = ion_linear_hash_destroy_dictionary;
    handler->update = ion_linear_hash_dict_update;
    /* handler->find				= linear_hash_dict_find; */
    handler->close_dictionary = ion_linear_hash_close_dictionary;
    handler->open_dictionary = ion_linear_hash_open_dictionary;
}

ion_status_t
ion_linear_hash_dict_insert(
        ion_dictionary_t *dictionary,
        ion_key_t key,
        ion_value_t value
) {
    return ion_linear_hash_insert(key, value, (ion_linear_hash_table_t *) dictionary->instance);
}

ion_status_t
ion_linear_hash_dict_get(
        ion_dictionary_t *dictionary,
        ion_key_t key,
        ion_value_t value
) {
    return ion_linear_hash_get(key, value, (ion_linear_hash_table_t *) dictionary->instance);
}

ion_status_t
ion_linear_hash_dict_update(
        ion_dictionary_t *dictionary,
        ion_key_t key,
        ion_value_t value
) {
    return ion_linear_hash_update(key, value, (ion_linear_hash_table_t *) dictionary->instance);
}

ion_err_t
ion_linear_hash_create_dictionary(
        ion_dictionary_id_t id,
        ion_key_type_t key_type,
        ion_key_size_t key_size,
        ion_value_size_t value_size,
        ion_dictionary_size_t dictionary_size,
        ion_dictionary_compare_t compare,
        ion_dictionary_handler_t *handler,
        ion_dictionary_t *dictionary
) {
    int initial_size, split_threshold;

    dictionary->instance = malloc(sizeof(ion_linear_hash_table_t));

    if (NULL == dictionary->instance) {
        return err_out_of_memory;
    }

    dictionary->instance->compare = compare;

    initial_size = 4;
    split_threshold = 85;

    /* TODO Should we handle the possible error code returned by this? If yes, what sorts of errors does it return? */
    ion_err_t result = ion_linear_hash_init(
            id,
            key_type,
            key_size,
            value_size,
            initial_size,
            split_threshold,
            (ion_linear_hash_table_t *) dictionary->instance
    );

    if ((err_ok == result) && (NULL != handler)) {
        dictionary->handler = handler;
        dictionary->instance->type = dictionary_type_linear_hash_t;
    }

    return result;
}

ion_status_t
ion_linear_hash_dict_delete(
        ion_dictionary_t *dictionary,
        ion_key_t key
) {
    return ion_linear_hash_delete(key, (ion_linear_hash_table_t *) dictionary->instance);
}

ion_err_t
ion_linear_hash_destroy_dictionary(
        ion_dictionary_id_t id
) {
    char filename[ION_MAX_FILENAME_LENGTH];

    dictionary_get_filename(id, "lhs", filename);

    if (0 != fremove(filename)) {
        return err_file_delete_error;
    }

    dictionary_get_filename(id, "lhd", filename);

    if (0 != fremove(filename)) {
        return err_file_delete_error;
    }

    return err_ok;
}

ion_err_t
ion_linear_hash_delete_dictionary(
        ion_dictionary_t *dictionary
) {
    ion_dictionary_id_t id = dictionary->instance->id;
    ion_err_t err;
    err = ion_linear_hash_close_dictionary(dictionary);
    if (err_ok != err) {
        return err;
    }
    err = ion_linear_hash_destroy_dictionary(id);
    if (err_ok != err) {
        return err;
    }
    free(dictionary->instance);
    dictionary->instance = NULL;
    return err;
}

ion_err_t
ion_linear_hash_open_dictionary(
        ion_dictionary_handler_t *handler,
        ion_dictionary_t *dictionary,
        ion_dictionary_config_info_t *config,
        ion_dictionary_compare_t compare
) {
    return ion_linear_hash_create_dictionary(config->id, config->type, config->key_size, config->value_size,
                                             config->dictionary_size, compare, handler, dictionary);
}

ion_err_t
ion_linear_hash_close_dictionary(
        ion_dictionary_t *dictionary
) {
    ion_err_t err = err_ok;
    err = ion_linear_hash_close((ion_linear_hash_table_t *) dictionary->instance);
    if (dictionary->instance != NULL) {
        free(dictionary->instance);
        dictionary->instance = NULL;
    }
    return err;
}

ion_status_t
ion_linear_hash_dict_find(
        ion_dictionary_t *dictionary
) {
    ion_status_t status = ION_STATUS_INITIALIZE;

    UNUSED(dictionary);
    status.error = err_not_implemented;
    return status;
}
