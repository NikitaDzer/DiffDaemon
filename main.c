#include <stdio.h>
#include <stdlib.h>

#include "include/io.h"
#include "include/sem.h"
#include "include/error.h"
#include "include/signal.h"
#include "include/daemon.h"
#include "include/cmdline.h"


int main( const int argc, const char *argv[])
{
        const char *pid_str = NULL;
        const ProgMode mode = parse_cmdline( argc, argv, &pid_str);

        if ( mode == PROG_BAD_MODE )
        {
                return EXIT_FAILURE;
        }

        int res = create_sem();
        if ( is_sys_err( res) )
        {
                return EXIT_FAILURE;
        }

        const int dem_pid = dem_fork();
        if ( is_sys_err( dem_pid) )
        {
                free_sem();
                return EXIT_FAILURE;
        }

        if ( dem_pid == 0 )
        {
                res = dem_init();
                if ( is_sys_err( res) )
                {
                        dem_free();
                        signal_io_close();
                        return EXIT_FAILURE;
                }

                res = dem_run( pid_str);
                if ( is_sys_err( res) )
                {
                        dem_free();
                        signal_io_close();
                        return EXIT_FAILURE;
                }

                res = dem_free();
                if ( is_sys_err( res) )
                {
                        return EXIT_FAILURE;
                }

                return EXIT_SUCCESS;
        }
        
        if ( mode == PROG_DEM_MODE )
        {
                res = io_fork();
                if ( is_sys_err( res) )
                {
                        free_sem();
                        signal_dem_close( dem_pid);
                        return EXIT_FAILURE;
                }
        }

        res = io_init( mode);
        if ( is_sys_err( res) )
        {
                io_free( mode);
                signal_dem_close( dem_pid);
                return EXIT_FAILURE;
        }

        res = io_run( mode, dem_pid);
        if ( is_sys_err( res) )
        {
                io_free( mode);

                if ( !is_dem_failed() )
                {
                        signal_dem_close( dem_pid);
                }

                return EXIT_FAILURE;
        }

        signal_dem_close( dem_pid);

        res = io_free( mode);
        if ( is_sys_err( res) )
        {
                return EXIT_FAILURE;
        }
        
        return EXIT_SUCCESS;
}
