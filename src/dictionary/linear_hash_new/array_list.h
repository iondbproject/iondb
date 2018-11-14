/******************************************************************************/
/**
@file		array_list.h
@author		Andrew Feltham, Spencer MacBeth
@brief		Array List Implementation.
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
#ifndef IONDB_ARRAY_LIST_H
#define IONDB_ARRAY_LIST_H

#include "../../key_value/kv_system.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define ARRAY_LIST_END_OF_LIST            -1

/* SIMPLE ARRAY_LIST FOR BUCKET MAP */
typedef struct {
    int current_size;
    int *data;
} ion_array_list_t;

/**
@brief		Initialize an array list
@param[in]	init_size
				The number of indexes to initialize the array list with
@param[in]	array_list
				Pointer to the array list location in memory
@return		err_ok if successful, err_out_of_memory if array list cannot be created with init_size.
*/
ion_err_t
ion_array_list_init(
        int init_size,
        ion_array_list_t *array_list
);

/**
@brief		Insert a value into an array list.
@details	If the index specified for insertion is larger than the size of the table, the array list size is doubled.
@param[in]	index
				The index in the array list to insert the value at.
@param[in]	value
				The value to insert.
@param[in]	array_list
				The array list to insert the value into.
@return		err_ok if successful, err_out_of_memory if array list cannot be created doubled in size.
*/
ion_err_t
ion_array_list_insert(
        int index,
        int value,
        ion_array_list_t *array_list
);

/**
@brief		Retrieves a value from an array list.
@param[in]	index
				The index in the array list to retrieve the value from.
@param[in]	array_list
				The array list to retrieve the value from.
@return		An int of the value at that position if successful, array_list_end_of_list if array bucket idx is outside
            of array list bounds.
*/
int
ion_array_list_get(
        int index,
        ion_array_list_t *array_list
);

/**
 * @brief Frees all memory for an array list.
 * @param[in] array_list
 *                  The array list to free.
 */
void
ion_array_list_destroy(ion_array_list_t *array_list);

/**
 * @brief Writes an array list to the current seek position in a file.
 * @param[in] file
 *              The file to write to
 * @param[in] array_list
 *              The array list to write.
 * @return err_ok if successful, err_file_write_error if a write failure occurred.
 */
ion_err_t
ion_array_list_save_to_file(FILE *file, ion_array_list_t *array_list);

/**
 * @brief Initializes a
 * @param[in] file
 *              The file to read from at the current seek position
 * @param[in] array_list
 *              The array list to use. This should be created already.
 * @return err_ok if successful, err_file_read_error if the file could not be read.
 */
ion_err_t
ion_array_list_init_from_file(FILE *file, ion_array_list_t *array_list);

#if defined(__cplusplus)
}
#endif

#endif //IONDB_ARRAY_LIST_H
