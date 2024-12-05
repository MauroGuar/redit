#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/types.h>


char *getCurrentWorkingDirectory();

char *getAbsolutePath(const char *path);

char *getAbsolutePathFuture(const char *path);

uid_t getEffectiveUserId();

mode_t getFilePermissions(const char *file_path);

uid_t getFileOwner(const char *file_path);

#endif
