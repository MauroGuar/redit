#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/file_utils.h"

#define DEFAULT_EDITOR "vim"

void copyFile(const char *src, const char *dest, const int BUF_SIZE) {
    int src_fd, dest_fd;
    u_int8_t buffer[BUF_SIZE];
    ssize_t n_read, n_written;

    if (strcmp(src, dest) == 0) {
        fprintf(stderr, "Source and destination paths cannot be the same\n");
        exit(EXIT_FAILURE);
    }


    src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd == -1) {
        perror("Failed to open destination file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    while ((n_read = read(src_fd, buffer, BUF_SIZE)) > 0) {
        n_written = write(dest_fd, buffer, n_read);
        if (n_written != n_read) {
            perror("Failed to write to destination file");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (n_read == -1) {
        perror("Failed to read from source file");
    }

    close(src_fd);
    close(dest_fd);
}

void changeFileOwner(const char *file_path, uid_t user_uid) {
    gid_t group_id = -1;
    if (chown(file_path, user_uid, group_id) == -1) {
        perror("chown");
        exit(EXIT_FAILURE);
    }
}

void addFilePermissions(const char *file_path, mode_t add_mode) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    mode_t new_mode = file_stat.st_mode | add_mode;
    if (chmod(file_path, new_mode) == -1) {
        perror("chmod");
        exit(EXIT_FAILURE);
    }
}

void overwriteFilePermissions(const char *file_path, mode_t new_mode) {
    if (chmod(file_path, new_mode) == -1) {
        perror("chmod");
        exit(EXIT_FAILURE);
    }
}

int executeEditorCommand(const char *editor, const char *copy_file_path) {
    char *ed;
    if (editor != NULL) {
        ed = strdup(editor);
        for (int i = 0; i < strlen(editor); ++i) {
            ed[i] = tolower(editor[i]);
        }
    } else {
        ed = DEFAULT_EDITOR;
    }

    const uid_t user_id = getEffectiveUserId();
    char command[512];
    snprintf(command, sizeof(command), "sudo -u \\#%d %s %s", user_id, ed, copy_file_path);

    if (editor != NULL) { free(ed); }
    return system(command);
}
