#ifndef SEM_H
#define SEM_H


int create_sem( void);
int   free_sem( void);

int wait_sem( void);
int post_sem( void);


#endif // SEM_H
