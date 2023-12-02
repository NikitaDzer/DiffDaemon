#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "include/error.h"
#include "utils/apply.h"


int apply( const char *const  file_path,
           const char *const patch_path,
           const ApplyMode mode)
{
        const pid_t pid = fork();
        if ( pid == -1 )
        {
                perror( "fork");
                return ret_sys_err();
        }

        if ( pid == 0 )
        {
                switch ( mode )
                {
                        case APPLY_MODE_FORWARD:
                                execlp( APPLY_UTIL_NAME,
                                        APPLY_UTIL_NAME, 
                                        file_path, patch_path,
                                        NULL);

                        case APPLY_MODE_REVERT:
                                execlp( APPLY_UTIL_NAME,
                                        APPLY_UTIL_NAME,
                                        file_path, patch_path,
                                        APPLY_UTIL_REVERT_OPT,
                                        NULL);

                        default:
                                assert( 0 && "Unreachable");
                                break; 
                }
        }
        else
        {
                int wstatus = 0;
                wait( &wstatus);

                return 0;
        }
}
