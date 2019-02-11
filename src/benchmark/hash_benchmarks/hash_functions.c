//
// Created by itsal on 2019-02-10.
//

#include "hash_functions.h"

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
 || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t
sdbm(char *key, size_t size) {
    uint32_t hash = 0;
    for (size_t i = 0; i < size; i++) {
        hash = key[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

uint32_t
jenkins(char *key, size_t size) {
    uint32_t hash = 0;
    for (size_t i = 0; i < size; i++) {
        hash += key[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

uint32_t fnv1a(char *key, size_t size) {
    const uint32_t prime = 0x1000193;
    uint32_t hash = 0x811C9DC5;
    for (size_t i = 0; i < size; i++) {
        hash ^= key[i];
        hash *= prime;
    }
    return hash;
}

uint32_t djb2(char *key, size_t size) {
    uint32_t hash = 5381;
    for (size_t i = 0; i < size; i++)
        hash = ((hash << 5) + hash) + key[i]; /* hash * 33 + c */
    return hash;
}

uint32_t SuperFastHash(char *data, size_t len) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--) {
        hash += get16bits (data);
        tmp = (get16bits (data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3:
            hash += get16bits (data);
            hash ^= hash << 16;
            hash ^= ((signed char) data[sizeof(uint16_t)]) << 18;
            hash += hash >> 11;
            break;
        case 2:
            hash += get16bits (data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1:
            hash += (signed char) *data;
            hash ^= hash << 10;
            hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
