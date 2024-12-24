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

int copyMode(const char *copy_file_path, const char *privileged_file_path);

int overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy);

char *tryHelpMessage();

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
        fprintf(stderr, "Error: Cannot use -C and -O at the same time.\n%s\n", tryHelpMessage());
        return ERROR_INVALID_ARGUMENT;
    }
    if (!(copy_mode || overwrite_mode)) {
        fprintf(stderr, "Error: Must use either -C or -O.\n%s\n", tryHelpMessage());
        return ERROR_INVALID_ARGUMENT;
    }

    char *copy_file_path;
    char *privileged_file_path;
    if (copied_file_path) {
        if (param_index + 1 >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/copy/file /path/to/original/file\n%s\n", argv[0], tryHelpMessage());
            return ERROR_INVALID_ARGUMENT;
        }

        const int cpy_path_result = getAbsolutePathFuture(argv[param_index], &copy_file_path);
        switch (cpy_path_result) {
            case ERROR_MEMORY_ALLOCATION:
                fprintf(stderr, "Error allocating memory for copy file path.\n");
                return ERROR_MEMORY_ALLOCATION;
            case ERROR_RESOLVING_PATH:
                fprintf(stderr, "Error resolving copy file path.\n");
                return ERROR_RESOLVING_PATH;
        }
        const int prv_path_result = getAbsolutePath(argv[param_index + 1], &privileged_file_path);
        switch (prv_path_result) {
            case ERROR_MEMORY_ALLOCATION:
                fprintf(stderr, "Error allocating memory for privileged file path.\n");
                return ERROR_MEMORY_ALLOCATION;
            case ERROR_RESOLVING_PATH:
                fprintf(stderr, "Error resolving privileged file path.\n");
                return ERROR_RESOLVING_PATH;
        }
    } else {
        if (param_index >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/original/file\n%s\n", argv[0], tryHelpMessage());
            return ERROR_INVALID_ARGUMENT;
        }

        char *cwd;
        const int cwd_result = getCurrentWorkingDirectory(&cwd);
        switch (cwd_result) {
            case ERROR_MEMORY_ALLOCATION:
                fprintf(stderr, "Error allocating memory for current working directory.\n");
                return ERROR_MEMORY_ALLOCATION;
            case ERROR_CWD:
                fprintf(stderr, "Error resolving current working directory.\n");
                return ERROR_CWD;
        }

        const int prv_path_result = getAbsolutePath(argv[param_index], &privileged_file_path);
        switch (prv_path_result) {
            case ERROR_MEMORY_ALLOCATION:
                fprintf(stderr, "Error allocating memory for privileged file path.\n");
                free(cwd);
                return ERROR_MEMORY_ALLOCATION;
            case ERROR_RESOLVING_PATH:
                fprintf(stderr, "Error resolving privileged file path.\n");
                free(cwd);
                return ERROR_RESOLVING_PATH;
        }

        char *base_name = basename(privileged_file_path);
        copy_file_path = malloc(strlen(cwd) + strlen(base_name) + 2);
        if (copy_file_path == NULL) {
            fprintf(stderr, "Error allocating memory for copy file path.\n");
            free(cwd);
            return ERROR_MEMORY_ALLOCATION;
        }

        sprintf(copy_file_path, "%s/%s", cwd, base_name);
        free(cwd);
    }

    if (copy_mode) {
        int cpy_mode_result = copyMode(copy_file_path, privileged_file_path);
        if (cpy_mode_result != SUCCESS) {
            free(copy_file_path);
            free(privileged_file_path);
            return cpy_mode_result;
        }

        if (!e_included) {
            printf("%s\n", copy_file_path);
        } else {
            const int editor_result = executeEditorCommand(EDITOR, copy_file_path);
            switch (editor_result) {
                case ERROR_USER_NOT_FOUND:
                    fprintf(stderr, "Error getting user id for the editor command.\n");
                    free(copy_file_path);
                    free(privileged_file_path);
                    return ERROR_USER_NOT_FOUND;
                case ERROR_MEMORY_ALLOCATION:
                    fprintf(stderr, "Error allocating memory for editor command.\nProceeding without the editor.\n");
                    printf("\n%s\n", copy_file_path);
                    break;
                case ERROR_COMMAND_NOT_FOUND:
                    fprintf(stderr, "Proceeding without the editor.\n");
                    printf("\n%s\n", copy_file_path);
                    break;
                case -1:
                    fprintf(stderr, "Proceeding without the editor.\n");
                    printf("\n%s\n", copy_file_path);
                    break;
            }
        }
    } else if (overwrite_mode) {
        int ovw_mode_result = overwriteMode(copy_file_path, privileged_file_path, keep_copy);
        if (ovw_mode_result != SUCCESS) {
            free(copy_file_path);
            free(privileged_file_path);
            return ovw_mode_result;
        }
    }
    free(copy_file_path);
    free(privileged_file_path);
    return SUCCESS;
}

int copyMode(const char *copy_file_path, const char *privileged_file_path) {
    uid_t USER_EF_ID;
    const int uid_result = getEffectiveUserId(&USER_EF_ID);
    if (uid_result == ERROR_USER_NOT_FOUND) {
        return ERROR_USER_NOT_FOUND;
    }

    const int copy_result = copyFile(privileged_file_path, copy_file_path, COPY_BUF_SIZE);
    switch (copy_result) {
        case ERROR_SAME_SOURCE:
            fprintf(stderr, "Error copying file: Source and destination are the same.\n");
            return ERROR_SAME_SOURCE;
        case ERROR_FILE_NOT_FOUND:
            fprintf(stderr, "Error copying file: File not found.\n");
            return ERROR_FILE_NOT_FOUND;
        case ERROR_PERMISSION_DENIED:
            fprintf(stderr, "Error copying file: Permission denied.\n");
            return ERROR_PERMISSION_DENIED;
    }

    const int ch_own_result = changeFileOwner(copy_file_path, USER_EF_ID);
    if (ch_own_result == ERROR_PERMISSION_DENIED) {
        fprintf(stderr, "Error changing file owner: Permission denied.\n");
        return ERROR_PERMISSION_DENIED;
    }

    mode_t new_perms = S_IRUSR | S_IWUSR;
    const int add_perms_result = addFilePermissions(copy_file_path, new_perms);
    switch (add_perms_result) {
        case ERROR_FILE_NOT_FOUND:
            fprintf(stderr, "Error adding file permissions: File not found.\n");
            return ERROR_FILE_NOT_FOUND;
        case ERROR_PERMISSION_DENIED:
            fprintf(stderr, "Error adding file permissions: Permission denied.\n");
            return ERROR_PERMISSION_DENIED;
    }

    return SUCCESS;
}

int overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy) {
    uid_t prv_file_owner;
    const int own_result = getFileOwner(privileged_file_path, &prv_file_owner);
    if (own_result == ERROR_FILE_NOT_FOUND) {
        fprintf(stderr, "Error getting file owner: File not found.\n");
        return ERROR_FILE_NOT_FOUND;
    }

    mode_t prv_file_perms;
    const int perm_result = getFilePermissions(privileged_file_path, &prv_file_perms);
    if (perm_result == ERROR_FILE_NOT_FOUND) {
        fprintf(stderr, "Error getting file permissions: File not found.\n");
        return ERROR_FILE_NOT_FOUND;
    }

    const int copy_result = copyFile(copy_file_path, privileged_file_path, COPY_BUF_SIZE);
    switch (copy_result) {
        case ERROR_SAME_SOURCE:
            fprintf(stderr, "Error copying file: Source and destination are the same.\n");
            return ERROR_SAME_SOURCE;
        case ERROR_FILE_NOT_FOUND:
            fprintf(stderr, "Error copying file: File not found.\n");
            return ERROR_FILE_NOT_FOUND;
        case ERROR_PERMISSION_DENIED:
            fprintf(stderr, "Error copying file: Permission denied.\n");
            return ERROR_PERMISSION_DENIED;
    }

    const int ch_own_result = changeFileOwner(privileged_file_path, prv_file_owner);
    if (ch_own_result == ERROR_PERMISSION_DENIED) {
        fprintf(stderr, "Error changing file owner: Permission denied.\n");
        return ERROR_PERMISSION_DENIED;
    }

    const int ovr_perms_result = overwriteFilePermissions(privileged_file_path, prv_file_perms);
    if (ovr_perms_result == ERROR_PERMISSION_DENIED) {
        fprintf(stderr, "Error overwriting file permissions: Permission denied.\n");
        return ERROR_PERMISSION_DENIED;
    }

    if (!keep_copy) {
        if (remove(copy_file_path) == -1) {
            perror("Failed to remove copy file.\n");
        }
    }

    return SUCCESS;
}

char *tryHelpMessage() {
    return "Try 'redit --help' for more information.";
}
