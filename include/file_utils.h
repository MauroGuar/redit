#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/types.h>
#include <cargs.h>

const struct cag_option *getProgramOptions();

size_t getProgramOptionsSize();

int getCurrentWorkingDirectory(char cwd[]);

int getEffectiveUserId(uid_t *u_id);

int getFilePermissions(const char *file_path, mode_t *permissions);

int getFileOwner(const char *file_path, uid_t *owner);

#endif
