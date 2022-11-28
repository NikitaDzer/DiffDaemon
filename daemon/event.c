#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/inotify.h>
#include <unistd.h>

#include "../include/backup.h"
#include "../include/config.h"
#include "../include/event.h"
#include "../include/watch.h"
#include "../include/error.h"


typedef struct EventEntry
{
        const char *path;
        uint32_t action;
        bool is_dir;
} EventEntry;

typedef struct EventBank
{
        EventEntry *entries;
        int size;
} EventBank;


static EventBank event_bank = { 0 };


static const char *get_dir_path( const char *const base_path,
                                 const char *const dir_name)
{
        static char dir_path[MAX_PATH_SZ] = { 0 };

        memset( dir_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( dir_path, base_path);
        strcat( dir_path, dir_name);
        strcat( dir_path, "/");

        return dir_path;
}

static const char *get_file_path( const char *const base_path,
                                  const char *const file_name)
{
        static char file_path[MAX_PATH_SZ] = { 0 };

        memset( file_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( file_path, base_path);
        strcat( file_path, file_name);

        return file_path;
}

static EventEntry *find_entry( const char *const path)
{
        for ( int i = 0; i < event_bank.size; i++ )
        {
                if ( strcmp( event_bank.entries[i].path, path) == 0 )
                {
                        return event_bank.entries + i;
                }
        }

        return NULL;
}

static EventEntry *get_entry( const char *const path)
{
        EventEntry *entry = find_entry( path);

        if ( entry == NULL )
        {
                const int new_size = event_bank.size + 1;

                event_bank.entries = realloc( event_bank.entries,
                                              sizeof( EventEntry) * new_size);
                if ( errno == ENOMEM )
                {
                        perror( "realloc");
                        return NULL;
                }

                entry = event_bank.entries + new_size - 1;
                entry->action = IN_ACCESS;
                entry->is_dir = false;
                entry->path   = strdup( path);

                if ( entry->path == NULL )
                {
                        perror( "strdup");
                        return NULL;
                }
                
                event_bank.size++;
        }

        return entry;
}

static int set_entry( const char *const path, 
                      const uint32_t action,
                      const bool is_dir)
{
        EventEntry *const entry = get_entry( path);
        if ( entry == NULL )
        {
                return ret_sys_err();
        }

        entry->action = action;
        entry->is_dir = is_dir;

        return 0;
}

static int stash_event_dir( const char *const dir_path,
                            const struct inotify_event *const event)
{
        const char *const origin_path = get_dir_path( dir_path,
                                                      event->name);

        uint32_t action = IN_ACCESS;
        
        if ( event->mask & IN_CREATE )
        {
                action = IN_CREATE;
        }
        else if ( event->mask & IN_DELETE )
        {
                action = IN_DELETE;
        }
        else if ( event->mask & IN_DELETE_SELF )
        {
                action = IN_DELETE_SELF;
        }

        if ( action != IN_ACCESS )
        {
                const int res = set_entry( origin_path, action, true);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        return 0;
}

static int stash_event_file( const char *const dir_path,
                             const struct inotify_event *const event)
{
        const char *const origin_path = get_file_path( dir_path,
                                                       event->name);

        uint32_t action = IN_ACCESS;

        if ( event->mask & IN_CREATE )
        {
                action = IN_CREATE;        
        }
        else if ( event->mask & IN_DELETE )
        {
                action = IN_DELETE;
        }
        else if ( event->mask & IN_MODIFY )
        {
                action = IN_MODIFY;
        }

        if ( action != IN_ACCESS )
        {
                const int res = set_entry( origin_path, action, false);
                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        return 0;
}

static int commit_event_dir( EventEntry *const entry)
{
        const uint32_t action = entry->action;
        int res = 0;

        if ( action == IN_CREATE )
        {
                res = watch_dir( entry->path);
        }
        else if ( action == IN_DELETE || action == IN_DELETE_SELF )
        {
                res = unwatch_dir( entry->path);
        }

        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        entry->action = IN_ACCESS;

        return 0;
}

static int commit_event_file( EventEntry *const entry)
{
        const uint32_t action = entry->action;
        int res = 0;

        if ( action == IN_CREATE )
        {
                res = add_backup( entry->path);
        }
        else if ( action == IN_DELETE )
        {
                res = remove_backup( entry->path);
        }
        else if ( action == IN_MODIFY )
        {
                res = add_sample( entry->path);
        }

        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        entry->action = IN_ACCESS;

        return 0;
}

int stash_event( const char *const dir_path,
                 const struct inotify_event *const event )
{
        int res = 0;

        if ( event->mask & IN_ISDIR )
        {
                res = stash_event_dir( dir_path, event);
        }
        else
        {
                res = stash_event_file( dir_path, event);
        }

        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}

int commit_events( void)
{
        const int         size    = event_bank.size;
        EventEntry *const entries = event_bank.entries;

        for ( int i = 0; i < size; i++ )
        {
                EventEntry *const entry = entries + i;
                int res = 0;

                if ( entry->is_dir )
                {
                        res = commit_event_dir( entry);
                }
                else
                {
                        res = commit_event_file( entry);
                }

                if ( is_sys_err( res) )
                {
                        return ret_sys_err();
                }
        }

        return 0;
}

void free_event_bank( void)
{
        const int size = event_bank.size;

        for ( int i = 0; i < size; i++ )
        {
                free( (void *)event_bank.entries[i].path);
        }

        free( event_bank.entries);
}
