#ifndef FILE_ACTIONS_H
#define FILE_ACTIONS_H

#include <sys/types.h>

int copyFile(const char *src, const char *dest, const int BUF_SIZE);

int changeFileOwner(const char *file_path, uid_t user_uid);

int addFilePermissions(const char *file_path, mode_t add_mode);

int overwriteFilePermissions(const char *file_path, mode_t new_mode);

int executeEditorCommand(const char *editor, char copy_file_path[]);

#endif
