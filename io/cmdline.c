#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "include/cmdline.h"
#include "include/config.h"


typedef enum InputError
{
        INPUT_NO_MODE         = 1,
        INPUT_PLURAL_MODES    = 2,
        INPUT_PLURAL_PIDS     = 3,
        INPUT_UNSPECIFIED_PID = 4,
} InputError;


static bool is_help_mode( const int argc, const char *argv[])
{
        for ( int i = 1; i < argc; i++ )
        {
                if ( strcmp( argv[i], OPT_HELP) == 0 )
                {
                        return true;
                }
        }

        return false;
}

static bool is_dem_opt( const char *const arg)
{
        return strcmp( arg, OPT_DEM) == 0;
} 

static bool is_int_opt( const char *const arg)
{
        return strcmp( arg, OPT_INT) == 0;
}

static bool is_pid_opt( const char *const arg)
{
        return strcmp( arg, OPT_PID) == 0;
}

static void print_help()
{
        printf( "## Diff Daemon (or DD) ##\n"
                "Options:\n"
                "\t%s         Print help message.\n"
                "\t%s         Use DD as daemon.\n"
                "\t%s         Use DD in the interactive mode.\n"
	            "\t%s <pid>   Specify process <pid> to track its cwd (default = DD pid).\n",
                OPT_HELP, OPT_DEM, OPT_INT, OPT_PID);
}

static void print_error( const InputError error)
{
        switch ( error )
        {
                case INPUT_NO_MODE:
                        printf( "DD mode is not specified.\n");
                        break;

                case INPUT_PLURAL_MODES:
                        printf( "Cannot use DD in plural modes.\n");
                        break;
                
                case INPUT_PLURAL_PIDS:
                        printf( "Only one process can be watched.\n");
                        break;

                case INPUT_UNSPECIFIED_PID:
                        printf( "Missed pid after option %s.\n", OPT_PID);
                        break;

                default:
                        assert( 0 && "Unreachable.\n");
                        break;
        }
}


ProgMode parse_cmdline( const int argc, const char *argv[],
                        const char **const pid_str)
{
        static char pid_buffer[MAX_PATH_SZ] = { 0 };

        if ( is_help_mode( argc, argv) )
        {
                print_help();

                return PROG_HELP_MODE;
        }

        bool was_dem_opt = false;
        bool was_int_opt = false;
        bool was_pid_set = false;

        int pid_opt_count = 0;
        int pid           = 0;

        for ( int i = 1; i < argc; i++ )
        {
                const char *const arg = argv[i];

                if ( is_dem_opt( arg) )
                {
                        was_dem_opt = true;
                }
                else if ( is_int_opt( arg) )
                {
                        was_int_opt = true;
                }
                else if ( is_pid_opt( arg) )
                {
                        pid_opt_count++;

                        if ( i != argc - 1 )
                        {
                                i++;
                                *pid_str = argv[i];

                                was_pid_set = true;
                        }
                }
        }

        if ( pid_opt_count == 0 )
        {
                snprintf( pid_buffer, sizeof( pid_buffer), "%d", getpid());

                *pid_str = pid_buffer;
                was_pid_set = true;
        }

        if ( !(was_dem_opt || was_int_opt) )
        {
                print_error( INPUT_NO_MODE);

                return PROG_BAD_MODE;
        }

        if ( was_dem_opt && was_int_opt )
        {
                print_error( INPUT_PLURAL_MODES);

                return PROG_BAD_MODE;
        }

        if ( pid_opt_count >= 2 )
        {
                print_error( INPUT_PLURAL_PIDS);

                return PROG_BAD_MODE;
        }

        if ( !was_pid_set )
        {
                print_error( INPUT_UNSPECIFIED_PID);

                return PROG_BAD_MODE;
        }
        
        if ( was_dem_opt )
        {
                return PROG_DEM_MODE;
        }

        if ( was_int_opt )
        {
                return PROG_INT_MODE;
        }

        assert( 0 && "Unreachable.\n");
}
