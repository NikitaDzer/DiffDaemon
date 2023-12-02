#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

#include "include/config.h"
#include "include/cwd.h"


const char *find_cwd( const char *const pid_str)
{
        static char cwd_path[MAX_PATH_SZ] = { 0 };
        memset( cwd_path, 0, sizeof( char) * MAX_PATH_SZ);

        char link_path[MAX_PATH_SZ] = { 0 };
        strcat( link_path, "/proc/");
        strcat( link_path, pid_str);
        strcat( link_path, "/cwd");

        ssize_t n_bytes = readlink( link_path, cwd_path, MAX_PATH_SZ);
        if ( n_bytes == -1 )
        {
                perror( "readlink");
                return NULL;
        }

        cwd_path[n_bytes] = '\0';

        return cwd_path; 
}
