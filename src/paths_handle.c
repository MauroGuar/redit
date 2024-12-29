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

#include "../include/error_codes.h"
#include "../include/paths_handle.h"

#include <ctype.h>
#include <threads.h>

#include "../include/file_operations.h"
#include "../include/file_utils.h"

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

            if (mkdir(path_dir, 0755) == -1 && errno != EEXIST) {
                return ERROR_PERMISSION_DENIED;
            }

            uid_t effective_uid;
            const int uid_result = getEffectiveUserId(&effective_uid);
            if (uid_result == ERROR_USER_NOT_FOUND) {
                return ERROR_USER_NOT_FOUND;
            }

            const int chown_result = changeFileOwner(path_dir, effective_uid);
            if (chown_result == ERROR_PERMISSION_DENIED) {
                return ERROR_PERMISSION_DENIED;
            }

            return SUCCESS;
        }
        return ERROR_PATH_INVALID;
    }

    return validatePath(path, check_read, check_write);
}
