/*
 * file_handler.h
 *
 *  Created on: Aug 20, 2014
 *	  Author: workstation
 */

#if !defined(FILE_HANDLER_H_)
#define FILE_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define EMPTY			-1
#define DELETED			-2
#define IN_USE			-3
#define SIZEOF(STATUS) 1
#define UNINITIALISED	-1

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct f_file_record {
	ion_record_status_t status;	/**< the status of the record */

	ion_byte_t			data[];		/**< the data in the record */
} ion_f_file_record_t;

#if defined(__cplusplus)
}
#endif

#endif /* FILE_HANDLER_H_ */
