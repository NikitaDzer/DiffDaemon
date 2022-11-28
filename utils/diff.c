#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/error.h"
#include "diff.h"


int diff( const char *const file1_path,
          const char *const file2_path,
          const char *const  dest_path)
{
        const pid_t pid = fork();
        if ( pid == -1 )
        {
                perror( "fork");
                return ret_sys_err();
        }

        if ( pid == 0 )
        {
                int fd = open( dest_path, O_CREAT | O_RDWR, 0666);
                if ( fd == -1 )
                {
                        perror( "open");
                        exit( ret_sys_err());
                }

                if ( dup2( fd, STDOUT_FILENO) == -1 )
                {
                        perror( "dup2");
                        close( fd);
                        exit( ret_sys_err());
                }

                execlp( DIFF_UTIL_NAME,
                        DIFF_UTIL_NAME, 
                        file2_path, file1_path, 
                        DIFF_UTIL_FORMAT,
                        NULL);
                assert( 0 && "Unreachable");
        }
        else
        {
                int wstatus = 0;
                wait( &wstatus);

                return 0;
        }
}
