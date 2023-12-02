#ifndef DAEMON_H
#define DAEMON_H


void dem_apply ( const char *const rel_path, const int rev_state);
void dem_diff  ( const char *const rel_path, const int rev_state);
void dem_chpid ( const char *const pid_str);
void dem_chtime( const double time);

int  dem_fork ( void);
int  dem_init ( void);
int  dem_free ( void);
void dem_close( void);
int  dem_run  ( const char *const initial_cwd);


#endif // DAEMON_H
