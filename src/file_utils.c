#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <libgen.h>

char *getCurrentWorkingDirectory() {
    char *cwd = (char *) malloc(PATH_MAX * sizeof(char));
    if (cwd == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("Failed to get current working directory");
        free(cwd);
        exit(EXIT_FAILURE);
    }

    return cwd;
}

char *getAbsolutePath(const char *path) {
    char tempResolvedPath[PATH_MAX];
    char *resolvedPath;
    if (realpath(path, tempResolvedPath) == NULL) {
        perror("Failed to resolve absolute path");
        exit(EXIT_FAILURE);
    }
    resolvedPath = (char *) malloc(strlen(tempResolvedPath) + 1);
    if (resolvedPath == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    strcpy(resolvedPath, tempResolvedPath);
    return resolvedPath;
}

char *getAbsolutePathFuture(const char *path) {
    char tempResolvedPath[PATH_MAX];
    char *resolvedPath;
    char *path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    char *dir_path = dirname(path_copy);

    if (realpath(dir_path, tempResolvedPath) == NULL) {
        perror("Failed to resolve directory path");
        free(path_copy);
        exit(EXIT_FAILURE);
    }

    resolvedPath = malloc(strlen(tempResolvedPath) + strlen(basename((char *)path)) + 2);
    if (resolvedPath == NULL) {
        perror("Failed to allocate memory");
        free(path_copy);
        exit(EXIT_FAILURE);
    }

    snprintf(resolvedPath, strlen(tempResolvedPath) + strlen(basename((char *)path)) + 2, "%s/%s", tempResolvedPath, basename((char *)path));

    free(path_copy);
    return resolvedPath;
}


uid_t getEffectiveUserId() {
    char *sudo_user = getenv("SUDO_USER");
    if (sudo_user) {
        struct passwd *pw = getpwnam(sudo_user);
        if (pw) {
            return pw->pw_uid;
        } else {
            perror("getpwnam");
            return -1;
        }
    } else {
        return getuid();
    }
}

mode_t getFilePermissions(const char *file_path) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    return file_stat.st_mode;
}

uid_t getFileOwner(const char *file_path) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    return file_stat.st_uid;
}
