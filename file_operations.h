#ifndef FILE_ACTIONS_H
#define FILE_ACTIONS_H

#include <sys/types.h>

void copyFile(const char *src, const char *dest);
void makeFileAvailable(const char *file_path, uid_t user_uid);

#endif // FILE_ACTIONS_H
