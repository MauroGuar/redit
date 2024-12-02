#include <ctype.h>
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

void copyMode(const char *copy_file_path, const char *privileged_file_path);

void overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy);

int main(int argc, char *argv[]) {
    int opt;
    bool copy_mode = false;
    bool overwrite_mode = false;

    bool copied_file_path = false;
    bool keep_copy = false;

    char *editor = "code";
    bool e_included = false;


    // TODO -e flag to choose editor
    while ((opt = getopt(argc, argv, "COde::k")) != -1) {
        switch (opt) {
            case 'C':
                copy_mode = true;
                break;
            case 'O':
                overwrite_mode = true;
                break;
            case 'd':
                copied_file_path = true;
                break;
            case 'e':
                e_included = true;
                break;
            case 'k':
                keep_copy = true;
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
    }

    char *copy_file_path;
    char *privileged_file_path;
    if (copied_file_path) {
        if (optind + 1 >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/copy/file /path/to/original/file\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        copy_file_path = getAbsolutePathFuture(argv[optind]);
        privileged_file_path = getAbsolutePath(argv[optind + 1]);
    } else {
        if (optind >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/original/file\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        char *cwd = getCurrentWorkingDirectory();
        privileged_file_path = getAbsolutePath(argv[optind]);
        char *base_name = basename(privileged_file_path);
        copy_file_path = malloc(strlen(cwd) + strlen(base_name) + 2);
        if (copy_file_path == NULL) {
            perror("malloc");
            free(cwd);
            exit(EXIT_FAILURE);
        }

        sprintf(copy_file_path, "%s/%s", cwd, base_name);
    }

    if (copy_mode) {
        copyMode(copy_file_path, privileged_file_path);
        if (!e_included) {
            printf("%s\n", copy_file_path);
        } /*else {
            for (int i = 0; i < strlen(editor); ++i) {
                editor[i] = tolower(editor[i]);
            }
            char command[1024];
            snprintf(command, sizeof(command), "code %s", editor);
            system(command);
        }*/
        return 0;
    } else if (overwrite_mode) {
        overwriteMode(copy_file_path, privileged_file_path, keep_copy);
        return 0;
    }

    free(copy_file_path);
    free(privileged_file_path);
}

void copyMode(const char *copy_file_path, const char *privileged_file_path) {
    const uid_t USER_EF_ID = getEffectiveUserId();

    copyFile(privileged_file_path, copy_file_path, COPY_BUF_SIZE);
    changeFileOwner(copy_file_path, USER_EF_ID);
    mode_t new_perms = S_IRUSR | S_IWUSR;
    addFilePermissions(copy_file_path, new_perms);
}

void overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy) {
    uid_t prv_file_owner = getFileOwner(privileged_file_path);
    mode_t prv_file_perms = getFilePermissions(privileged_file_path);

    copyFile(copy_file_path, privileged_file_path, COPY_BUF_SIZE);
    changeFileOwner(privileged_file_path, prv_file_owner);
    overwriteFilePermissions(privileged_file_path, prv_file_perms);

    if (!keep_copy) {
        if (remove(copy_file_path) == -1) { perror("Failed to remove copy file."); }
    }
}
