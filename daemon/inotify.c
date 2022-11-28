#include <stdalign.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/backup.h"
#include "../include/config.h"
#include "../include/watch.h"
#include "../include/event.h"
#include "../include/error.h"
#include "../include/poll.h"
#include "../include/vfs.h"


static int use_dir( const struct dirent *const entry,
                    const char *const base_path)
{
        static char dir_path[MAX_PATH_SZ] = { 0 };

        memset( dir_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( dir_path, base_path);

        return watch_dir( dir_path);
}

static int use_reg( const struct dirent *const entry,
                    const char *const base_path)
{
        static char reg_path[MAX_PATH_SZ] = { 0 };

        memset( reg_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( reg_path, base_path);
        strcat( reg_path, entry->d_name);

        int check_res = check_text( reg_path);
        if ( is_sys_err( check_res) )
        {
                return ret_sys_err();
        }

        if ( check_res )
        {
                const int res = add_backup( reg_path);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        return 0;
}


int set_inotify_root( const char *const dir_path)
{
        static char root[MAX_PATH_SZ] = { 0 };

        int res = 0;

        if ( root[0] != 0 )
        {
                res = unwatch_dir( root);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        res = visit_dir( dir_path, "", use_dir, use_reg);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        strcpy( root, dir_path);

        return 0;
}

int poll_inotify( void)
{
        alignas( alignof( struct inotify_event)) static char buf[4096] = { 0 };

        int res = 0;

        while ( true )
        {
                const ssize_t nbytes = read( get_ifd(), buf, sizeof( buf));
                if ( nbytes == -1 && errno != EAGAIN )
                {
                        perror( "read");
                        return ret_sys_err();
                }

                if ( nbytes <= 0 )
                {
                        break;
                }

                const char                 *ptr   = buf;
                const struct inotify_event *event = NULL;
                const char                 *path  = NULL;

                for ( /* nothing */;
                      ptr < buf + nbytes;
                      ptr += sizeof( struct inotify_event) + event->len )
                {
                        event = (const struct inotify_event *)ptr;
                        path  = find_watch_path( event->wd);
                        
                        if ( path == NULL )
                        {
                                continue;
                        }

                        res = stash_event( path, event);
                        if ( is_sys_err( res) )
                        {
                                return ret_sys_err();
                        }
                }
        }

        res = commit_events();
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}
