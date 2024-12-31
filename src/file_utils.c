#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <cargs.h>

#include "../include/error_handler.h"
#include "../include/file_utils.h"

/**
 * @file file_utils.c
 * @brief Provides utility functions for file and user management in the `redit` program.
 * 
 * This file implements helper functions to retrieve information about the current
 * working directory, user details, and file attributes such as permissions and ownership.
 * These utilities are essential for managing file operations with proper error handling.
 */

/**
 * @brief Returns a standard help message for the `redit` program.
 * 
 * @return A pointer to a static string containing the help message.
 */
char *tryHelpMessage() {
    return "Try 'redit --help' for more information.";
}

/**
 * @brief Retrieves the current working directory.
 * 
 * @param cwd A buffer to store the path of the current working directory. Must be at least PATH_MAX in size.
 * @return `SUCCESS` if the working directory is retrieved successfully, or an error code otherwise.
 * 
 * @details
 * - Uses the `getcwd` system call to obtain the current working directory.
 * - Handles errors such as insufficient buffer size or inaccessible directories.
 */
int getCurrentWorkingDirectory(char cwd[PATH_MAX]) {
    if (getcwd(cwd, PATH_MAX) == NULL) {
        return ERROR_CWD;  // Failed to retrieve current working directory
    }
    return SUCCESS;
}

/**
 * @brief Retrieves the effective user ID.
 * 
 * @param u_id Pointer to a variable where the effective user ID will be stored.
 * @return `SUCCESS` if the user ID is retrieved successfully, or an error code otherwise.
 * 
 * @details
 * - If the program is run with `sudo`, retrieves the user ID of the original user via the `SUDO_USER` environment variable.
 * - Defaults to the real user ID if `SUDO_USER` is not set.
 */
int getEffectiveUserId(uid_t *u_id) {
    const char *sudo_user = getenv("SUDO_USER");
    if (sudo_user) {
        const struct passwd *pw = getpwnam(sudo_user);
        if (pw) {
            *u_id = pw->pw_uid; // Original user ID when using sudo
            return SUCCESS;
        }
        return ERROR_USER_NOT_FOUND; // Failed to find user in system records
    }
    *u_id = getuid(); // Default to real user ID
    return SUCCESS;
}

/**
 * @brief Retrieves the permissions of a file.
 * 
 * @param file_path Path to the file whose permissions will be retrieved.
 * @param permissions Pointer to a variable where the permissions will be stored.
 * @return `SUCCESS` if permissions are retrieved successfully, or an error code otherwise.
 * 
 * @details
 * - Uses the `stat` system call to fetch file metadata.
 * - The permissions are stored as a bitmask in the `mode_t` format.
 */
int getFilePermissions(const char *file_path, mode_t *permissions) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;  // File does not exist or cannot be accessed
    }
    *permissions = file_stat.st_mode;  // Store file permissions
    return SUCCESS;
}

/**
 * @brief Retrieves the owner of a file.
 * 
 * @param file_path Path to the file whose ownership will be retrieved.
 * @param file_owner Pointer to a variable where the owner's user ID will be stored.
 * @return `SUCCESS` if ownership is retrieved successfully, or an error code otherwise.
 * 
 * @details
 * - Uses the `stat` system call to fetch file metadata.
 * - Extracts the owner's user ID from the metadata.
 */
int getFileOwner(const char *file_path, uid_t *file_owner) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;  // File does not exist or cannot be accessed
    }
    *file_owner = file_stat.st_uid;  // Store the owner's user ID
    return SUCCESS;
}
