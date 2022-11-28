#ifndef SHMEM_H
#define SHMEM_H


#include "action.h"


int create_shm( void);
int   free_shm( void);

Action *get_shm_action( void);
int    free_shm_action( const Action *const action);


#endif // SHMEM_H
