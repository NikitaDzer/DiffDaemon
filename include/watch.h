#ifndef WATCH_H
#define WATCH_H


const char *find_watch_path( const int wd);
void free_watch_bank( void);

int   watch_dir( const char *const path);
int unwatch_dir( const char *const path);

int watch_file( const char *const path);
int  show_file( const char *const path);
int  hide_file( const char *const path);

int  get_ifd( void);
int free_ifd( void);

#endif // WATCH_H
