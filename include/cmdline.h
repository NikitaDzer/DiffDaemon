#ifndef MODE_H
#define MODE_H


typedef enum ProgMode
{
        PROG_BAD_MODE  = 0,
        PROG_INT_MODE  = 1,
        PROG_DEM_MODE  = 2,
        PROG_HELP_MODE = 3,
} ProgMode;


static const char OPT_HELP[] = "-h";
static const char OPT_INT [] = "-i";
static const char OPT_DEM [] = "-d";
static const char OPT_PID [] = "-p";


ProgMode parse_cmdline( const int argc, const char *argv[],
                        const char **const pid_str);


#endif // MODE_H
