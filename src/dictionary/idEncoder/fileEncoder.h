/*
 * fileEncoder.h
 *
 *  Created on: Jul 22, 2015
 *      Author: workstation
 */

#ifndef FILEENCODER_H_
#define FILEENCODER_H_


#include "./../../kv_system.h"
#include "./../dicttypes.h"

#define MAX_PARENT_NIBBLES		3				/** Parent ID can be no more that 3 nibbles */
#define NUMBER_OF_BITS 			4				/** 4 bits per nibble */
#define FILENAME_SIZE			13				/** 8.3 + null ( 8 + 1 + 3 + 1 )=13 */
#define SIZE_OF_TYPE_CODE		3				/** 3 digits for type code */

/** @FIXME add optimization for  __flash for AVR  **/
static const char encode[16] =
{'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P'};

static const char parent_type[16] =
{'0','1','2','3','4','5','6','7','8','9','Z','Y','X','W','V'};

typedef enum {
	mastertable,
	flatfile,
	linear_hash,
	bplus_tree,
	file_based_open_address_hash,
} datastore_e;

typedef struct child {
	char						*child_filename;
	ion_dictionary_id_t			child_id;
} child_t;

typedef struct parent {
	char						*parent_filename;
	datastore_e					type;
} parent_t;

typedef struct filename			filename_t;

struct filename{
	ion_dictionary_id_t 		instance_id;
	void 						(* destroy)(filename_t*);
	union {
		parent_t				parent;
		child_t					child;
	};
} ;


void
destory(
	filename_t						*file
);
/**
 * @brief
 * @param file_id
 * @return
 */
err_t
encode_parent_id(
	filename_t						*file
);

err_t
encode_child_id(
	filename_t						*file
);

#endif /* FILEENCODER_H_ */
