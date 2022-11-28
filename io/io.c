#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "../include/daemonize.h"
#include "../include/action.h"
#include "../include/backup.h"
#include "../include/config.h"
#include "../include/signal.h"
#include "../include/error.h"
#include "../include/watch.h"
#include "../include/shmem.h"
#include "../include/sem.h"
#include "../include/vfs.h"
#include "../include/cwd.h"
#include "../include/io.h"


typedef enum IOError
{
        NO_ERROR        = 0,
        ERROR_BAD_INPUT = 1,
        ERROR_DEM_FAIL  = 2,
        ERROR_IO_FAIL   = 3,
} IOError;


static const char NO_ERROR_MSG[]        = "";
static const char ERROR_BAD_INPUT_MSG[] = "Bad input.\n";
static const char ERROR_DEM_FAIL_MSG[]  = "Daemon failure.\n";
static const char ERROR_IO_FAIL_MSG[]   = "IO failure.\n";

static const char ACTION_CLOSE_TEXT [] = "close";
static const char ACTION_APPLY_TEXT [] = "apply";
static const char ACTION_DIFF_TEXT  [] = "diff";
static const char ACTION_CHPID_TEXT [] = "chpid";
static const char ACTION_CHTIME_TEXT[] = "chtime";


static volatile bool dem_running = true;


static bool is_dem_running( void)
{
        return dem_running;
}

static int print_buf( const char *const buf, const int size)
{
        if ( write( STDOUT_FILENO, buf, size) == -1 )
        {
                perror( "write");
                return ret_sys_err();
        }

        sync();
        
        return 0;
}

static int print_error( const IOError err)
{
        const char *msg = NULL;

        switch ( err )
        {
                case NO_ERROR:
                        msg = NO_ERROR_MSG;
                        break;

                case ERROR_BAD_INPUT:
                        msg = ERROR_BAD_INPUT_MSG;
                        break;

                case ERROR_DEM_FAIL:
                        msg = ERROR_DEM_FAIL_MSG;
                        break;

                case ERROR_IO_FAIL:
                        msg = ERROR_IO_FAIL_MSG;
                        break;

                default:
                        assert( 0 && "Unreachable.\n");
                        break;
        }

        return print_buf( msg, strlen(msg));
}

static int print_diff( void)
{
        wait_sem();

        char tmpbuf[MAX_PATH_SZ] = { 0 };
        int nbytes = 0;
        int res    = 0;

        FILE *const tmpfile = fopen( TMPFILE_PATH, "r");
        if ( tmpfile == NULL )
        {
                perror( "fopen");
                return ret_sys_err();
        }

        while ( !feof( tmpfile))
        {
                nbytes = fread( tmpbuf, sizeof( char), MAX_PATH_SZ, tmpfile);
                if ( nbytes == -1 )
                {
                        perror( "fread");
                        fclose( tmpfile);
                        return ret_sys_err();
                }
                                
                res = print_buf( tmpbuf, nbytes);
                if ( is_sys_err( res) )
                {
                        fclose( tmpfile);
                        return ret_sys_err();
                }
       }

        if ( fclose( tmpfile) == -1 )
        {
                perror( "tmpfile");
                return ret_sys_err();
        }

        return 0;
}

static int handle_input( Action *const action)
{
        char  input[MAX_PATH_SZ] = { 0 };
        char   type[MAX_PATH_SZ] = { 0 };

        const int nbytes = read( STDIN_FILENO, input, MAX_PATH_SZ);
        if ( nbytes == -1 )
        {
                perror( "read");
                return ret_sys_err();
        }

        action->type = NO_ACTION;

        if ( nbytes == 0 )
        {
                return 0;
        }

        if ( nbytes == MAX_PATH_SZ )
        {
                input[nbytes - 1] = '\0';
        }

        if ( sscanf( input, "%s", type) == 0 )
        {       
                return 0;
        }

        if ( strncmp( type, 
                      ACTION_APPLY_TEXT, 
                      sizeof( ACTION_APPLY_TEXT)) == 0 )
        {
                char tmpbuf[MAX_PATH_SZ] = { 0 };
                int rev_state = 0;

                if ( sscanf( input + strlen( type), 
                             "%s %d",
                             tmpbuf, &rev_state) != 2 )
                {
                        return print_error( ERROR_BAD_INPUT);
                }

                action->type = ACTION_APPLY;
                strcpy( action->arg1.str, tmpbuf);
                action->arg2.num = rev_state;
        
                return 0;
        }

        if ( strncmp( type, 
                      ACTION_DIFF_TEXT,
                      sizeof( ACTION_DIFF_TEXT)) == 0 )
        {
                char tmpbuf[MAX_PATH_SZ] = { 0 };
                int rev_state = 0;

                if ( sscanf( input + strlen( type), 
                             "%s %d",
                             tmpbuf, &rev_state) != 2 )
                {
                        return print_error( ERROR_BAD_INPUT);
                }

                action->type = ACTION_DIFF;
                strcpy( action->arg1.str, tmpbuf);
                action->arg2.num = rev_state;

                return 0;
        }
        
        if ( strncmp( type, 
                      ACTION_CHPID_TEXT,
                      sizeof( ACTION_CHPID_TEXT)) == 0 )
        {
                char tmpbuf[MAX_PATH_SZ] = { 0 };

                if ( sscanf( input + strlen( type),
                             "%s",
                             tmpbuf) != 1 )
                {
                        return print_error( ERROR_BAD_INPUT);
                }

                action->type = ACTION_CHPID;
                strcpy( action->arg1.str, tmpbuf);

                return 0;
        }

        if ( strncmp( type,
                      ACTION_CHTIME_TEXT,
                      sizeof( ACTION_CHTIME_TEXT)) == 0 )
        {
                double time = 0.0;
                
                if ( sscanf( input + strlen( type),
                             "%lg",
                             &time) != 1 )
                {
                        return print_error( ERROR_BAD_INPUT);
                }

                action->type      = ACTION_CHTIME;
                action->arg1.dnum = time;

                return 0;
        }

        if ( strncmp( type, 
                      ACTION_CLOSE_TEXT, 
                      sizeof( ACTION_CLOSE_TEXT)) == 0 )
        {
                action->type = ACTION_CLOSE;

                return 0;
        }


        return print_error( ERROR_BAD_INPUT);
}

static int make_fifo( const char *const path)
{
        const mode_t FIFO_MODE = S_IRUSR | S_IWUSR;

        if ( mkfifo( path, 0777) == -1 )
        {
                perror( "mkfifo");
                return ret_sys_err();
        }

        return 0;
}

static int unlink_fifo( const char *const path)
{
        if ( unlink( path) == -1 )
        {
                perror( "unlink");
                return ret_sys_err();
        }

        return 0;
}

static int open_input( const char *const path)
{
        const int res = make_fifo( path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        int fd = open( path, O_RDONLY);
        if ( fd == -1)
        {
                perror( "open");
                return ret_sys_err(); 
        }

        if ( dup2( STDIN_FILENO, fd) == -1 )
        {
                perror( "dup2");
                return ret_sys_err();
        }

        return 0;
}

static int close_input( const char *const path)
{
        if ( close( STDIN_FILENO) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        return unlink_fifo( path);
}

static int open_output( const char *const path)
{
        const int fd = open( path, O_CREAT | O_RDWR, 0666);
        if ( fd == -1 )
        {
                perror( "open");
                return ret_sys_err();
        }

        if ( dup2( STDOUT_FILENO, fd) == -1 )
        {
                perror( "dup2");
                return ret_sys_err();
        }

        return 0;
}

static int close_output( const char *const path)
{
        if ( close( STDOUT_FILENO) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        if ( remove( path) == -1 )
        {
                perror( "remove");
                return ret_sys_err();
        }

        return 0;
}

bool is_dem_failed( void)
{
        return !dem_running;
}

void io_close( void)
{
        dem_running = false;
}

int io_free( const ProgMode mode)
{
        if ( mode == PROG_DEM_MODE )
        {
                int res = free_shm();
                if ( is_sys_err( res) )
                {
                        free_sem();
                        close_input ( INPUT_PATH);
                        close_output( OUTPUT_PATH);

                        return ret_sys_err();
                }

                res = free_sem();
                if ( is_sys_err( res) )
                {
                        close_input ( INPUT_PATH);
                        close_output( OUTPUT_PATH);
                        return ret_sys_err();
                }

                res = close_input( INPUT_PATH);
                if ( is_sys_err( res) )
                {
                        close_output( OUTPUT_PATH);
                        return ret_sys_err();
                }

                res = close_output( OUTPUT_PATH);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();        
                }
        }
        else
        {
                int res = free_shm();
                if ( is_sys_err( res) )
                {
                        free_sem();
                        return ret_sys_err();
                }

                res = free_sem();
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        return 0;
}

int io_fork( void)
{
        int pid = 0;

        const int res = daemonize( &pid);

        if ( pid == -1 )
        {
                return ret_sys_err();
        }

        if ( pid > 0 )
        {
                if ( is_sys_err( res) )
                {
                        exit( EXIT_FAILURE);
                }

                exit( EXIT_SUCCESS);
        }

        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}

int io_init( const ProgMode mode)
{
        int res = init_io_handler();
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        res = create_shm();
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        if ( mode == PROG_DEM_MODE )
        {
                res = open_input( INPUT_PATH);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }

                res = open_output( OUTPUT_PATH);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        return 0;
}

int io_run( const ProgMode mode, const pid_t dem_pid)
{
        Action action = { 0 };
        int res = 0;

        while ( is_dem_running() )
        {
                res = handle_input( &action);
                if ( is_sys_err( res) )
                {
                        break;
                }
                
                if ( action.type == ACTION_CLOSE )
                {
                        break;
                }

                res = signal_dem( &action, dem_pid);
                if ( is_sys_err( res) )
                {
                        break;
                }
                
                if ( action.type == ACTION_DIFF )
                {
                        res = print_diff();
                        if ( is_sys_err( res) )
                        {
                                break;
                        }
                }
        }

        if ( is_sys_err( res) )
        {
                print_error( ERROR_IO_FAIL);
        }
        
        if ( is_dem_failed() )
        {
                print_error( ERROR_DEM_FAIL);
        }

        if ( is_sys_err( res) || is_dem_failed() )
        {
                return ret_sys_err();
        }

        return 0;
}
