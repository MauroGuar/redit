#ifndef FILE_ACTIONS_H
#define FILE_ACTIONS_H

#include <sys/types.h>

void copyFile(const char *src, const char *dest, const int BUF_SIZE);

void changeFileOwner(const char *file_path, uid_t user_uid);

void addFilePermissions(const char *file_path, mode_t add_mode);

void overwriteFilePermissions(const char *file_path, mode_t new_mode);

#endif
