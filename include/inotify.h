#ifndef INOTIFY_H
#define INOTIFY_H


int set_inotify_root( const char *const dir_path);
int poll_inotify( void);


#endif // INOTIFY_H
