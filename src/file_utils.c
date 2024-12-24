#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <libgen.h>
#include <cargs.h>

#include "../include/error_codes.h"
#include "../include/file_utils.h"

static struct cag_option options[] = {
    {
        .identifier = 'C',
        .access_letters = "Cc",
        .access_name = "copy",
        .value_name = NULL,
        .description = "Copy Mode"
    },
    {
        .identifier = 'O',
        .access_letters = "Oo",
        .access_name = "overwrite",
        .value_name = NULL,
        .description = "Overwrite Mode"
    },
    {
        .identifier = 'd',
        .access_letters = "d",
        .access_name = "dir",
        .value_name = NULL,
        .description = "Copied file path"
    },
    {
        .identifier = 'e',
        .access_letters = "e",
        .access_name = "editor",
        .value_name = "EDITOR",
        .description = "Editor to use"
    },
    {
        .identifier = 'k',
        .access_letters = "k",
        .access_name = "keep",
        .value_name = NULL,
        .description = "Keep copy"
    },
    {
        .identifier = 'h',
        .access_letters = "h",
        .access_name = "help",
        .description = "Shows the command help"
    }
};

const struct cag_option *getProgramOptions() {
    return options;
}

size_t getProgramOptionsSize() {
    return sizeof(options) / sizeof(options[0]);
}

int getCurrentWorkingDirectory(char **cwd) {
    *cwd = (char *) malloc(PATH_MAX * sizeof(char));
    if (*cwd == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    if (getcwd(*cwd, PATH_MAX) == NULL) {
        free(*cwd);
        return ERROR_CWD;
    }

    return SUCCESS;
}

int getAbsolutePath(const char *original_path, char **resolved_path) {
    char tempResolvedPath[PATH_MAX];
    if (realpath(original_path, tempResolvedPath) == NULL) {
        return ERROR_RESOLVING_PATH;
    }

    *resolved_path = (char *) malloc(strlen(tempResolvedPath) + 1);
    if (*resolved_path == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    strcpy(*resolved_path, tempResolvedPath);
    return SUCCESS;
}

int getAbsolutePathFuture(const char *original_path, char **resolved_path) {
    char temp_resolved_path[PATH_MAX];
    char *path_copy = strdup(original_path);
    if (path_copy == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    char *dir_path = dirname(path_copy);

    if (realpath(dir_path, temp_resolved_path) == NULL) {
        free(path_copy);
        return ERROR_RESOLVING_PATH;
    }

    *resolved_path = (char *) malloc(strlen(temp_resolved_path) + strlen(basename((char *) original_path)) + 2);
    if (*resolved_path == NULL) {
        free(path_copy);
        return ERROR_MEMORY_ALLOCATION;
    }

    snprintf(*resolved_path, strlen(temp_resolved_path) + strlen(basename((char *) original_path)) + 2, "%s/%s",
             temp_resolved_path,
             basename((char *) original_path));

    free(path_copy);

    return SUCCESS;
}

int getEffectiveUserId(uid_t *u_id) {
    char *sudo_user = getenv("SUDO_USER");
    if (sudo_user) {
        struct passwd *pw = getpwnam(sudo_user);
        if (pw) {
            *u_id = pw->pw_uid;
            return SUCCESS;
        } else {
            return ERROR_USER_NOT_FOUND;
        }
    } else {
        *u_id = getuid();
        return SUCCESS;
    }
}

int getFilePermissions(const char *file_path, mode_t *permissions) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    *permissions = file_stat.st_mode;
    return SUCCESS;
}

int getFileOwner(const char *file_path, uid_t *owner) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    return file_stat.st_uid;
}
