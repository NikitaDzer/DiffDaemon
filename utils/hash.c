#include "hash.h"


hash_t qhash( const void *const key, const unsigned bytes)
{
        const char *byte      = (const char *)key;
        const char *last_byte = byte + bytes - 1;
        hash_t      hash      = 0xDED007;

        while ( byte <= last_byte )
        {
                hash = (( hash << 0x8 ) + (hash >> 0x8)) ^ *byte++;
        }

        return hash;
}
