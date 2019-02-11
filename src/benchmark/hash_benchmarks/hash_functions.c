//
// Created by itsal on 2019-02-10.
//

#include "hash_functions.h"

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
