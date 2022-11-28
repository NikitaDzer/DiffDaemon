#ifndef APPLY_H
#define APPLY_H


typedef enum ApplyMode
{
        APPLY_MODE_FORWARD = 0,
        APPLY_MODE_REVERT  = 1,
} ApplyMode;


static const char APPLY_UTIL_NAME[]       = "patch";
static const char APPLY_UTIL_REVERT_OPT[] = "-R";


int apply( const char *const  file_path,
           const char *const patch_path,
           const ApplyMode mode);


#endif // APPLY_H
