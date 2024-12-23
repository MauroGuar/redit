#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <cargs.h>
#include <linux/limits.h>

#include "../include/error_codes.h"
#include "../include/file_utils.h"
#include "../include/file_operations.h"

#define COPY_BUF_SIZE 4096

void copyMode(const char *copy_file_path, const char *privileged_file_path);

void overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy);

int main(int argc, char *argv[]) {
    bool copy_mode = false;
    bool overwrite_mode = false;
    bool copied_file_path = false;
    bool keep_copy = false;
    const char *EDITOR = NULL;
    bool e_included = false;

    const struct cag_option *program_options = getProgramOptions();
    size_t options_size = getProgramOptionsSize();
    int param_index;
    cag_option_context context;

    cag_option_init(&context, program_options, options_size, argc, argv);
    while (cag_option_fetch(&context)) {
        switch (cag_option_get_identifier(&context)) {
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
                if (cag_option_get_value(&context) != NULL) {
                    EDITOR = cag_option_get_value(&context);
                }
            case 'k':
                keep_copy = true;
                break;
            case '?':
                cag_option_print_error(&context, stdout);
                break;
        }
    }
    param_index = cag_option_get_index(&context);

    if (copy_mode && overwrite_mode) {
        fprintf(stderr, "Error: Cannot use -C and -O at the same time.\n");
        exit(EXIT_FAILURE);
    }

    char *copy_file_path;
    char *privileged_file_path;
    if (copied_file_path) {
        if (param_index + 1 >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/copy/file /path/to/original/file\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        // TODO handle prv & cpy result
        const int cpy_path_result = getAbsolutePathFuture(argv[param_index], &copy_file_path);
        const int prv_path_result = getAbsolutePath(argv[param_index + 1], &privileged_file_path);
    } else {
        if (param_index >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/original/file\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        char *cwd;
        // TODO handle cwd_result
        const int cwd_result = getCurrentWorkingDirectory(&cwd);
        // TODO handle prv result
        const int prv_path_result = getAbsolutePath(argv[param_index], &privileged_file_path);
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
        } else {
            const int result = executeEditorCommand(EDITOR, copy_file_path);
            if (result == 256) {
                free(copy_file_path);
                free(privileged_file_path);
                exit(EXIT_FAILURE);
            }
            if (result == -1) {
                fprintf(stderr, "Error executing the editor command.");
                free(copy_file_path);
                free(privileged_file_path);
                exit(EXIT_FAILURE);
            }
        }
    } else if (overwrite_mode) {
        overwriteMode(copy_file_path, privileged_file_path, keep_copy);
    }
    free(copy_file_path);
    free(privileged_file_path);
    return 0;
}

void copyMode(const char *copy_file_path, const char *privileged_file_path) {
    uid_t USER_EF_ID;
    // TODO handle uid_result
    const int uid_result = getEffectiveUserId(&USER_EF_ID);

    // TODO handle copy_result
    const int copy_result = copyFile(privileged_file_path, copy_file_path, COPY_BUF_SIZE);
    // TODO handle ch_own_result
    const int ch_own_result = changeFileOwner(copy_file_path, USER_EF_ID);
    mode_t new_perms = S_IRUSR | S_IWUSR;
    // TODO handle add_perms_result
    const int add_perms_result = addFilePermissions(copy_file_path, new_perms);
}

void overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy) {

    uid_t prv_file_owner;
    // TODO handle own_result
    const int own_result = getFileOwner(privileged_file_path, &prv_file_owner);

    mode_t prv_file_perms;
    // TODO handle perm_result
    const int perm_result = getFilePermissions(privileged_file_path, &prv_file_perms);

    // TODO handle copy_result
    const int copy_result = copyFile(copy_file_path, privileged_file_path, COPY_BUF_SIZE);
    // TODO handle ch_own_result
    const int ch_own_result = changeFileOwner(privileged_file_path, prv_file_owner);
    // TODO handle ovr_perms_result
    const int ovr_perms_result = overwriteFilePermissions(privileged_file_path, prv_file_perms);

    if (!keep_copy) {
        if (remove(copy_file_path) == -1) { perror("Failed to remove copy file."); }
    }
}
