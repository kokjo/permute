#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include "vec.h"
#include "dict.h"
#include "util.h"

dict_t *dict_new(size_t size){
    return vec_new(size);
}

dict_entry_t *dict_get_entry(dict_t *dict, uintptr_t key){
    for(int i = 0; i < vec_length(dict); i++){
        dict_entry_t *e = vec_get(dict, i);
        if(e->key == key) return e;
    }
    return NULL;
}

dict_entry_t *dict_get_or_create_entry(dict_t *dict, uintptr_t key){
    dict_entry_t *e = dict_get_entry(dict, key);
    if(e == NULL){
        e = malloc(sizeof(dict_entry_t));
        e->key = key;
        e->value = NULL;
        vec_push(dict, e);
    }
    return e;
}

void *dict_get(dict_t *dict, uintptr_t key){
    dict_entry_t *e = dict_get_entry(dict, key);
    if(e) return e->value;
    return NULL;
}

void *dict_put(dict_t *dict, uintptr_t key, void *value){
    dict_entry_t  *e = dict_get_or_create_entry(dict, key);
    void *tmp = e->value;
    e->value = value;
    return tmp;
}

dict_entry_t *dict_elem(dict_t *dict, size_t idx){
    return vec_get(dict, idx);
}

