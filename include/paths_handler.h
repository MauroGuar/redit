/**
 * @file paths_handler.h
 * @brief This header file contains declarations for functions in paths_handler.c.
 *
 * This functions allow for resolving and validating paths based on command-line arguments,
 * converting relative paths to absolute paths, validating file paths for read/write permissions,
 * and creating paths if they do not exist.
 *
 * Functions:
 * - int resolveAndValidatePaths(int argc, char *argv[], const flag_state_t *flags,
 *                              char copy_file_path[PATH_MAX], char privileged_file_path[PATH_MAX]);
 * - int getAbsolutePath(const char *original_path, char resolved_path[]);
 * - int getAbsolutePathFuture(const char *original_path, char resolved_path[]);
 * - int getAbsFilePathFromDir(char path[PATH_MAX], const char *file_name);
 * - int validatePath(const char path[PATH_MAX], bool check_read, bool check_write);
 * - int validateOrCreatePath(const char path[], bool check_read, bool check_write);
 */
#ifndef PATHS_HANDLE_H
#define PATHS_HANDLE_H
#include "flags_handler.h"
#include <stdbool.h>

int resolveAndValidatePaths(int argc, char *argv[], const flag_state_t *flags,
                            char copy_file_path[PATH_MAX], char privileged_file_path[PATH_MAX]);

int getAbsolutePath(const char *original_path, char resolved_path[]);

int getAbsolutePathFuture(const char *original_path, char resolved_path[]);

int getAbsFilePathFromDir(char path[PATH_MAX], const char *file_name);

int validatePath(const char path[PATH_MAX], bool check_read, bool check_write);

int validateOrCreatePath(const char path[], bool check_read, bool check_write);

#endif
