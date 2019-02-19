//
// Created by itsal on 2019-02-13.
//

#ifndef IONDB_LINEAR_HASH_MACROS_H
#define IONDB_LINEAR_HASH_MACROS_H

/**
 * Defines constants and debugging macros
 */
// Granular debugging flags
#define LINEAR_HASH_DEBUG 0
#define LINEAR_HASH_DEBUG_INSERT 0
#define LINEAR_HASH_DEBUG_DELETE 0
#define LINEAR_HASH_DEBUG_STATE 0
#define LINEAR_HASH_DEBUG_INIT 1
#define LINEAR_HASH_DEBUG_CLOSE 0
#define LINEAR_HASH_DEBUG_SAVE 0
#define LINEAR_HASH_DEBUG_INCREMENT 0
#define LINEAR_HASH_DEBUG_SPLIT 0
#define LINEAR_HASH_DEBUG_ERRORS 0
#define LINEAR_HASH_DEBUG_WRITE_BLOCK 0
#define LINEAR_HASH_DEBUG_READ_BLOCK 0

// printf for Ardunio
#ifdef ARDUINO

#include "../../serial/serial_c_iface.h"

#endif

#if LINEAR_HASH_DEBUG
#define LH_DEBUG_PRINT(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_DEBUG_PRINT(fmt, ...)    /* Don't do anything in release builds */
#endif

#if LINEAR_HASH_DEBUG_INSERT
#define LH_INSERT_DEBUG(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_INSERT_DEBUG(fmt, ...)    /* Don't do anything in release builds */
#endif

#if LINEAR_HASH_DEBUG_DELETE
#define LH_DELETE_DEBUG(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_DELETE_DEBUG(fmt, ...)    /* Don't do anything in release builds */
#endif

#if LINEAR_HASH_DEBUG_SPLIT
#define LH_SPLIT_DEBUG(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_SPLIT_DEBUG(fmt, ...)    /* Don't do anything in release builds */
#endif

#if LINEAR_HASH_DEBUG_WRITE_BLOCK
#define LH_WRITE_BLOCK_DEBUG_PRINT(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_WRITE_BLOCK_DEBUG_PRINT(fmt, ...)    /* Don't do anything in release builds */
#endif

#if LINEAR_HASH_DEBUG_READ_BLOCK
#define LH_READ_BLOCK_DEBUG_PRINT(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_READ_BLOCK_DEBUG_PRINT(fmt, ...)    /* Don't do anything in release builds */
#endif

#if LINEAR_HASH_DEBUG_ERRORS
#define LH_ERROR(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define LH_ERROR(fmt, ...)    /* Don't do anything in release builds */
#endif

#endif //IONDB_LINEAR_HASH_MACROS_H
