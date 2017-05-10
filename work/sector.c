#include <stdio.h>
#include "mount.h"
#include "error.h"
#include "sector.h"



int sector_read(FILE *f, uint32_t sector, void *data){
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);

    //set file cursor and read the wanted sector
    fseek(f, sector*SECTOR_SIZE, SEEK_SET);
    size_t j = 0;
    j = fread(data, sizeof(uint8_t),SECTOR_SIZE, f);
    if(j == 0){
        return ERR_IO;
    }
    return 0;
}

int sector_write(FILE *f, uint32_t sector, void *data){ //see const
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);

    int err = fseek(f, sector*SECTOR_SIZE, SEEK_SET);
    size_t j = fwrite(data, sizeof(uint8_t), SECTOR_SIZE, f);
    if(j == 0){
        return ERR_IO;
    }
    return 0;
}
