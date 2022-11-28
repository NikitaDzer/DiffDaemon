#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "../include/daemonize.h"
#include "../include/error.h"


static int ignore_signals( void)
{
        if ( signal( SIGINT, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }
        
        if ( signal( SIGABRT, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }

        if ( signal( SIGQUIT, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }

        if ( signal( SIGTRAP, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }

        if ( signal( SIGTERM, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }

        if ( signal( SIGCONT, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }

        if ( signal( SIGTSTP, SIG_IGN) == SIG_ERR )
        {
                return ret_sys_err();
        }

        return 0;
}

static int close_descriptors( void)
{
        if ( close( STDIN_FILENO) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        if ( close( STDOUT_FILENO) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        if ( close( STDERR_FILENO) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        return 0;
}

int daemonize( int *const pid)
{
        *pid = fork();
        if ( *pid == -1 )
        {
                perror( "fork");
                return ret_sys_err();
        }

        if ( *pid != 0 )
        {
                return 0;
        }

        umask( 0);

        if ( setsid() == -1 )
        {
                perror( "setsid");
                return ret_sys_err();
        }

        if ( chdir( "/") == -1 )
        {
                perror( "chdir");
                return ret_sys_err();
        }

        int res = close_descriptors();
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }
        
        res = ignore_signals();
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}
