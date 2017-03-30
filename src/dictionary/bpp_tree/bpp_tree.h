#if !defined(BPP_TREE_H_)
#define BPP_TREE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
/*
#include <stdarg.h>
#include "../../key_value/kv_system.h"
#include "./../dictionary.h"
#include "./../../file/ion_file.h"
*/

/*
 *	Header for B+ tree file
 *	On most systems (short=16bit, long=64bit), sized at 184 bits (23 bytes)
 */
typedef struct {
	unsigned char			revision;	/* Versioning information; incase format ever changes, we can identify, and correct files */
	unsigned short			keySize;	/* length, in bytes, of key */
	unsigned short			valueSize;	/* ^^ values */
	unsigned short			leafCount;	/* number of nodes per leaf; links this+1 */
	unsigned long			recordCount;/* number of records in tree*/
	unsigned long			depth;		/* number of levels in the tree */
	bool					allow_duplicates;
} ion_bpp_header;

/* Since the header is suspected to be 23 bytes, and may change in the future, offset the first data record by 32 bytes; 256 bits */
#define BPP_FIRST_LEAF_OFFSET 32 
typedef struct {
	void*					keys;		/* array of key values; sizeof(ion_bpp_header.keySize)*ion_bpp_header.leafCount; */
} ion_bpp_leaf;
typedef ion_bpp_leaf ion_bpp_end_leaf;	/* array of dictionary values; sizeof(ion_bpp_header.valueSize)*ion_bpp_header.leafCount; */

typedef struct {
	char					*fileName;	/* Filename associated with this header */
	ion_bpp_header			*head;		/* Header of file statistics and information */
	long					*indices;	/* offsets for where each row of the tree starts in disk */
	FILE					*file;		/* File on Disk */
} ion_bpp;

typedef struct {
	bool				found;			/* Whether the key was at teh destination or not */
	bool				has_end_offset;	/* Whether there where multiple matching keys; otherwise, end_offset variable is set to NULL */
	unsigned long		*row_offset;	/* How many records in was it found (or, the leaf searched, and failed leaf loc) */
	unsigned long		*end_offset;	/* Likely null */
} ion_bpp_search_record;

/*
static long* getRowIndices(ion_bpp_header *head);
static void writeHeader(FILE *file, ion_bpp_header *new_head);
static ion_bpp* openBpp(char *filename, ion_bpp_header *new_file);
static void* searchRecords(ion_bpp *bpp, void* searchKey);
static ion_bpp_search_record* searchRecordOffset(ion_bpp *bpp, void* searchKey);
static void* readRecords(ion_bpp *bpp, ion_bpp_search_record *rec, bool return_all);
static unsigned char insertRecords(ion_bpp *bpp, void* key, void* value);
static unsigned char updateRecords(ion_bpp *bpp, void* key, void* value);
static unsigned char deleteRecords(ion_bpp *bpp, void* key);
static bool mergeRight(ion_bpp *bpp, int depth, long leaf_offset);
static bool mergeUp(ion_bpp *bpp, int depth, long leaf_offset);
static void move(ion_bpp *bpp, unsigned int depth, long source_offset, long dest_offset, bool clear, bool whole_leaf);
static void* calculateNullNode(unsigned short keySize, unsigned char revision);

static unsigned short getLeafCount(ion_bpp *bpp, unsigned short elementSize, long absolute_offset);

static void debugPrintHeader(ion_bpp_header *head);
static void debugPrintTree(ion_bpp *tree);
*/ //DOESNT WORK WITH ION *shrug*

#if defined(__cplusplus)
}
#endif

#endif
