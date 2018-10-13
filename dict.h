#ifndef _DICT_H_
#define _DICT_H_
#include <stddef.h>
#include <inttypes.h>
#include "vec.h"
#include "dict.h"

typedef struct dict_entry {
    uintptr_t key;
    void *value;
} dict_entry_t;

typedef vec_t dict_t;
dict_t *dict_new(size_t size);
dict_entry_t *dict_get_entry(dict_t *dict, uintptr_t key);
dict_entry_t *dict_get_or_create_entry(dict_t *dict, uintptr_t key);
void *dict_get(dict_t *dict, uintptr_t key);
void *dict_put(dict_t *dict, uintptr_t key, void *value);
dict_entry_t *dict_elem(dict_t *dict, size_t idx);

#endif
