#ifndef HASH_H
#define HASH_H


typedef unsigned long long hash_t;

#define HASH_SPECIFIER "ull"


hash_t qhash( const void *const key, const unsigned bytes);


#endif // HASH_H
