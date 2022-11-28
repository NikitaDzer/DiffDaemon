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
#include "../include/io.h"
#include "../include/daemon.h"
#include "../include/signal.h"
#include "../include/shmem.h"
#include "../include/sem.h"


static void handle_io_signal( int sig, siginfo_t *si, void *ucontext)
{
        const ActionType action_type = (ActionType)si->si_value.sival_int;

        switch ( action_type )
        {
                case NO_ACTION:
                        return;

                case ACTION_CLOSE:
                        dem_close();
                        return;

                case ACTION_APPLY:
                case ACTION_DIFF:
                case ACTION_CHPID:
                case ACTION_CHTIME:
                default:
                        break;
        }

        const Action *const action = get_shm_action();
        if ( action == NULL )
        {
                signal_io_close();
                return;
        }

        int res = 0;

        switch ( action->type )
        {
                case ACTION_APPLY:
                        dem_apply( action->arg1.str, action->arg2.num);
                        break;

                case ACTION_DIFF:
                        dem_diff( action->arg1.str, action->arg2.num);
                        post_sem();
                        break;

                case ACTION_CHPID:
                        dem_chpid( action->arg1.str);
                        break;
                        
                case ACTION_CHTIME:
                        dem_chtime( action->arg1.dnum);
                        break;

                case NO_ACTION:
                case ACTION_CLOSE:
                default:
                        assert( 0 && "Unreachable.");
                        break;
        }

        if ( is_sys_err( res) )
        {
                free_shm_action( action);
                signal_io_close();
                return;
        }

        if ( is_sys_err( free_shm_action( action)) )
        {
                signal_io_close();
                return;
        }
}

static void handle_dem_signal( int sig, siginfo_t *si, void *ucontext)
{
        io_close();
}

int init_dem_handler( void)
{
        struct sigaction sa = { 0 };

        sigemptyset( &sa.sa_mask);
        sa.sa_sigaction = handle_io_signal;
        sa.sa_flags     = SA_SIGINFO;

        if ( sigaction( SIGUSR1, &sa, NULL) == -1 )
        {
                perror( "sigaction");
                return ret_sys_err();
        }

        return 0;
}

int init_io_handler( void)
{
        struct sigaction sa = { 0 };

        sigemptyset( &sa.sa_mask);
        sa.sa_sigaction = handle_dem_signal;
        sa.sa_flags     = SA_SIGINFO;

        if ( sigaction( SIGUSR1, &sa, NULL) == -1 )
        {
                perror( "sigaction");
                return ret_sys_err();
        }

        return 0;
}

int signal_dem( const Action *const action, const pid_t pid)
{
        Action *const shm_action = get_shm_action();
        if ( shm_action == NULL )
        {
                free_shm();
                return ret_sys_err();        
        }

        memcpy( shm_action, action, sizeof( Action));

        union sigval sv = { 0 };
        sv.sival_int = action->type;

        if ( sigqueue( pid, SIGUSR1, sv) == -1 )
        {
                perror( "sigqueue");
                free_shm_action( action);
                return ret_sys_err();
        }

        if ( is_sys_err( free_shm_action( shm_action)) )
        {
                return ret_sys_err();
        }

        return 0;
}

int signal_dem_close( const pid_t pid)
{
        union sigval sv = { 0 };
        sv.sival_int = ACTION_CLOSE;

        if ( sigqueue( pid, SIGUSR1, sv) == -1 )
        {
                perror( "sigqueue");
                return ret_sys_err();
        }

        return 0;
}

int signal_io_close( void)
{
        union sigval sv = { 0 };
        sv.sival_int = 0;

        if ( sigqueue( getppid(), SIGUSR1, sv) == -1 )
        {
                perror( "sigqueue");
                return ret_sys_err();
        }

        return 0;
}
