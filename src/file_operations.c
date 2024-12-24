#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/error_codes.h"
#include "../include/file_utils.h"

#define DEFAULT_EDITOR "vim"

int copyFile(const char *src, const char *dest, const int BUF_SIZE) {
    int src_fd, dest_fd;
    u_int8_t buffer[BUF_SIZE];
    ssize_t n_read, n_written;

    if (strcmp(src, dest) == 0) {
        return ERROR_SAME_SOURCE;
    }

    src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        if (errno == EACCES) {
            return ERROR_PERMISSION_DENIED;
        }
        return ERROR_FILE_NOT_FOUND;
    }

    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd == -1) {
        close(src_fd);
        if (errno == EACCES) {
            return ERROR_PERMISSION_DENIED;
        }
        return ERROR_FILE_NOT_FOUND;
    }

    while ((n_read = read(src_fd, buffer, BUF_SIZE)) > 0) {
        n_written = write(dest_fd, buffer, n_read);
        if (n_written != n_read) {
            close(src_fd);
            close(dest_fd);
            return ERROR_COPY_FAILED;
        }
    }

    if (n_read == -1) {
        close(src_fd);
        close(dest_fd);
        return ERROR_COPY_FAILED;
    }

    close(src_fd);
    close(dest_fd);
    return SUCCESS;
}

int changeFileOwner(const char *file_path, uid_t user_uid) {
    gid_t group_id = -1;
    if (chown(file_path, user_uid, group_id) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int addFilePermissions(const char *file_path, mode_t add_mode) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    mode_t new_mode = file_stat.st_mode | add_mode;
    if (chmod(file_path, new_mode) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int overwriteFilePermissions(const char *file_path, mode_t new_mode) {
    if (chmod(file_path, new_mode) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int executeEditorCommand(const char *editor, const char *copy_file_path) {
    char *ed;
    if (editor != NULL) {
        ed = strdup(editor);
        if (ed == NULL) {
            return ERROR_MEMORY_ALLOCATION;
        }
        for (int i = 0; i < strlen(editor); ++i) {
            ed[i] = tolower(editor[i]);
        }
    } else {
        ed = DEFAULT_EDITOR;
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
