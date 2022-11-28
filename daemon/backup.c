#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/backup.h"
#include "../include/config.h"
#include "../include/error.h"
#include "../utils/copy.h"
#include "../utils/apply.h"
#include "../utils/diff.h"
#include "../utils/hash.h"


typedef struct BackupDescr
{
        int size;
} BackupDescr;


static const char *latest_apply_path = NULL;


static hash_t hash_path( const char *const file_path)
{
        return qhash( file_path, strlen( file_path));
}

static const char *get_backup_path( const char *const file_path)
{
        static char backup_path[MAX_PATH_SZ] = { 0 };

        char tmp[MAX_PATH_SZ] = { 0 };
        sprintf( tmp, "%" HASH_SPECIFIER, hash_path( file_path));

        memset( backup_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( backup_path, BACKUP_DIR_PATH);
        strcat( backup_path, tmp);

        return backup_path;
}

static const char *get_sample_path( const char *const file_path,
                                    const int index)
{
        static char sample_path[MAX_PATH_SZ] = { 0 };
        
        char tmp[MAX_PATH_SZ] = { 0 };
        sprintf( tmp, "%d", index);

        memset( sample_path, 0, sizeof( char) * MAX_PATH_SZ);
        strcat( sample_path, get_backup_path( file_path));
        strcat( sample_path, tmp);

        return sample_path;
}

static int recover_state( const char *const file_path, const int state)
{
        const char *sample_path = get_sample_path( file_path, 0);
        
        int res = copy_file( TMPFILE_PATH, sample_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        for ( int i = 1; i <= state; i++ )
        {
                sample_path = get_sample_path( file_path, i);

                res = apply( TMPFILE_PATH, sample_path,
                             APPLY_MODE_FORWARD);
                if ( is_sys_err( res) )
                {
                        return res;
                }
        }

        return 0;
}

static int read_backup( BackupDescr *const descr,
                        const char *const backup_path)
{
        const int fd = open( backup_path, O_RDWR, 0666);
        if ( fd == -1 )
        {
                perror( "open");
                return ret_sys_err();
        }

        const int nbytes = read( fd, descr, sizeof( BackupDescr));
        if ( nbytes == -1 )
        {
                perror( "read");
                close( fd);
                return ret_sys_err();
        }

        if ( close( fd) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        return 0;
}

static int write_backup( const BackupDescr *const descr,
                         const char *const backup_path)
{
        const int fd = open( backup_path, O_CREAT | O_RDWR, 0666);
        if ( fd == -1 )
        {
                perror( "open");
                return ret_sys_err();
        }

        const int nbytes = write( fd, descr, sizeof( BackupDescr));
        if ( nbytes == -1 )
        {
                perror( "write");
                close( fd);
                return ret_sys_err();
        }

        if ( close( fd) == -1 )
        {
                perror( "close");
                return ret_sys_err();
        }

        return 0;
}

static bool has_backup( const char *const file_path)
{
        return access( get_backup_path( file_path), F_OK) == 0;
}

int add_backup( const char *const file_path)
{
        if ( has_backup( file_path) )
        {
                return 0;
        }

        const char *const sample_path = get_sample_path( file_path, 0);
        const char *const backup_path = get_backup_path( file_path);

        int res = copy_file( sample_path, file_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        BackupDescr descr = { 0 };
        descr.size = 1;

        res = write_backup( &descr, backup_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}

int remove_backup( const char *const file_path)
{
        if ( !has_backup( file_path) )
        {
                return 0;
        }

        BackupDescr descr = { 0 };

        const char *const backup_path = get_backup_path( file_path);
        const char       *sample_path = NULL;

        int res = read_backup( &descr, backup_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        for ( int i = 0; i < descr.size; i++ )
        {
                sample_path = get_sample_path( file_path, i);

                if ( remove( sample_path) == -1 )
                {
                        perror( "remove");
                        return ret_sys_err();
                }
        }

        if ( remove( backup_path) == -1 )
        {
                perror( "remove");
                return ret_sys_err();
        }

        return 0;
}

int add_sample( const char *const file_path)
{
        if ( !has_backup( file_path) )
        {
                return add_backup( file_path);
        }

        if ( latest_apply_path != NULL
             && strncmp( file_path, latest_apply_path, 
                         strlen( file_path)) == 0 )
        {
                latest_apply_path = NULL;
                return 0;
        }

        BackupDescr descr = { 0 };

        const char *const backup_path = get_backup_path( file_path);
        const char       *sample_path = NULL;

        int res = read_backup( &descr, backup_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }
        
        res = recover_state( file_path, descr.size - 1);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        sample_path = get_sample_path( file_path, descr.size);

        res = diff( file_path, TMPFILE_PATH, sample_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        descr.size++;

        res = write_backup( &descr, backup_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        return 0;
}

int apply_state( const char *const file_path, 
                 const int rev_state)
{
        BackupDescr descr = { 0 };

        const char *const backup_path = get_backup_path( file_path);

        int res = read_backup( &descr, backup_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        if ( !(rev_state < descr.size) )
        {
                return 1;
        }

        const int size  = descr.size;
        const int state = (size - 1) - rev_state;

        res = recover_state( file_path, state);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        res = copy_file( file_path, TMPFILE_PATH);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        res = add_sample( file_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        latest_apply_path = file_path;

        return 0;
}

int write_diff( const char *const file_path,
                const int rev_state)
{
        BackupDescr descr = { 0 };

        const char *const backup_path = get_backup_path( file_path);
        const char       *sample_path = NULL;

        int res = read_backup( &descr, backup_path);
        if ( is_sys_err( res) )
        {
                return ret_sys_err();
        }

        if ( !(rev_state < descr.size) )
        {
                return 0;
        }

        FILE *const tmpfile = fopen( TMPFILE_PATH, "w");
        if ( tmpfile == NULL )
        {
                perror( "fopen");
                return ret_sys_err();
        }

        const int size  = descr.size;
        const int start = size - rev_state;
        
        FILE *sample_file = NULL;

        for ( int i = start; i < size; i++ )
        {
                sample_path = get_sample_path( file_path, i);

                sample_file = fopen( sample_path, "r");
                if ( sample_file == NULL )
                {       
                        perror( "fopen");
                        fclose( sample_file);
                        fclose( tmpfile);

                        return ret_sys_err();
                }

                while ( !feof( sample_file) )
                {
                        char tmpbuf[MAX_PATH_SZ] = { 0 };
                        const int nbytes = fread( tmpbuf, sizeof( char),
                                                  MAX_PATH_SZ, sample_file);
                        if ( nbytes == -1 )
                        {
                                perror( "fread");
                                fclose( sample_file);
                                fclose( tmpfile);
                                
                                return ret_sys_err();
                        }

                        if ( fwrite( tmpbuf, sizeof( char), 
                                     nbytes, tmpfile) == -1 )
                        {
                                perror( "fwrite");
                                fclose( sample_file);
                                fclose( tmpfile);

                                return ret_sys_err();
                        }
                }

                if ( fclose( sample_file) == EOF )
                {
                        perror( "fclose");
                        return ret_sys_err();

                }
        }

        if ( fclose( tmpfile) == EOF )
        {
                perror( "fclose");
                return ret_sys_err();
        }

        return 0;
}
