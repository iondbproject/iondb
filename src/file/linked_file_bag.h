/******************************************************************************/
/**
@file		linked_file_bag.h
@author		Graeme Douglas
@brief		API for a persistent bag. Items are linked together in a singly
			linked list.
@details	The bag is constituted of several sub bags, described by the singly
			linked lists. All operations act on these sub bags.
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

#if !defined(ION_LINKED_FILE_BAG_H_)
#define ION_LINKED_FILE_BAG_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../key_value/kv_system.h"
#include "ion_file.h"

#define ION_LFB_NULL ION_FILE_NULL

/**
@brief		A handler struct for a linked file bag instance.
*/
typedef struct linkedfilebag {
	/**> The file handle for the file where the data is stored. */
	ion_file_handle_t	file_handle;
	/**> The offset for the next empty slot to write to. */
	ion_file_offset_t	next_empty;
} ion_lfb_t;

/**
@brief		Add an item to the linked file bag.
@param		bag
				A pointer to the linked file bag handler object which
				we wish to add this item to.
@param		to_write
				A pointer to the buffer of data to write.
@param		num_bytes
				The number of bytes to write from the start of @p to_write.
@param		next
				The offset of next item in this bag, if one exists (otherwise,
				pass in @c -1).
@param		wrote_at
				A pointer to an already allocated file offset used to
				write where the linked file bag actually wrote. This is useful
				if the calling function needs to use this information.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_put(
	ion_lfb_t			*bag,
	ion_byte_t			*to_write,
	unsigned int		num_bytes,
	ion_file_offset_t	next,
	ion_file_offset_t	*wrote_at
);

/**
@brief
*/
/**
@brief		Add an item to the linked file bag.
@param		bag
				A pointer to the linked file bag handler object which
				we wish to add this item to.
@param		offset
				Where to read the information from within the file bag.
@param		num_bytes
				The number of bytes to read into @p write_to.
@param		write_to
				A pointer for a memory buffer to write the retrieved data
				into.
@param		next
				A pointer to a file offset (already allocated) which is written
				to describing where the next item in this bag is located,
				for traversal purposes. This read from the file does NOT
				count towards the @p num_bytes parameter specified.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_get(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*write_to,
	ion_file_offset_t	*next
);

/**
@brief		Attempt to delete a record stored at a given offset.
@param		bag
				A pointer to the initialized linked file bag handler
				for which we wish to delete the record from.
@param		offset
				The offset of the record to remove from its bag.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_delete(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset
);

/**
@brief		Attempt to delete all contents from the bag starting at
			a given offset.
@details	This will not delete everything stored in the object
			with handle @p bag, but instead delete everything linked
			starting with the record at @p offset.
@param		bag
				A pointer to the initialized linked file bag handler for which
				we wish to delete from.
@param		offset
				The offset of the first linked record to delete from.
@param		count
				A pointer to write count data to. If it is @c NULL,
				then no data will be written.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_delete_all(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	ion_result_count_t	*count
);

/**
@brief		Attempt to update a record within a linked file bag
			at a given offset.
@details	This will update a record in place. If @p num_bytes
			does not match the size of the record already stored
			(especially if @p num_bytes is larger than the size
			of the record already stored) then bad things may ensue.
@param		bag
				A pointer to the linked file bag handler for which we
				wish to update a record.
@param		offset
				The offset where the record data starts.
@param		num_bytes
				The number of bytes to write to the record.
@param		to_write
				The data to actually write.
@param		next
				A pointer to a file offset (already allocated), which
				will have the file offset of the next record in this
				bag written into it. This can be useful for traversal
				purposes.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_update(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*to_write,
	ion_file_offset_t	*next
);

/**
@brief		Attempt to update all records kept within a specific bag,
			starting at some record at a given offset.
@details	All records linked should be the same size (@p num_bytes) or else
			wastage or corruption may occur.
@param		bag
				A pointer to the linked file bag handler for which we
				wish to update a record.
@param		offset
				The offset of the first record to update.
@param		num_bytes
				The number of bytes to write to each record.
@param		to_write
				The data to actually write to each record.
@param		count
				A pointer to write count data to. If it is @c NULL,
				then no data will be written.
@returns	An error code describing the result of the call.
*/
ion_err_t
lfb_update_all(
	ion_lfb_t			*bag,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*to_write,
	ion_result_count_t	*count
);

#if defined(__cplusplus)
}
#endif

#endif
