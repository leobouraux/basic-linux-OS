#include "bmblock.h"
#include "error.h"
#include <memory.h>
#include <stdlib.h>
#include <stdint.h>


struct bmblock_array *bm_alloc(uint64_t min, uint64_t max){
    if(min > max){
        return NULL;
    }
    uint64_t bits = max - min + 1;
    uint64_t length = bits/64;
    if(bits % 64 != 0){
        length += 1;
    }
    struct bmblock_array* bm = NULL;
    bm = malloc(sizeof(struct bmblock_array)+(length-1)*sizeof(uint64_t));
    if(bm == NULL){
        return bm;
    }
    bm->min = min;
    bm->max = max;
    bm->length = length;
    bm->cursor = 0;
    memset(bm->bm, 0, sizeof(uint64_t));
    return bm;
}

int bm_get(struct bmblock_array *bmblock_array, uint64_t x){
    if(x < bmblock_array->min || bmblock_array->max < x){
        return ERR_BAD_PARAMETER;
    }
    uint64_t x_real = x -bmblock_array->min;
    uint64_t row = bmblock_array->bm[x_real/64];
    int elem = (int)(row >> (x_real % 64)) & 1;
    return elem;
}

void bm_set(struct bmblock_array *bmblock_array, uint64_t x){
    if(x < bmblock_array->min || bmblock_array->max < x){
        return;
    }
    uint64_t x_real = x -bmblock_array->min;
    uint64_t tr = (uint64_t)(1 << (x_real % 64));
    bmblock_array->bm[x_real/64] = bmblock_array->bm[x_real/64] | tr;
}

void bm_clear(struct bmblock_array *bmblock_array, uint64_t x){
    if(x < bmblock_array->min || bmblock_array->max < x){
        return;
    }
    uint64_t x_real = x -bmblock_array->min;
    uint64_t row = bmblock_array->bm[x_real/64];

    uint64_t firsts = (row >> (64 - x_real%64)) << (64 - x_real%64);
    uint64_t lasts = (row << ((x_real%64)+1)) >> x_real%64;

    bmblock_array->bm[x_real/64] = firsts | lasts;

}

void bm_print(struct bmblock_array *bmblock_array){
    printf("**********BitMap Block START**********\n");
    printf("length: %", bmblock_array->length);
    

}


