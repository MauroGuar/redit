#ifndef FILE_ACTIONS_H
#define FILE_ACTIONS_H

#include <sys/types.h>

int copyFile(const char *src, const char *dest);

int changeFileOwner(const char *file_path, uid_t user_uid);

int addFilePermissions(const char *file_path, mode_t add_mode);

int overwriteFilePermissions(const char *file_path, mode_t new_mode);

int executeEditorCommand(const char *editor, const char copy_file_path[], const char *PROGRAM_DEFAULT_EDITOR);

#endif
