#ifndef _SET_H_
#define _SET_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "vec.h"

typedef vec_t set_t;

typedef struct set_entry {
    uintptr_t item;
} set_entry_t;

set_t *set_new(size_t size);
bool set_contains(set_t *set, uintptr_t item);
bool set_add(set_t *set, uintptr_t item);
void set_extend(set_t *set, set_t *from);

#endif
