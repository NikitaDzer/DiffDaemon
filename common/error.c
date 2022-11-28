#include "../include/error.h"


int RET_SYS_ERR( void)
{
        return SYS_DEFAULT_ERR;
}

bool is_sys_err( const int err)
{
        return err <= SYS_DEFAULT_ERR;
}
