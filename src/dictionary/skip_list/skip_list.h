/******************************************************************************/
/**
@file		skip_list.h
@author		Kris Wallperington
@brief		Implementation of a Skiplist data store
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

#if !defined(SKIP_LIST_STORE_H_)
#define SKIP_LIST_STORE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "skip_list_types.h"
/* #include <time.h> / * For random seed * / */

/**
@brief	  Initializes an in-memory skiplist.

@param	  skiplist
				Pointer to a skiplist instance to initialize
@param	  key_type
				Type of key used in this instance of a skiplist.
@param	  key_size
				Size of key in bytes.
@param	  value_size
				Size of value in bytes.
@param	  maxheight
				Maximum number of levels the skiplist will have.
@param	  pnum
				The numerator portion of the p value.
@param	  pden
				The denominator portion of the p value.
@return Status of initialization.
*/
ion_err_t
sl_initialize(
	ion_skiplist_t	*skiplist,
	ion_key_type_t	key_type,
	int				key_size,
	int				value_size,
	int				maxheight,
	int				pnum,
	int				pden
);

/**
@brief	  Destroys the skiplist in memory.

@details	Destroys the skiplist in memory and frees the underlying structures.

@param	  skiplist
				The skiplist to be destroyed
@return	 Status of destruction.
*/
ion_err_t
sl_destroy(
	ion_skiplist_t *skiplist
);

/**
@brief	  Inserts a @p key @p value pair into the skiplist.

@details	Inserts a @p key @p value pair into the skiplist. The key and value
			are copied byte-for-byte as passed by the user. Duplicate inserts
			are implicitly supported.

@param	  skiplist
				The skiplist in which to insert
@param	  key
				The key to be insert
@param	  value
				The value to be insert
@return	 Status of insertion.
*/
ion_status_t
sl_insert(
	ion_skiplist_t	*skiplist,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief	  Requests the @p value stored at the given @p key.

@details	Requests the @p value stored at the given @p key. The resultant
			value is then copied into the pointer provided by the user.

@param	  skiplist
				The skiplist in which to query
@param	  key
				The key to be found
@param	  value
				The container in which to put the resultant data
@return	 Status of query.
*/
ion_status_t
sl_get(
	ion_skiplist_t	*skiplist,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief	  Updates the value stored at @p key with the new @p value.

@details	Updates the value stored at @p key with the new @p value. The given
			value is copied byte-for-byte into the malloc'd memory already
			stored at the key. If the @p key does not exist within the skiplist,
			the key/value pair is inserted into the skiplist instead.

@param	  skiplist
				The skiplist in which to update
@param	  key
				The key to find and update
@param	  value
				The new value to be updated to
@return	 Status of updating.
*/
ion_status_t
sl_update(
	ion_skiplist_t	*skiplist,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief	  Attempts to delete all key/value pairs stored at the given @p key.

@details	Attempts to delete all key/value pairs stored at the given @p key.
			Returns "err_item_not_found" if the requested @p key is not in
			the skiplist, and "err_ok" if the deletion was successful. Any
			memory previously used for the deleted key/value pair(s) is freed.

@param	  skiplist
				The skiplist in which to delete from
@param	  key
				The key to delete
@return	 Status of deletion.
*/
ion_status_t
sl_delete(
	ion_skiplist_t	*skiplist,
	ion_key_t		key
);

/**
@brief	  Searches for a node with the given @p key. Used in conjunction with
			sl_query to perform key lookups.

@details	Searches for a node with the given @p key. Used in conjunction with
			sl_query to perform key lookups. Returns the first node containing
			the target key (if it exists), or the closest node less than the
			target key if it does not exist.
*/
ion_sl_node_t *
sl_find_node(
	ion_skiplist_t	*skiplist,
	ion_key_t		key
);

/**
@brief	  Iterates through each level of a skiplist and prints out the content
			of each node in a meaningful way. Intended for debug use only.

@param	  skiplist
				The skiplist to print
*/
void
print_skiplist(
	ion_skiplist_t *skiplist
);

/**
@brief	  Generates a psuedo-random height, bounded within [0, maxheight). The
			generator is seeded using the current epoch time when the skiplist
			is initialized.

@param	  skiplist
				The skiplist to read level generation parameters from
@return	 A height.
*/
ion_sl_level_t
sl_gen_level(
	ion_skiplist_t *skiplist
);

#if defined(__cplusplus)
}
#endif

#endif /* SKIP_LIST_STORE_H_ */
