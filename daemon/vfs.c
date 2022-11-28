#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/config.h"
#include "../include/error.h"
#include "../include/vfs.h"


static bool is_reg_file( const struct dirent *const entry) 
{
        return entry->d_type == DT_REG;
}

static bool is_current_dir( const struct dirent *const entry)
{
        return entry->d_type == DT_DIR
               && strcmp( entry->d_name, ".") == 0;
}

static bool is_nested_dir( const struct dirent *const entry)
{
        return entry->d_type == DT_DIR           &&
               strcmp( entry->d_name, ".")  != 0 &&
               strcmp( entry->d_name, "..") != 0;
}

static const char *get_dir_path( const char *const base_path,
                                 const char *const dir_name)
{
        char *const dir_path = calloc( MAX_PATH_SZ, sizeof( char));
        if ( dir_path == NULL )
        {
                perror( "calloc");
                return NULL;
        }

        strcat( dir_path, base_path);
        strcat( dir_path, dir_name);
        strcat( dir_path, "/");

        return dir_path;
}


int check_dir( const char *const dir_path)
{
        struct stat sb = { 0 };

        if ( stat( dir_path, &sb) == -1 )
        {
                perror( "stat");
                return ret_sys_err();
        }

        return S_ISDIR( sb.st_mode);
}

int check_text( const char *const file_path)
{
        struct stat sb = { 0 };

        if ( stat( file_path, &sb) == -1 )
        {
                perror( "stat");
                return ret_sys_err();
        }

        if ( sb.st_mode & S_IXUSR ) 
        {
                return 0;
        }

        return 1;
}

int visit_dir( const char *const base_path, 
               const char *const dir_name,
               int (*const use_dir)(const struct dirent *const entry,
                                    const char *const base_path),
               int (*const use_reg)(const struct dirent *const entry,
                                    const char *const base_path))
{
        const char *dir_path = get_dir_path( base_path, dir_name);
        if ( dir_path == NULL )
        {
                return ret_sys_err();
        }

        DIR *const dir = opendir( dir_path);
        if ( dir == NULL )
        {
                free( (void *)dir_path);

                perror( "opendir");
                return ret_sys_err();
        }

        while ( true )
        {
                const struct dirent *const entry = readdir( dir);
                if ( entry == NULL )
                {
                        break;
                }

                int res = 0;

                if ( is_reg_file( entry) )
                {
                        res = use_reg( entry, dir_path);
                }
                else if ( is_current_dir( entry) )
                {
                        res = use_dir( entry, dir_path);
                }
                else if ( is_nested_dir( entry) )
                {
                        res = visit_dir( dir_path, entry->d_name,
                                         use_dir, use_reg);
                }

                if ( is_sys_err( res) )
                {
                        free( (void *)dir_path);
                        closedir( dir);
                        
                        return ret_sys_err();
                }

        }

        free( (void *)dir_path);
        
        if ( closedir( dir) == -1 )
        {
                perror( "closedir");
                return ret_sys_err();
        }
        
        return 0;
}
