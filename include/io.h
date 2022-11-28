#ifndef IO_H
#define IO_H


#include <stdbool.h>

#include "cmdline.h"


void io_close( void);
int  io_fork ( void);
int  io_init ( const ProgMode mode);
int  io_free ( const ProgMode mode);
int  io_run  ( const ProgMode mode, const int dem_pid);

bool is_dem_failed( void);


#endif // IO_H
