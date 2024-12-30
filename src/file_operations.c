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


int copyFile(const char *src, const char *dest) {
    if (strcmp(src, dest) == 0) {
        return ERROR_SAME_SOURCE;
    }

    struct stat src_stat;
    if (stat(src, &src_stat) == -1) {
        return errno == EACCES ? ERROR_PERMISSION_DENIED : ERROR_FILE_NOT_FOUND;
    }
    if (!S_ISREG(src_stat.st_mode)) {
        return ERROR_INVALID_SOURCE;
    }

    size_t buf_size = 4096;
    struct statvfs fs_stat;
    if (statvfs(src, &fs_stat) == 0) {
        buf_size = fs_stat.f_bsize;
    }

    if (src_stat.st_size > 64 * 1024) {
        buf_size = buf_size > 64 * 1024 ? buf_size : 64 * 1024;
    }

    if (buf_size > 128 * 1024) {
        buf_size = 128 * 1024;
    }

    const int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        return errno == EACCES ? ERROR_PERMISSION_DENIED : ERROR_FILE_NOT_FOUND;
    }

    const int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd == -1) {
        close(src_fd);
        return errno == EACCES ? ERROR_PERMISSION_DENIED : ERROR_COPY_FAILED;
    }

    u_int8_t *buffer = malloc(buf_size);
    if (!buffer) {
        close(src_fd);
        close(dest_fd);
        return ERROR_MEMORY_ALLOCATION;
    }

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

    if (n_read == -1) {
        free(buffer);
        close(src_fd);
        close(dest_fd);
        return ERROR_COPY_FAILED;
    }

    free(buffer);
    close(src_fd);
    close(dest_fd);

    return SUCCESS;
}

int changeFileOwner(const char *file_path, const uid_t user_uid) {
    const gid_t group_id = -1;
    if (chown(file_path, user_uid, group_id) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int addFilePermissions(const char *file_path, const mode_t add_mode) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    const mode_t new_mode = file_stat.st_mode | add_mode;
    if (chmod(file_path, new_mode) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int overwriteFilePermissions(const char *file_path, const mode_t new_mode) {
    if (chmod(file_path, new_mode) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int executeEditorCommand(const char *editor, const char copy_file_path[PATH_MAX], const char *PROGRAM_DEFAULT_EDITOR) {
    char *ed;
    if (editor != NULL) {
        ed = strdup(editor);
        if (ed == NULL) {
            return ERROR_MEMORY_ALLOCATION;
        }
    } else {
        ed = getenv("REDIT_EDITOR");
        if (ed == NULL) { ed = (char *) PROGRAM_DEFAULT_EDITOR; }
    }

    uid_t user_id;
    const int uid_result = getEffectiveUserId(&user_id);
    if (uid_result == ERROR_USER_NOT_FOUND) {
        return ERROR_USER_NOT_FOUND;
    }
    char command[512];
    snprintf(command, sizeof(command), "sudo -u \\#%d %s %s", user_id, ed, copy_file_path);
    if (editor != NULL) {
        free(ed);
    }
    return system(command);
}
