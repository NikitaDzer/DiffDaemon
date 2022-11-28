#ifndef VFS_H
#define VFS_H


#include <dirent.h>


int check_dir ( const char *const dir_path);
int check_text( const char *const file_path);

int visit_dir( const char *const base_path,
               const char *const dir_name,
               int (*const use_dir)(const struct dirent *const entry,
                                    const char *const base_path),
               int (*const use_reg)(const struct dirent *const entry,
                                    const char *const base_path));


#endif // VFS_H
