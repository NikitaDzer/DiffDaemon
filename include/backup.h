#ifndef BACKUP_H
#define BACKUP_H


int    add_backup( const char *const file_path);
int remove_backup( const char *const file_path);

int  add_sample( const char *const file_path);
int apply_state( const char *const file_path,
                 const int rev_state);

int write_diff( const char *const file_path,
                const int rev_state);


#endif // BACKUP_H
