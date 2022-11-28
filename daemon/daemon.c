#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/stat.h>
#include <unistd.h>

#include "../include/daemonize.h"
#include "../include/inotify.h"
#include "../include/daemon.h"
#include "../include/backup.h"
#include "../include/signal.h"
#include "../include/config.h"
#include "../include/watch.h"
#include "../include/event.h"
#include "../include/error.h"
#include "../include/sem.h"
#include "../include/cwd.h"


static bool dem_running  = true;
static const char *cwd   = NULL;
static double sleep_time = 1.0;


static const char *get_file_path( const char *const rel_path)
{
        static char file_path[MAX_PATH_SZ] = { 0 };
       
        memset( file_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( file_path, cwd);
        strcat( file_path, "/");
        strcat( file_path, rel_path);

        return file_path;
}

static bool is_dem_running( void)
{
        return dem_running;
}

void dem_apply( const char *const rel_path, const int rev_state)
{
        const int res = apply_state( get_file_path( rel_path), rev_state);
        if ( is_sys_err( res) )
        {
                dem_running = false;
        }
}

void dem_diff( const char *const rel_path, const int rev_state)
{
        const int res = write_diff( get_file_path( rel_path), rev_state);
        if ( is_sys_err( res) )
        {
                dem_running = false;
        }
}

void dem_chpid( const char *const pid_str)
{
        cwd = find_cwd( pid_str);
        if ( cwd == NULL )
        {
                dem_running = false;
                return;
        }

        const int res = set_inotify_root( cwd);
        if ( is_sys_err( res) )
        {
                dem_running = false;
        }
}

void dem_chtime( const double time)
{
        sleep_time = time;
}

int dem_fork( void)
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

                return pid;
        }

        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return pid;
}

int dem_init( void)
{
        int res = init_dem_handler();
        if ( is_sys_err( res) )
        {
                return res;
        }

        return 0;
}

int dem_free( void)
{
        free_watch_bank();
        free_event_bank();
        
        return free_ifd();
}

void dem_close( void)
{
        dem_running = false;
}

int dem_run( const char *const pid_str)
{
        cwd = find_cwd( pid_str);
        if ( cwd == NULL )
        {
                return ret_sys_err();
        }

        const int ifd = get_ifd();
        if ( is_sys_err( get_ifd()) )
        {
                return ret_sys_err();
        }

        int res = set_inotify_root( cwd);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        while ( is_dem_running() )
        {
                sleep( sleep_time);

                res = poll_inotify();
                if ( is_sys_err( res) )
                {
                        break;       
                }
        }

        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}
