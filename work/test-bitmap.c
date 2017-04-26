#include "bmblock.h"

int main(){
    struct bmblock_array * bm = bm_alloc(4, 131);
    bm_print(bm);
    printf("find_next() = %d\n",bm_find_next(bm));
    bm_set(bm, 4);
    bm_set(bm, 5);
    bm_set(bm, 6);
    bm_print(bm);
    printf("find_next() = %d\n",bm_find_next(bm));
    for (uint64_t i = bm->min; i < bm->max; i += 3) {
        bm_set(bm, i);
    }
    bm_print(bm);
    printf("find_next() = %d\n",bm_find_next(bm));
    for (uint64_t i = bm->min+1; i < bm->max; i += 5) {
        bm_clear(bm, i);
    }
    bm_set(bm, 38);
    bm_print(bm);
    printf("find_next() = %d\n",bm_find_next(bm));
    return 0;
}