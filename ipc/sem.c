#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>

#include "../include/sem.h"
#include "../include/error.h"


static const char SEM_NAME[] = "DD_SEM";

static sem_t *sem = NULL;


int create_sem( void)
{
        sem = sem_open( SEM_NAME, O_CREAT | O_RDWR, 
                        0644, 0);
        if ( sem == SEM_FAILED )
        {
                return ret_sys_err();        
        }

        return 0;
}

int free_sem( void)
{
        if ( sem_unlink( SEM_NAME) == -1 )
        {
                return ret_sys_err();
        }

        return 0;
}

int wait_sem( void)
{
        if ( sem_wait( sem) == -1 )
        {
                return ret_sys_err();
        }

        return 0;
}

int post_sem( void)
{
        if ( sem_post( sem) == -1 )
        {
                return ret_sys_err();
        }

        return 0;
}
