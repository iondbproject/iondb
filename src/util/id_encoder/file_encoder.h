/******************************************************************************/
/**
@file		file_encoder.h
@author		Scott Fazackerley
@brief		Encoding for numeric ID to file names.
*/
/******************************************************************************/

#ifndef FILE_ENCODER_H_
#define FILE_ENCODER_H_


#include "./../../kv_system.h"
#include "../../dictionary/dictionary_types.h"

#define MAX_PARENT_NIBBLES		3				/** Parent ID can be no more that 3 nibbles */
#define NUMBER_OF_BITS 			4				/** 4 bits per nibble */
#define FILENAME_SIZE			13				/** 8.3 + null ( 8 + 1 + 3 + 1 )=13 */
#define SIZE_OF_TYPE_CODE		3				/** 3 digits for type code */

/** @FIXME add optimization for  __flash for AVR  **/
/**
 * @brief 		Array for encoding for parent IDs
 */
static const char encode[16] =
{'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P'};

/**
 * @brief 		Array for encoding for child IDs
 */
static const char parent_type[16] =
{'0','1','2','3','4','5','6','7','8','9','Z','Y','X','W','V'};


/**
 * @brief		Descriptors for the available types of tables
 */
typedef enum {
	mastertable,                 			/**< mastertable */
	flatfile,                    			/**< flatfile */
	linear_hash,                 			/**< linear_hash */
	bplus_tree,                  			/**< bplus_tree	*/
	file_based_open_address_hash,			/**< file_based_open_address_hash */
} fe_datastore_e;

/**
 * @brief 		Struct for managing child file names associated with a parent
 * 				structure
 */
typedef struct child {
	char					*child_filename;/**< filename to be created */
	ion_dictionary_id_t		child_id;		/**< Caller provided ID of the child file */
} fe_child_t;


/**
 * @brief		Structure for managing parent file names.
 */
typedef struct parent {
	char					*parent_filename;
											/**< filename to be created */
	fe_datastore_e			type;			/**< Caller provided type of parent file to create */
} fe_parent_t;

/**
 * @brief		Structure for managing config file names for a parent file.
 */
typedef struct config {
	char					*config_filename;
} fe_config_t;

/**
 * @brief		Stuct for filename
 */
typedef struct filename		fe_filename_t;

struct filename{
	ion_dictionary_id_t 	instance_id;	/**< Caller provided instance ID */
	void 					(* destroy)(fe_filename_t*);
											/**< Bound destory function */
	union {
		fe_parent_t			parent;			/**< Parent file specifics 	*/
		fe_child_t			child;			/**< Child file specifics	*/
		fe_config_t			config;			/**< Config file specifics	*/
	};
};


/**
 * @brief		Cleans up memory, if need be.
 * @details		Cleans up memory, if need be.  This function will be
 * 				bound to the file during encoding and will prevent callers
 * 				from unintentionally freeing memory that has been previously
 * 				feed
 *
 * @param 		file
 * 					The filename to destory
 */
void
fe_destroy(
	fe_filename_t						*file
);


/**
 * @brief		Generates an encoded filename for a parent file (main) based on
 * 				the type of data structure and ID.
 *
 * @details		Generates an encoded filename for a parent file (main) based on
 * 				the type of data structure and ID which are caller provided in
 * 				@p file.   The maximum ID value for the parent file is 4095.
 * 				Filenames are encoded as:
 * 					00000XXX.YYY
 * 							 ^^^-----encoded file type
 * 						 ^^^---------encoded parent id
 *				It function also binds the destroy function to the @p file structure
 *				allowing the caller to cleaning destroy the item.  The function
 *				will also manage memory allocation such that the caller will not
 *				have to preallocate memory.
 *
 * @param 		file
 * 					The filename that will be encoded.  Memory will be allocated by
 * 					function.
 *
 * @return		Success or failure of filename creation.
 */
err_t
fe_encode_parent_id(
	fe_filename_t						*file
);

/**
 * @brief		Generates an encoded filename for a child file based on the ID
 * 				of the parent file.
 *
 * @details		Generates an encoded filename for a child file based on the ID
 * 				of the parent file. The caller provides child ID in @p file as
 * 				well as the parent ID.  The maximum ID value for the
 * 				child file is sizeof(unsigned int).  Filenames are encoded as:
 * 					XXXXXXXX.YYY
 * 							 ^^^-----encoded parent ID
 * 					^^^^^^^^---------encoded child ID
 *				It function also binds the destroy function to the @p file structure
 *				allowing the caller to cleaning destroy the item.The function
 *				will also manage memory allocation such that the caller will not
 *				have to preallocate memory.
 *
 * @param 		file
 * 					The filename that will be encoded.  Memory will be allocated by function.
 *
 * @return		Success or failure of filename creation.
 */
err_t
fe_encode_child_id(
	fe_filename_t						*file
);

/**
 * @brief		Generates an encoded filename for a config file based on the ID
 * 				of the parent file.
 *
 * @details		Generates an encoded filename for a config file based on the
 * 				caller provided ID of the parent file in @p file.  Filenames
 * 				are encoded as:
 * 					00000000.YYY
 * 							 ^^^-----encoded parent ID
 * 					^^^^^^^^---------Always zeros
 *				It function also binds the destroy function to the @p file structure
 *				allowing the caller to cleaning destroy the item.The function
 *				will also manage memory allocation such that the caller will not
 *				have to preallocate memory.
 *
 * @param 		file
 * 					The filename that will be encoded.  Memory will be allocated by function.
 *
 * @return		Success or failure of filename creation.
 */
err_t
fe_encode_config_id(
	fe_filename_t						*file
);

#endif /* FILE_ENCODER_H_ */
