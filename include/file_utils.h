#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/types.h>
#include <cargs.h>

const struct cag_option *getProgramOptions();

size_t getProgramOptionsSize();

char *tryHelpMessage();

bool checkProgramFlags(bool copy_mode, bool overwrite_mode, bool copied_file_path, bool copied_dir_path,
                       bool e_included, bool keep_copy);

int getCurrentWorkingDirectory(char cwd[]);

int getEffectiveUserId(uid_t *u_id);

int getFilePermissions(const char *file_path, mode_t *permissions);

int getFileOwner(const char *file_path, uid_t *owner);

#endif
