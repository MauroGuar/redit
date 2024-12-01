// file_utils.h
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/types.h>


char *getAbsolutePath(const char *path);

char *getCurrentWorkingDirectory();

uid_t getEffectiveUserId();

mode_t getFilePermissions(char *file_path);

#endif // FILE_UTILS_H
