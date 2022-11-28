#ifndef ACTION_H
#define ACTION_H


#include "config.h"


typedef enum ActionType
{
        NO_ACTION     = 0,
        ACTION_CLOSE  = 1,
        ACTION_APPLY  = 2,
        ACTION_DIFF   = 3,
        ACTION_CHPID  = 4,
        ACTION_CHTIME = 5,
} ActionType;

typedef union ActionArg
{
        char str[MAX_PATH_SZ];
        double dnum;
        int num;
} ActionArg;

typedef struct Action
{
        ActionType type;
        ActionArg  arg1;
        ActionArg  arg2;
} Action;


#endif // ACTION_H
