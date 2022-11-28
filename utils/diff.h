#ifndef DIFF_H
#define DIFF_H


static const char DIFF_UTIL_NAME[]   = "diff";
static const char DIFF_UTIL_FORMAT[] = "-u";


int diff( const char *const file1_path, 
          const char *const file2_path,
          const char *const  dest_path);


#endif // DIFF_H
