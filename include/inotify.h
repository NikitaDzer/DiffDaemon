#ifndef POLL_H
#define POLL_H


int set_inotify_root( const char *const dir_path);
int poll_inotify( void);


#endif // POLL_H
