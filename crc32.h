#ifndef CRC32_H_
#define CRC32_H_

#include <stdlib.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    uint32_t
    compute_crc32(uint32_t in_crc32,
            const void *buf,
            size_t buf_len);

#ifdef	__cplusplus
}
#endif

#endif // CRC32_H_