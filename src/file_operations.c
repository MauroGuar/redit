#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "../include/error_handler.h"
#include "../include/file_utils.h"

/**
 * @file file_operations.c
 * @brief Implements file operations for the `redit` program.
 *
 * This file provides functionality for copying files, changing file ownership,
 * modifying file permissions, and executing editor commands. It ensures secure
 * handling of files and integrates error handling to manage potential issues
 * during operations.
 */

/**
 * @brief Copies a file from source to destination.
 *
 * @param src Path to the source file.
 * @param dest Path to the destination file.
 * @return `SUCCESS` if the file is copied successfully, or an error code otherwise.
 *
 * @details
 * - Validates that the source and destination are not the same.
 * - Adjusts buffer size dynamically based on file system and file size.
 * - Ensures the source file exists and is a regular file.
 * - Handles errors during file opening, reading, writing, or memory allocation.
 */
int copyFile(const char *src, const char *dest) {
    if (strcmp(src, dest) == 0) {
        return ERROR_SAME_SOURCE; // Prevent copying a file onto itself
    }

    struct stat src_stat;
    if (stat(src, &src_stat) == -1) {
        return errno == EACCES ? ERROR_PERMISSION_DENIED : ERROR_FILE_NOT_FOUND;
    }
    if (!S_ISREG(src_stat.st_mode)) {
        return ERROR_INVALID_SOURCE; // Source must be a regular file
    }

    // Dynamically adjust buffer size
    size_t buf_size = 4096; // Default buffer size
    struct statvfs fs_stat;
    if (statvfs(src, &fs_stat) == 0) {
        buf_size = fs_stat.f_bsize;
    }
    // Adjust buffer size based on file size
    if (src_stat.st_size > 64 * 1024) {
        buf_size = buf_size > 64 * 1024 ? buf_size : 64 * 1024;
    }
    // Limit buffer size to 128 KB
    if (buf_size > 128 * 1024) {
        buf_size = 128 * 1024;
    }

    // Open source file
    const int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        return errno == EACCES ? ERROR_PERMISSION_DENIED : ERROR_FILE_NOT_FOUND;
    }

    // Open destination file
    const int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd == -1) {
        close(src_fd);
        return errno == EACCES ? ERROR_PERMISSION_DENIED : ERROR_COPY_FAILED;
    }

    // Allocate buffer for file copying
    u_int8_t *buffer = malloc(buf_size);
    if (!buffer) {
        close(src_fd);
        close(dest_fd);
        return ERROR_MEMORY_ALLOCATION;
    }

    // Copy content from source to destination
    ssize_t n_read;
    while ((n_read = read(src_fd, buffer, buf_size)) > 0) {
        ssize_t n_written = 0;
        while (n_written < n_read) {
            const ssize_t result = write(dest_fd, buffer + n_written, n_read - n_written);
            if (result == -1) {
                free(buffer);
                close(src_fd);
                close(dest_fd);
                return ERROR_COPY_FAILED;
            }
            n_written += result;
        }
    }

    // Check for read errors or incomplete copy
    if (n_read == -1) {
        free(buffer);
        close(src_fd);
        close(dest_fd);
        return ERROR_COPY_FAILED;
    }

    // Clean up
    free(buffer);
    close(src_fd);
    close(dest_fd);

    return SUCCESS; // File copied successfully
}

/**
 * @brief Changes the ownership of a file.
 *
 * @param file_path Path to the file whose ownership will be changed.
 * @param user_uid User ID of the new owner.
 * @return `SUCCESS` if ownership is changed successfully, or an error code otherwise.
 *
 * @details
 * - Uses `chown` to change the file owner to the specified user.
 * - Group ownership remains unchanged.
 */
int changeFileOwner(const char *file_path, const uid_t user_uid) {
    const gid_t group_id = -1; // Keep the group ownership unchanged
    if (chown(file_path, user_uid, group_id) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

/**
 * @brief Adds permissions to a file.
 *
 * @param file_path Path to the file whose permissions will be modified.
 * @param add_mode Permission bits to add.
 * @return `SUCCESS` if permissions are updated successfully, or an error code otherwise.
 *
 * @details
 * - Fetches the current permissions using `stat` and adds the specified bits.
 * - Ensures the file exists and permissions can be modified.
 */
int addFilePermissions(const char *file_path, const mode_t add_mode) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    // Add the new specified permission bits
    const mode_t new_mode = file_stat.st_mode | add_mode;
    if (chmod(file_path, new_mode) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

/**
 * @brief Overwrites the permissions of a file.
 *
 * @param file_path Path to the file whose permissions will be overwritten.
 * @param new_mode New permission bits to set.
 * @return `SUCCESS` if permissions are updated successfully, or an error code otherwise.
 *
 * @details
 * - Directly applies the specified permission bits using `chmod`.
 */
int overwriteFilePermissions(const char *file_path, const mode_t new_mode) {
    if (chmod(file_path, new_mode) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

/**
 * @brief Executes a specified editor command on a file.
 *
 * @param editor The editor to use (e.g., "vim", "nano"). If NULL, a default editor is used.
 * @param copy_file_path Path to the file to be opened in the editor.
 * @param PROGRAM_DEFAULT_EDITOR Default editor to use if none is specified.
 * @return `SUCCESS` if the command is executed successfully, or an error code otherwise.
 *
 * @details
 * - Uses the `system` call to run the editor command.
 * - Determines the effective user ID for executing the editor under the appropriate user.
 * - Allows environment variable `REDIT_EDITOR` to override the default editor.
 */
int executeEditorCommand(const char *editor, const char copy_file_path[PATH_MAX], const char *PROGRAM_DEFAULT_EDITOR) {
    char *ed;
    if (editor != NULL) {
        ed = strdup(editor); // Duplicate the editor string
        if (ed == NULL) {
            return ERROR_MEMORY_ALLOCATION;
        }
    } else {
        ed = getenv("REDIT_EDITOR"); // Check environment variable
        if (ed == NULL) { ed = (char *) PROGRAM_DEFAULT_EDITOR; } // Assign default editor
    }

    // Get the effective user ID for the editor command
    uid_t user_id;
    const int uid_result = getEffectiveUserId(&user_id);
    if (uid_result == ERROR_USER_NOT_FOUND) {
        return ERROR_USER_NOT_FOUND;
    }

    // Construct and execute the command
    char command[512];
    snprintf(command, sizeof(command), "sudo -u \\#%d %s %s", user_id, ed, copy_file_path);

    if (editor != NULL) {
        free(ed); // Free the duplicated editor string
    }

    return system(command); // Execute the editor command and return the result
}
