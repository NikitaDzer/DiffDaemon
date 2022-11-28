#ifndef ERROR_H
#define ERROR_H


#include <stdbool.h>


static const int SYS_DEFAULT_ERR = -1;


#define ret_sys_err() \
        (fprintf( stderr, "ERROR: %s:%d\n", __FILE__, __LINE__), fflush( stderr), RET_SYS_ERR())


int  RET_SYS_ERR( void);
bool  is_sys_err( const int err);


#endif // ERROR_H
