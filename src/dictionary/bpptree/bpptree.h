#ifndef BPPTREE_H
#define BPPTREE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "./../../kv_system.h"
#include "./../dictionary.h"
#include "./../../files/ion_file.h"

/****************************
 * implementation dependent *
 ****************************/
typedef long eAdrType;          /* record address for external record */
typedef long bAdrType;          /* record address for btree node */

#define CC_EQ           0
#define CC_GT           1
#define CC_LT          -1

/* compare two keys and return:
 *    CC_LT     key1 < key2
 *    CC_GT     key1 > key2
 *    CC_EQ     key1 = key2
 */
typedef char (*bCompType)(ion_key_t key1, ion_key_t key2, ion_key_size_t size);
//typedef int (*bCompType)(const void *key1, const void *key2, unsigned int size);

/******************************
 * implementation independent *
 ******************************/

/* statistics */
int maxHeight;          /* maximum height attained */
int nNodesIns;          /* number of nodes inserted */
int nNodesDel;          /* number of nodes deleted */
int nKeysIns;           /* number of keys inserted */
int nKeysDel;           /* number of keys deleted */
int nDiskReads;         /* number of disk reads */
int nDiskWrites;        /* number of disk writes */

/* line number for last IO or memory error */
int bErrLineNo;

typedef boolean_e bool;
//typedef enum {false, true} bool;
typedef enum {
    bErrOk,
    bErrKeyNotFound,
    bErrDupKeys,
    bErrSectorSize,
    bErrFileNotOpen,
    bErrFileExists,
    bErrIO,
    bErrMemory 
} bErrType;

typedef void *bHandleType;

typedef struct {                /* info for bOpen() */
    char *iName;                /* name of index file */
    int keySize;                /* length, in bytes, of key */
    bool dupKeys;               /* true if duplicate keys allowed */
    int sectorSize;             /* size of sector on disk */
    bCompType comp;             /* pointer to compare function */
} bOpenType;

/***********************
 * function prototypes *
 ***********************/
bErrType bOpen(bOpenType info, bHandleType *handle);
    /*
     * input:
     *   info                   info for open
     * output:
     *   handle                 handle to btree, used in subsequent calls
     * returns:
     *   bErrOk                 open was successful
     *   bErrMemory             insufficient memory
     *   bErrSectorSize         sector size too small or not 0 mod 4
     *   bErrFileNotOpen        unable to open index file
     */

bErrType bClose(bHandleType handle);
    /*
     * input:
     *   handle                 handle returned by bOpen
     * returns:
     *   bErrOk                 file closed, resources deleted
     */

bErrType bInsertKey(bHandleType handle, void *key, eAdrType rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     *   key                    key to insert
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrDupKeys            duplicate keys (and info.dupKeys = false)
     * notes:
     *   If dupKeys is false, then all records inserted must have a
     *   unique key.  If dupkeys is true, then duplicate keys are
     *   allowed, but they must all have unique record addresses.
     *   In this case, record addresses are included in internal
     *   nodes to generate a "unique" key.
     */

bErrType bUpdateKey(bHandleType handle, void *key, eAdrType rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     *   key                    key to update
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrDupKeys            duplicate keys (and info.dupKeys = false)
     * notes:
     *   If dupKeys is false, then all records updated must have a
     *   unique key.  If dupkeys is true, then duplicate keys are
     *   allowed, but they must all have unique record addresses.
     *   In this case, record addresses are included in internal
     *   nodes to generate a "unique" key.
     */


bErrType bDeleteKey(bHandleType handle, void *key, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     *   key                    key to delete
     *   rec                    record address of key to delete
     * output:
     *   rec                    record address deleted
     * returns:
     *   bErrOk                 operation successful
     *   bErrKeyNotFound        key not found
     * notes:
     *   If dupKeys is false, all keys are unique, and rec is not used
     *   to determine which key to delete.  If dupKeys is true, then
     *   rec is used to determine which key to delete.
     */

bErrType bFindKey(bHandleType handle, void *key, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     *   key                    key to find
     * output:
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrKeyNotFound        key not found
     */

bErrType bFindFirstGreaterOrEqual(bHandleType handle, void *key, void *mkey, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     *   key                    key to find
     * output:
     *   mkey                   key associated with the found offset
     *   rec                    record address of least element greater than or equal to
     * returns:
     *   bErrOk                 operation successful
     */

bErrType bFindFirstKey(bHandleType handle, void *key, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     * output:
     *   key                    first key in sequential set
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrKeyNotFound        key not found
     */

bErrType bFindLastKey(bHandleType handle, void *key, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     * output:
     *   key                    last key in sequential set
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrKeyNotFound        key not found
     */

bErrType bFindNextKey(bHandleType handle, void *key, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     * output:
     *   key                    key found
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrKeyNotFound        key not found
     */

bErrType bFindPrevKey(bHandleType handle, void *key, eAdrType *rec);
    /*
     * input:
     *   handle                 handle returned by bOpen
     * output:
     *   key                    key found
     *   rec                    record address
     * returns:
     *   bErrOk                 operation successful
     *   bErrKeyNotFound        key not found
     */

#ifdef  __cplusplus
}
#endif

#endif
