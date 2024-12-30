#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <errno.h>
#include <pwd.h>
#include <regex.h>
#include <ctype.h>

#include "../include/error_handler.h"
#include "../include/paths_handler.h"
#include "../include/flags_handler.h"
#include "../include/file_operations.h"
#include "../include/file_utils.h"

int resolveAndValidatePaths(const int argc, char *argv[], const flag_state_t *flags,
                            char copy_file_path[PATH_MAX], char privileged_file_path[PATH_MAX]) {
    if (flags->copied_file_path || flags->copied_dir_path) {
        if (flags->param_index + 1 >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/copy/file /path/to/original/file\n%s\n", argv[0], tryHelpMessage());
            return ERROR_INVALID_ARGUMENT;
        }

        const int prv_path_result = getAbsolutePath(argv[flags->param_index + 1], privileged_file_path);
        if (prv_path_result != SUCCESS) {
            return printError(prv_path_result, "resolving privileged file path");
        }

        const int prv_valid_result = validatePath(privileged_file_path, true, false);
        if (prv_valid_result != SUCCESS) {
            return printError(prv_valid_result, "validating privileged file path");
        }

        const int cpy_path_result = getAbsolutePathFuture(argv[flags->param_index], copy_file_path);
        if (cpy_path_result != SUCCESS) {
            return printError(cpy_path_result, "resolving copy file path");
        }

        if (flags->copied_dir_path) {
            const char *file_base_name = basename(privileged_file_path);
            const int abs_file_path_result = getAbsFilePathFromDir(copy_file_path, file_base_name);
            if (abs_file_path_result != SUCCESS) {
                return printError(abs_file_path_result, "getting absolute file path from directory");
            }
        }

        const int validation_result = validateOrCreatePath(copy_file_path, true, false);
        if (validation_result != SUCCESS) {
            return printError(validation_result, "validating copy file path");
        }
    } else {
        if (flags->param_index >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/original/file\n%s\n", argv[0], tryHelpMessage());
            return ERROR_INVALID_ARGUMENT;
        }

        char cwd[PATH_MAX];
        const int cwd_result = getCurrentWorkingDirectory(cwd);
        if (cwd_result != SUCCESS) {
            return printError(cwd_result, "resolving current working directory");
        }

        const int prv_path_result = getAbsolutePath(argv[flags->param_index], privileged_file_path);
        if (prv_path_result != SUCCESS) {
            return printError(prv_path_result, "resolving privileged file path");
        }

        const int prv_validation_result = validatePath(privileged_file_path, true, false);
        if (prv_validation_result != SUCCESS) {
            return printError(prv_validation_result, "validating privileged file path");
        }

        const char *base_name = basename(privileged_file_path);
        snprintf(copy_file_path, strlen(cwd) + strlen(base_name) + 2, "%s/%s", cwd, base_name);

        const int validation_result = validateOrCreatePath(copy_file_path, false, true);
        if (validation_result != SUCCESS) {
            return printError(validation_result, "validating copy file path");
        }
    }

    return SUCCESS;
}

void normalizeSlashes(const char *input_path, char normalized_path[PATH_MAX]) {
    int input_index = 0, output_index = 0;

    while (input_path[input_index] != '\0') {
        normalized_path[output_index++] = input_path[input_index];

        if (input_path[input_index] == '/') {
            while (input_path[input_index + 1] == '/') {
                input_index++;
            }
        }
        input_index++;
    }

    normalized_path[output_index] = '\0';
}

int getAbsolutePath(const char *original_path, char resolved_path[PATH_MAX]) {
    if (realpath(original_path, resolved_path) == NULL) {
        return ERROR_RESOLVING_PATH;
    }
    return SUCCESS;
}

int getAbsolutePathFuture(const char *original_path, char resolved_path[PATH_MAX]) {
    char normalized_path[PATH_MAX];
    normalizeSlashes(original_path, normalized_path);

    if (normalized_path[0] == '.' && (normalized_path[1] == '\0' || normalized_path[1] == '/')) {
        if (realpath(".", resolved_path) == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        if (normalized_path[1] == '\0') {
            return SUCCESS;
        }
        snprintf(resolved_path + strlen(resolved_path), PATH_MAX - strlen(resolved_path), "/%s", normalized_path + 2);
        return SUCCESS;
    }

    if (isalpha(normalized_path[0])) {
        if (realpath(".", resolved_path) == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        snprintf(resolved_path + strlen(resolved_path), PATH_MAX - strlen(resolved_path), "/%s", normalized_path);
        return SUCCESS;
    }

    if (normalized_path[0] == '.' && normalized_path[1] == '.' && (
            normalized_path[2] == '\0' || normalized_path[2] == '/')) {
        int last_index = 0;
        for (int i = 0; !isalpha(normalized_path[i]) && normalized_path[i] != '\0'; ++i) {
            resolved_path[i] = normalized_path[i];
            last_index = i;
        }
        resolved_path[last_index + 1] = '\0';
        if (realpath(resolved_path, resolved_path) == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        if (isalpha(normalized_path[last_index + 1])) {
            snprintf(resolved_path + strlen(resolved_path), PATH_MAX - strlen(resolved_path), "%s",
                     normalized_path + last_index);
        }
        return SUCCESS;
    }

    if (normalized_path[0] == '~' && (normalized_path[1] == '/' || normalized_path[1] == '\0')) {
        const char *home = getenv("HOME");
        if (home == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        snprintf(resolved_path, PATH_MAX, "%s%s", home, normalized_path + 1);
        return SUCCESS;
    }

    if (normalized_path[0] == '~' && isalpha(normalized_path[1])) {
        char other_user[PATH_MAX];
        sscanf(normalized_path, "~%[^/]", other_user);
        const struct passwd *pw = getpwnam(other_user);
        if (pw == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        snprintf(resolved_path, PATH_MAX, "%s%s", pw->pw_dir, normalized_path + strlen(other_user) + 1);
        return SUCCESS;
    }

    if (normalized_path[0] == '$' && isalpha(normalized_path[1])) {
        char env_path[256];
        sscanf(normalized_path + 1, "%[^/]", env_path);
        const char *env_value = getenv(env_path);
        if (env_value == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        snprintf(resolved_path, PATH_MAX, "%s%s", env_value, normalized_path + strlen(env_path) + 1);
        return SUCCESS;
    }

    if (normalized_path[0] == '/' && isalpha(normalized_path[1])) {
        snprintf(resolved_path, PATH_MAX, "%s", normalized_path);
        return SUCCESS;
    }

    return ERROR_PATH_INVALID;
}


int getAbsFilePathFromDir(char path[PATH_MAX], const char *file_name) {
    if (strlen(path) + strlen(file_name) + 2 >= PATH_MAX) {
        return ERROR_PATH_TOO_LONG;
    }
    const int last_index = strlen(path) - 1;
    if (path[last_index] != '/') {
        path[last_index + 1] = '/';
        path[last_index + 2] = '\0';
    }
    snprintf(path + strlen(path), PATH_MAX - strlen(path), "%s", file_name);
    return SUCCESS;
}


int validatePath(const char path[PATH_MAX], const bool check_read, const bool check_write) {
    char path_copy[PATH_MAX];
    strcpy(path_copy, path);
    const char *path_dir = dirname(path_copy);

    if (check_read && access(path_dir, R_OK) == -1) {
        return ERROR_PERMISSION_DENIED;
    }
    if (check_write && access(path_dir, W_OK) == -1) {
        return ERROR_PERMISSION_DENIED;
    }

    return SUCCESS;
}

int createDirRecursively(const char *path) {
    char temp_path[PATH_MAX];
    strncpy(temp_path, path, sizeof(temp_path));
    const int last_index = strlen(temp_path);
    if (temp_path[last_index - 1] != '/') {
        if (strlen(temp_path) + 2 > PATH_MAX) {
            return ERROR_PATH_TOO_LONG;
        }
        temp_path[last_index] = '/';
        temp_path[last_index + 1] = '\0';
    }

    size_t mkdir_counter = 0;
    for (size_t pos = 1; temp_path[pos] != '\0'; ++pos) {
        if (temp_path[pos] == '/') {
            temp_path[pos] = '\0';
            if (mkdir(temp_path, 0755) == SUCCESS) {
                uid_t ef_uid;
                const int user_result = getEffectiveUserId(&ef_uid);
                if (user_result == ERROR_USER_NOT_FOUND) {
                    return ERROR_USER_NOT_FOUND;
                }

                const int chown_result = changeFileOwner(temp_path, ef_uid);
                if (chown_result == ERROR_PERMISSION_DENIED) {
                    return ERROR_PERMISSION_DENIED;
                }
                mkdir_counter++;
            }
            temp_path[pos] = '/';
        }
    }
    if (mkdir_counter == 0) {
        return ERROR_PATH_INVALID;
    }
    return SUCCESS;
}

int validateOrCreatePath(const char path[PATH_MAX], const bool check_read, const bool check_write) {
    struct stat path_stat;

    char path_copy[PATH_MAX];
    strcpy(path_copy, path);
    const char *path_dir = dirname(path_copy);

    if (stat(path_dir, &path_stat) == -1) {
        if (errno == ENOENT) {
            printf("The path '%s' does not exist. Do you want to create it? (y/n): ", path_copy);
            char response;
            while (scanf(" %c", &response) != 1 || (
                       response != 'y' && response != 'Y' && response != 'n' && response != 'N')) {
                printf("Invalid input. Please enter 'y' or 'n': ");
            }
            if (response == 'n' || response == 'N') {
                return USER_EXIT;
            }

            const int create_result = createDirRecursively(path_dir);
            switch (create_result) {
                case ERROR_PATH_TOO_LONG:
                    return ERROR_PATH_TOO_LONG;
                case ERROR_PATH_INVALID:
                    return ERROR_PATH_INVALID;
            }
        }

        uid_t effective_uid;
        const int uid_result = getEffectiveUserId(&effective_uid);
        if (uid_result == ERROR_USER_NOT_FOUND) {
            return ERROR_USER_NOT_FOUND;
        }

        return SUCCESS;
    }

    const int val_result = validatePath(path, check_read, check_write);
    if (val_result == ERROR_PERMISSION_DENIED) {
        return ERROR_PERMISSION_DENIED;
    }

    return SUCCESS;
}

