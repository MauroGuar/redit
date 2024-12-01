#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>

#include "file_utils.h"


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

uid_t getEffectiveUserId() {
    char *sudo_user = getenv("SUDO_USER");
    if (sudo_user) {
        struct passwd *pw = getpwnam(sudo_user);
        if (pw) { return pw->pw_uid; } else {
            perror("getpwnam");
            return -1;
        }
    } else { return getuid(); }
}

mode_t getFilePermissions(char *file_path) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    return file_stat.st_mode;
}
