#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/inotify.h>
#include <unistd.h>

#include "include/watch.h"
#include "include/error.h"


typedef struct WatchEntry
{
        const char *path;
        int wd;
        bool active;
} WatchEntry;

typedef struct WatchBank
{
        WatchEntry *entries;
        int size;
        int iter;
} WatchBank;


static const uint32_t MASK_INOTIFY = IN_CREATE | IN_DELETE | IN_MODIFY;

static WatchBank watch_bank = { 0 };


static WatchEntry *iterate_watch_bank( void)
{
        if ( watch_bank.iter == watch_bank.size )
        {       
                watch_bank.iter = 0;
                return NULL;
        }
       
        for ( /* nothing */; 
              watch_bank.iter != watch_bank.size; 
              watch_bank.iter++ )
        {
                return watch_bank.entries + watch_bank.iter++;
        }

        watch_bank.iter = 0;

        return NULL;
}

static void start_iterate_watch_bank( void)
{
        watch_bank.iter = 0;
}

static WatchEntry *find_next_watch( const char *const path)
{
        for ( WatchEntry *entry = iterate_watch_bank();
              entry != NULL;
              entry = iterate_watch_bank() )
        {
                if ( strcmp( path, entry->path) == 0
                     && entry->active )
                {
                        return entry;
                }
        }

        return NULL;
}

static WatchEntry *find_exact_watch( const char *const path)
{
        start_iterate_watch_bank();

        for ( WatchEntry *entry = find_next_watch( path);
              entry != NULL;
              entry = find_next_watch( path) )
        {
                if ( strlen( path) == strlen( entry->path) )
                {
                        return entry;
                }
        }

        return NULL;
}

static int add_watch( const char *const path, const int wd)
{
        WatchEntry *const prev_entries = watch_bank.entries;
        const int         new_size     = watch_bank.size + 1;

        WatchEntry *new_entries = NULL;
        new_entries = realloc( prev_entries,
                               new_size * sizeof( WatchEntry));
        if ( errno == ENOMEM )
        {
                perror( "realloc");
                return ret_sys_err();
        }

        WatchEntry *const entry = new_entries + new_size - 1;

        entry->wd     = wd;
        entry->active = true;
        entry->path   = strdup( path);

        if ( entry->path == NULL )
        {
                perror( "strdup");
                return ret_sys_err();
        }

        watch_bank.entries = new_entries;
        watch_bank.size    = new_size;

        return 0;
}

static void remove_next_watch( const char *const path)
{
        WatchEntry *const entry = find_next_watch( path);

        if ( entry != NULL )
        {
                entry->active = false;
        }
}

static void remove_all_watch( const char *const path)
{
        start_iterate_watch_bank();

        for ( WatchEntry *entry = find_next_watch( path);
              entry != NULL;
              entry = find_next_watch( path) )
        {
                entry->active = false;
        }
}

const char *find_watch_path( const int wd)
{
        start_iterate_watch_bank();

        for ( WatchEntry *entry = iterate_watch_bank();
              entry != NULL;
              entry = iterate_watch_bank() )
        {
                if ( entry->wd == wd
                     && entry->active )
                {
                        return entry->path;
                }
        }

        return NULL;
}

int watch_dir( const char *const path)
{
        const int wd = inotify_add_watch( get_ifd(), path, MASK_INOTIFY);
        if ( wd == -1 )
        {
                perror( "inotify_add_watch");
                return ret_sys_err();
        }

        const int res = add_watch( path, wd);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}

int unwatch_dir( const char *const path)
{
        const int ifd = get_ifd();

        start_iterate_watch_bank();

        for ( WatchEntry *entry = find_next_watch( path);
              entry != NULL;
              entry = find_next_watch( path) )
        {
                const int res = inotify_rm_watch( ifd, 
                                                  find_next_watch( path)->wd);
                if ( res == -1 )
                {
                        perror( "inotify_rm_watch");
                        return ret_sys_err();
                }
        }

        remove_all_watch( path);

        return 0;
}

int watch_file( const char *const path)
{
        const int wd = inotify_add_watch( get_ifd(), path, MASK_INOTIFY);
        if ( wd == -1 )
        {
                perror( "inotify_add_watch");
                return ret_sys_err();
        }

        const int res = add_watch( path, wd);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}

int show_file( const char *const path)
{
        WatchEntry *const entry = find_exact_watch( path);

        if ( entry != NULL )
        {
                const int wd = inotify_add_watch( get_ifd(), path, 
                                                  MASK_INOTIFY);
                if ( wd == -1 )
                {
                        perror( "inotify_add_watch");
                        return ret_sys_err();
                }

                entry->wd = wd;
        }

        return 0;
}

int hide_file( const char *const path)
{
        const WatchEntry *const entry = find_exact_watch( path);
        
        if ( entry != NULL )
        {
                const int res = inotify_rm_watch( get_ifd(), entry->wd);
                if ( res == -1 )
                {
                        perror( "inotify_rm_watch");
                        return ret_sys_err();
                }
        }

        return 0;
}

int get_ifd( void)
{
        static bool is_init = false;
        static int  ifd = 0;

        if ( is_init )
        {
                return ifd;
        }

        ifd = inotify_init1( IN_NONBLOCK);
        if ( ifd == -1 )
        {
                perror( "inotify_init1");
                return ret_sys_err();
        }

        is_init = true;

        return ifd;
}

int free_ifd( void)
{
        if ( close( get_ifd()) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        return 0;
}

void free_watch_bank( void)
{
        const int               size    = watch_bank.size;
        const WatchEntry *const entries = watch_bank.entries;
        
        for ( int i = 0; i < size; i++ )
        {
                free( (void *)entries[i].path);
        }

        free( watch_bank.entries);
}
