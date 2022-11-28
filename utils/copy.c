#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <sys/wait.h>
#include <unistd.h>

#include "../include/error.h"
#include "copy.h"


int copy_file( const char *const dest_path, const char *const src_path)
{
        const pid_t pid = fork();
        if ( pid == -1 )
        {
                perror( "fork");
                return ret_sys_err();
        }

        if ( pid == 0 )
        {
                execlp( COPY_UTIL_NAME, 
                        COPY_UTIL_NAME,
                        src_path, dest_path, NULL);
                assert( 0 && "Unreachable");
        } 
        else
        {
                int wstatus = 0;
                wait( &wstatus);

                return 0;
        }
}
