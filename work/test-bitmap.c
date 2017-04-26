#include "bmblock.h"

int main(){
    //struct bmblock_array * bm = bm_alloc(4, 131);
    struct bmblock_array * bm = bm_alloc(4, 195);
    int next = bm_find_next(bm);
    bm_print(bm);
    printf("find_next() = %d\n", next);
    bm_set(bm, 4);
    bm_set(bm, 5);
    bm_set(bm, 6);
    next = bm_find_next(bm);
    bm_print(bm);
    printf("find_next() = %d\n", next);
    for (uint64_t i = bm->min; i <= bm->max; i += 3) {
        bm_set(bm, i);
    }
    next = bm_find_next(bm);
    bm_print(bm);
    printf("find_next() = %d\n", next);
    for (uint64_t i = bm->min+1; i <= bm->max; i += 5) {
        bm_clear(bm, i);
    }
    next = bm_find_next(bm);
    bm_print(bm);
    printf("find_next() = %d\n", next);

    //debug
    for (uint64_t i = bm->min; i <= bm->max; i++) {
        bm_clear(bm, i);
    }

    for (uint64_t i = bm->min; i <= bm->max; i++) {
        bm_set(bm, i);
    }

    bm_clear(bm, 4+7 + (24-1)*8);
    next = bm_find_next(bm);
    bm_print(bm);
    printf("find_next() = %d\n",next);
    //
    return 0;
}


//set(bm, 5) quand max = 4  --> set le 2eme