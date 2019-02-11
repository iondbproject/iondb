//
// Created by itsal on 2019-02-10.
//

#ifndef IONDB_HASH_FUNCTIONS_H
#define IONDB_HASH_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

/* SDBM
 * A 32 bit implementation of the SDBM hashing algorithm for the Arduino microcontroller. Originally designed for SDBM, an open
 * source clone of the NDBM database library, this algorithm was proposed by Ozan Yigit (York) and Margo Seltzer (UC Berkeley)
 * in 1991 and is based off an earlier algorithm created by Per-Ake Larson in 1988.
 * Output is in the form of an 8 character hexadecimal string.
 * Author: Matthew Fritter
 */
uint32_t
sdbm(char *key, size_t size);

/* Jenkins One At a Time
 * An implementation of the Jenkins One-At-A-Time (JOAAT) algorithm for the Arduino microcontroller that
 * produces a 32 bit output hash. Based on code from Bob Jenkins.
 * Output is in the form a uint32_t unsigned 32 bit integer hash
 * Author: Matthew Fritter
 */
uint32_t
jenkins(char *key, size_t size);

/* FNV-1a
 * An implementation of the Fowler–Noll–Vo FNV-1a Hashing algorithm for the Arduino microcontroller that produces a 32 bit output hash
 * Using the recommended prime of 0x1000193 and the recommended offset of 0x811C9DC5.
 * Output is in the form a uint32_t unsigned 32 bit integer hash
 * Author: Matthew Fritter
 */
uint32_t
fnv1a(char *key, size_t size);

/**
 * Implementation of djb2 simple hashing
 * @param key
 * @param size
 * @return
 */
uint32_t
djb2(char *key, size_t size);

/**
 * Implementation of SuperFastHash from http://www.azillionmonkeys.com/qed/hash.html
 *
 * Author: Paul Hsieh
 * Licence: LGPL 2.1 license
 */
uint32_t SuperFastHash(char *data, size_t len);


#endif //IONDB_HASH_FUNCTIONS_H

