#include <stdio.h>
#include "mount.h"
#include "error.h"


/** 
 * week04
 * 
 * @brief read one 512-byte sector from the virtual disk
 * @param f open file of the virtual disk
 * @param sector the location (in sector units, not bytes) within the virtual disk
 * @param data a pointer to 512-bytes of memory (OUT)
 * @return 0 on success; <0 on error
 */
int sector_read(FILE *f, uint32_t sector, void *data){
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);
    fseek(f, sector*SECTOR_SIZE, SEEK_SET);
    size_t j = 0;
    j = fread(data, sizeof(uint8_t),SECTOR_SIZE, f);
    if(j == 0){
        return ERR_IO;
    }
    return 0;
}
