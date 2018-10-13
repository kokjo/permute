#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "vec.h"
#include "set.h"

set_t *set_new(size_t size){
    return vec_new(size);
}

bool set_contains(set_t *set, uintptr_t item){
    for(int i = 0; i < vec_length(set); i++){
        set_entry_t *e = vec_get(set, i);
        if(e->item == item) return true;
    }
    return false;
}

bool set_add(set_t *set, uintptr_t item) {
    if(set_contains(set, item)) return false;
    set_entry_t *e = malloc(sizeof(set_entry_t));
    e->item = item;
    vec_push(set, e);
    return true;
}

void set_extend(set_t *set, set_t *from) {
    for(int i = 0; i < vec_length(from); i++) {
        set_entry_t *e = vec_get(from, i); 
        set_add(set, e->item);
    }
}
