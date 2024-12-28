#ifndef PATHS_HANDLE_H
#define PATHS_HANDLE_H

int getAbsolutePath(const char *original_path, char resolved_path[]);

int getAbsolutePathFuture(const char *original_path, char resolved_path[]);

int getAbsFilePathFromDir(char path[PATH_MAX], const char *file_name);

int validatePath(const char path[PATH_MAX], const bool check_read, const bool check_write);

int validateOrCreatePath(const char path[], bool check_read, bool check_write);

#endif
