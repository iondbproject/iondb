#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EMPTY 			-1
#define DELETED 		-2
#define IN_USE 			-3
#define END_RECORD		-4
#define SIZEOF(STATUS) 	1
#define UNINITIALIZED 	-1
#define INITIALIZED		1

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct f_file_record
{
	char 			status;			/**< the status of the record */
	unsigned char 	data[];			/**< the data in the record */
} f_file_record_t;

#ifdef __cplusplus
}
#endif

#endif /* FILEHANDLER_H_ */
