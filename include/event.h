#ifndef EVENT_H
#define EVENT_H


#include <sys/inotify.h>


int stash_event( const char *const dir_path,
                 const struct inotify_event *const event);

int commit_events( void);

void free_event_bank( void);


#endif // EVENT_H
