#ifndef PATCH_H
#define PATCH_H


typedef enum ApplyMode
{
        APPLY_MODE_FORWARD = 0,
        APPLY_MODE_REVERT  = 1,
} ApplyMode;


static const char  DIFF_UTIL_NAME[] = "diff";
static const char APPLY_UTIL_NAME[] = "apply";

static const char APPLY_UTIL_FORWARD_OPT[] = "";
static const char APPLY_UTIL_REVERT_OPT[]  = "-R";


int diff ( const char *const file1_path, const char *const file2_path,
           const char *const dest_path);
int apply( const char *const file_path,  const char *const patch_path,
           const ApplyMode mode);


#endif // PATCH_H
