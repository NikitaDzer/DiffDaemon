#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "../include/signal.h"
#include "../include/error.h"
#include "../include/config.h"
#include "../include/signal.h"
#include "../include/shmem.h"


static const key_t SHM_KEY = 0xDED007;


int create_shm( void)
{
        const int shmid = shmget( SHM_KEY, sizeof( Action), 
                                  0666 | IPC_CREAT);
        if ( shmid == -1 )
        {
                perror( "shmget");
                return ret_sys_err();
        }

        return 0;
}

int free_shm( void)
{
        const int shmid = shmget( SHM_KEY, sizeof( Action), 0666);
        if ( shmid == -1 )
        {
                perror( "shmget");
                return ret_sys_err();
        }

        if ( shmctl( shmid, IPC_RMID, NULL) == -1 )
        {
                perror( "shmctl");
                return ret_sys_err();
        }

        return 0;
}

Action *get_shm_action( void)
{
        const int shmid = shmget( SHM_KEY, sizeof( Action), 0666);
        if ( shmid == -1 )
        {
                perror( "shmget");
                return NULL;
        }

        Action *const shm_action = shmat( shmid, NULL, 0);
        if ( shm_action == (void *)-1 )
        {
                perror( "shmat");
                return NULL;
        }

        return shm_action;
}

int free_shm_action( const Action *const action)
{
        if ( shmdt( action) == -1 )
        {
                perror( "shmdt");
                return ret_sys_err();
        }

        return 0;
}
