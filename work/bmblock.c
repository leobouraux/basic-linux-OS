#include "bmblock.h"
#include "error.h"
#include <stdlib.h>
#include <inttypes.h>
#include <memory.h>

struct bmblock_array *bm_alloc(uint64_t min, uint64_t max){
    if(min > max){
        return NULL;
    }
    uint64_t bits = max - min + 1;
    uint64_t length = bits/BITS_PER_VECTOR;
    if(bits % BITS_PER_VECTOR != 0){
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
    M_REQUIRE_NON_NULL(bmblock_array);
    if(x < bmblock_array->min || bmblock_array->max < x){
        return ERR_BAD_PARAMETER;
    }
    uint64_t x_real = x -bmblock_array->min;
    uint64_t row = bmblock_array->bm[x_real/BITS_PER_VECTOR];
    int elem = (int) ((row >> (x_real % BITS_PER_VECTOR)) & UINT64_C(1));
    return elem;
}


void bm_set(struct bmblock_array *bmblock_array, uint64_t x){
    if(bmblock_array == NULL || x < bmblock_array->min || bmblock_array->max < x){
        return;
    }
    uint64_t x_real = x -bmblock_array->min;
    //shift one at the position of x
    uint64_t one = (uint64_t)(UINT64_C(1) << (x_real % BITS_PER_VECTOR));
    bmblock_array->bm[x_real/BITS_PER_VECTOR] = bmblock_array->bm[x_real/BITS_PER_VECTOR] | one;
}

void bm_clear(struct bmblock_array *bmblock_array, uint64_t x) {
    if (bmblock_array == NULL || x < bmblock_array->min || bmblock_array->max < x) {
        return;
    }
    uint64_t x_real = x - bmblock_array->min;
    uint64_t row = bmblock_array->bm[x_real / BITS_PER_VECTOR];
    uint64_t pos = x_real % BITS_PER_VECTOR;

    //the 'x-1' firsts bits of the row
    uint64_t firsts = (pos == 0) ? 0 : (row << (BITS_PER_VECTOR - pos)) >> (BITS_PER_VECTOR - pos);
    //the 'x+1' lasts bits of the row
    if (pos < BITS_PER_VECTOR-1) {
        row >>= (pos + 1);
        row <<= (pos + 1);
    } else row = 0; //when an overflow occurs
    bmblock_array->bm[x_real / BITS_PER_VECTOR] = firsts | row;

    //change the cursor to know where the last 'smaller' 0 is
    if (bmblock_array->cursor > x_real / BITS_PER_VECTOR)
        bmblock_array->cursor = x_real / BITS_PER_VECTOR;
}

void bm_print(struct bmblock_array *bmblock_array){
    printf("**********BitMap Block START**********\n");
    printf("length: %zu\n", bmblock_array->length);
    printf("min: %"PRIu64"\n", bmblock_array->min);
    printf("max: %"PRIu64"\n", bmblock_array->max);
    printf("cursor: %"PRIu64"\n", bmblock_array->cursor);
    printf("content: \n");
    for (unsigned int i = 0; i < bmblock_array->length; ++i) {
        uint64_t current = bmblock_array->bm[i];
        printf("%d:", i);
        for (unsigned int j = 0; j < BITS_PER_VECTOR; ++j) {
            if(j % 8 == 0){
                printf(" ");
            }
            printf("%"PRIu64"", current & 1);
            current >>= 1;
        }
        printf("\n");
    }
    printf("**********BitMap Block END************\n");
}


int bm_find_next(struct bmblock_array *bmblock_array){
    M_REQUIRE_NON_NULL(bmblock_array);
    uint64_t current_row = bmblock_array->bm[bmblock_array->cursor];
    uint64_t init_cursor = bmblock_array->cursor;
    int i = 0;
    while ((current_row & 1) !=0) {
        current_row >>= 1;
        if(i%BITS_PER_VECTOR == BITS_PER_VECTOR-1  &&  bmblock_array->cursor < bmblock_array->length-1) {
            current_row = bmblock_array->bm[++bmblock_array->cursor];
        }
        i++;
    }
    if(bmblock_array->bm[bmblock_array->cursor] == UINT64_C(-1))
        return ERR_BITMAP_FULL;

    return (int)(init_cursor*BITS_PER_VECTOR + i+bmblock_array->min);
}


