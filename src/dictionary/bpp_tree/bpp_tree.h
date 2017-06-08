#if !defined(BPP_TREE_H_)
#define BPP_TREE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "../../key_value/kv_system.h"
#include "./../dictionary.h"
#include "./../../file/ion_file.h"

/****************************
 * implementation dependent *
 ****************************/
typedef long	ion_bpp_external_address_t;		/* record address for external record */
typedef long	ion_bpp_address_t;		/* record address for btree node */

#define ION_CC_LT	-1
#define ION_CC_EQ	0
#define ION_CC_GT	1

/* compare two keys and return:
 *	CC_LT	 key1 < key2
 *	CC_GT	 key1 > key2
 *	CC_EQ	 key1 = key2
*/
typedef char (*ion_bpp_comparison_t)(
	ion_key_t		key1,
	ion_key_t		key2,
	ion_key_size_t	size
);

/* typedef int (*ion_bpp_comparison_t)(const void *key1, const void *key2, unsigned int size); */

/******************************
 * implementation independent *
 ******************************/

/* statistics */
int maxHeight;	/* maximum height attained */
int nNodesIns;	/* number of nodes inserted */
int nNodesDel;	/* number of nodes deleted */
int nKeysIns;	/* number of keys inserted */
int nKeysDel;	/* number of keys deleted */
int nDiskReads;	/* number of disk reads */
int nDiskWrites;/* number of disk writes */

/* line number for last IO or memory error */
int bErrLineNo;

typedef ion_boolean_e ion_bpp_bool_t;

/* typedef enum {false, true} bool; */
typedef enum ION_BPP_ERR {
	bErrOk, bErrKeyNotFound, bErrDupKeys, bErrSectorSize, bErrFileNotOpen, bErrFileExists, bErrIO, bErrMemory
} ion_bpp_err_t;

typedef void *ion_bpp_handle_t;

typedef struct {
	/* info for bOpen() */
	char					*iName;	/* name of index file */
	int						keySize;/* length, in bytes, of key */
	ion_bpp_bool_t			dupKeys;		/* true if duplicate keys allowed */
	size_t					sectorSize;	/* size of sector on disk */
	ion_bpp_comparison_t	comp;			/* pointer to compare function */
} ion_bpp_open_t;

/***********************
 * function prototypes *
 ***********************/
ion_bpp_err_t
b_open(
	ion_bpp_open_t		info,
	ion_bpp_handle_t	*handle
);

/*
 * input:
 *   info				   info for open
 * output:
 *   handle				 handle to btree, used in subsequent calls
 * returns:
 *   bErrOk				 open was successful
 *   bErrMemory			 insufficient memory
 *   bErrSectorSize		 sector size too small or not 0 mod 4
 *   bErrFileNotOpen		unable to open index file
*/

ion_bpp_err_t
b_close(
	ion_bpp_handle_t handle
);

/*
 * input:
 *   handle				 handle returned by bOpen
 * returns:
 *   bErrOk				 file closed, resources deleted
*/

ion_bpp_err_t
b_insert(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 *   key					key to insert
 *   rec					record address
 * returns:
 *   bErrOk				 operation successful
 *   bErrDupKeys			duplicate keys (and info.dupKeys = false)
 * notes:
 *   If dupKeys is false, then all records inserted must have a
 *   unique key.  If dupkeys is true, then duplicate keys are
 *   allowed, but they must all have unique record addresses.
 *   In this case, record addresses are included in internal
 *   nodes to generate a "unique" key.
*/

ion_bpp_err_t
b_update(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 *   key					key to update
 *   rec					record address
 * returns:
 *   bErrOk				 operation successful
 *   bErrDupKeys			duplicate keys (and info.dupKeys = false)
 * notes:
 *   If dupKeys is false, then all records updated must have a
 *   unique key.  If dupkeys is true, then duplicate keys are
 *   allowed, but they must all have unique record addresses.
 *   In this case, record addresses are included in internal
 *   nodes to generate a "unique" key.
*/

ion_bpp_err_t
b_delete(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	*rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 *   key					key to delete
 *   rec					record address of key to delete
 * output:
 *   rec					record address deleted
 * returns:
 *   bErrOk				 operation successful
 *   bErrKeyNotFound		key not found
 * notes:
 *   If dupKeys is false, all keys are unique, and rec is not used
 *   to determine which key to delete.  If dupKeys is true, then
 *   rec is used to determine which key to delete.
*/

ion_bpp_err_t
b_get(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	*rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 *   key					key to find
 * output:
 *   rec					record address
 * returns:
 *   bErrOk				 operation successful
 *   bErrKeyNotFound		key not found
*/

ion_bpp_err_t
b_find_first_greater_or_equal(
	ion_bpp_handle_t			handle,
	void						*key,
	void						*mkey,
	ion_bpp_external_address_t	*rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 *   key					key to find
 * output:
 *   mkey				   key associated with the found offset
 *   rec					record address of least element greater than or equal to
 * returns:
 *   bErrOk				 operation successful
*/

ion_bpp_err_t
b_find_first_key(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	*rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 * output:
 *   key					first key in sequential set
 *   rec					record address
 * returns:
 *   bErrOk				 operation successful
 *   bErrKeyNotFound		key not found
*/

ion_bpp_err_t
b_find_last_key(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	*rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 * output:
 *   key					last key in sequential set
 *   rec					record address
 * returns:
 *   bErrOk				 operation successful
 *   bErrKeyNotFound		key not found
*/

ion_bpp_err_t
b_find_next_key(
	ion_bpp_handle_t			handle,
	void						*key,
	ion_bpp_external_address_t	*rec
);

/*
 * input:
 *   handle				 handle returned by bOpen
 * output:
 *   key					key found
 *   rec					record address
 * returns:
 *   bErrOk				 operation successful
 *   bErrKeyNotFound		key not found
*/

#if defined(__cplusplus)
}
#endif

#endif
