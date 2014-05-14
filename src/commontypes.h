/******************************************************************************/
/**
@file		commontypes.h
@brief		Definitions of types used throughout library.
@author		Graeme Douglas
*/
/******************************************************************************/

#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/**
@brief		General error type.
*/
enum error 
{
	ERROR_NONE,
	ERROR_UNSUPPORTED,
	ERROR_UNDEFINEDCHUNKTYPE,
	ERROR_CANNOTALLOCATEMEMORY,
	ERROR_OUTSIDEOFCHUNK,
	ERROR_COUNT
};
typedef enum error		error_t;

/**
@brief		General result throughout the library.
@details	Contains success and failure information.
@todo		Create macro to check for OK status.
*/
struct status
{
	error_t		error;
	unsigned int	affected;
};
typedef struct status		status_t;

/**
@brief		Check that a status is OK.
*/
#define IS_STATUS_OK(s) (ERROR_NONE == (s).error)

/**
@brief		An offset.
@details	May be "logical", "physical", otherwise.
*/
typedef unsigned int		offset_t;

/**
@brief		A single byte.
*/
typedef unsigned char		byte;

/**
@brief		A boolean.
*/
typedef byte			bool_t;

/**
@brief		A generic size type.
@todo		Better name.
*/
typedef unsigned int		my_size_t;

/**
@brief		A general data type that can store whatever you want.
@todo		Write a method/macro to determine the equality of a
		byte_segment.
*/
struct byte_segment
{
	my_size_t		length;		/**< The number of bytes in the
						     byte array. */
	byte			*segment;	/**< A byte array. Where the
						     data is stored. */
};

/**
@brief		A general type for keys.
*/
typedef struct byte*		my_key_t;

/**
@brief		A general type for values.
*/
typedef struct byte*		value_t;

/**
@brief		True value for booleans.
*/
#ifndef true
#define true 1
#endif

/**
@brief		False value for booleans.
*/
#ifndef false
#define false 0
#endif

#ifdef __cplusplus
}
#endif

#endif
