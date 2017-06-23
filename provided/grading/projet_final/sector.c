#include <stdio.h>
#include "mount.h"
#include "error.h"
#include "sector.h"



int sector_read(FILE *f, uint32_t sector, void *data){
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);

    //set file cursor and read the wanted sector
    int err = fseek(f, sector*SECTOR_SIZE, SEEK_SET);
    if(err < 0){
        return ERR_IO;
    }
    size_t read_size = fread(data, sizeof(uint8_t),SECTOR_SIZE, f);
    if(read_size != SECTOR_SIZE){
        return ERR_IO;
    }
    return 0;
}

int sector_write(FILE *f, uint32_t sector, const void *data){
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);

    int err = fseek(f, sector*SECTOR_SIZE, SEEK_SET);
    if(err < 0){
        return ERR_IO;
    }
    size_t write_size = fwrite(data, sizeof(uint8_t), SECTOR_SIZE, f);
    if(write_size != SECTOR_SIZE){
        return ERR_IO;
    }
    return 0;
}
