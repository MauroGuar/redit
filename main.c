#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>

#include "file_utils.h"
#include "file_operations.h"

#define COPY_BUF_SIZE 4096

char *copyMode(const char *src_file);

void overwriteMode(const char *dest_file);

int main(int argc, char *argv[]) {
    int opt;
    bool copy_mode = false;
    bool overwrite_mode = false;

    while ((opt = getopt(argc, argv, "CO")) != -1) {
        switch (opt) {
            case 'C':
                copy_mode = true;
                break;
            case 'O':
                overwrite_mode = true;
                break;
            default:
                fprintf(stderr, "Usage: %s -C /path/to/file\n", argv[0]);
                fprintf(stderr, "       %s -O /path/to/edited/file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (copy_mode && overwrite_mode) {
        fprintf(stderr, "Error: Cannot use -C and -O at the same time.\n");
        exit(EXIT_FAILURE);
    } else if (copy_mode) {
        if (optind >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/file\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        const char *source_file = argv[optind];
        char *copied_file_path = copyMode(source_file);
        printf("%s\n", copied_file_path);
        fflush(stdout);
        free(copied_file_path);
        return 1;
    } else if (overwrite_mode) {
        if (optind >= argc) {
            fprintf(stderr, "Usage: %s -O /path/to/edited/file\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        const char *dest_file = argv[optind];
        overwriteMode(dest_file);
        return 1;
    }
}

char *copyMode(const char *src_file) {
    char *CWD = getCurrentWorkingDirectory();
    char *SRC_ABS_PATH = getAbsolutePath(src_file);
    const char *SRC_FILE_NAME = basename(SRC_ABS_PATH);
    char *DEST_ABS_PATH = malloc(strlen(CWD) + strlen(SRC_FILE_NAME) + 2);
    if (DEST_ABS_PATH == NULL) {
        perror("malloc");
        free(CWD);
        free(SRC_ABS_PATH);
        exit(EXIT_FAILURE);
    }
    snprintf(DEST_ABS_PATH, strlen(CWD) + strlen(SRC_FILE_NAME) + 2, "%s/%s", CWD, SRC_FILE_NAME);
    const uid_t USER_EF_ID = getEffectiveUserId();

    copyFile(SRC_ABS_PATH, DEST_ABS_PATH, COPY_BUF_SIZE);
    changeFileOwner(DEST_ABS_PATH, USER_EF_ID);
    mode_t new_perms = S_IRUSR | S_IWUSR;
    addFilePermissions(DEST_ABS_PATH, new_perms);

    free(CWD);
    free(SRC_ABS_PATH);
    return strdup(DEST_ABS_PATH);
}

void overwriteMode(const char *dest_file) {
    char *CWD = getCurrentWorkingDirectory();
    char *DEST_ABS_PATH = getAbsolutePath(dest_file);
    const char *DEST_FILE_NAME = basename(DEST_ABS_PATH);

    char *SRC_ABS_PATH = malloc(strlen(CWD) + strlen(DEST_FILE_NAME) + 2);
    if (SRC_ABS_PATH == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    snprintf(SRC_ABS_PATH, strlen(CWD) + strlen(DEST_FILE_NAME) + 2, "%s/%s", CWD, DEST_FILE_NAME);

    mode_t DEST_FILE_PERMS = getFilePermissions(dest_file);
    uid_t DEST_FILE_OWNER = getFileOwner(DEST_ABS_PATH);

    copyFile(SRC_ABS_PATH, DEST_ABS_PATH, COPY_BUF_SIZE);
    changeFileOwner(DEST_ABS_PATH, DEST_FILE_OWNER);
    overwriteFilePermissions(DEST_ABS_PATH, DEST_FILE_PERMS);

    if (remove(SRC_ABS_PATH) == -1) {
        perror("Failed to remove source file.");
    }

    free(CWD);
    free(DEST_ABS_PATH);
    free(SRC_ABS_PATH);
}
