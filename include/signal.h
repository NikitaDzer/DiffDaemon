#ifndef SIGNAL_H
#define SIGNAL_H


#include <unistd.h>

#include "action.h"


int init_dem_handler( void);
int init_io_handler ( void);

int create_shm( void);
int free_shm();

int signal_dem( const Action *const action, const pid_t pid);

int signal_dem_close( const pid_t pid);
int signal_io_close ( void);


#endif // SIGNAL_H
