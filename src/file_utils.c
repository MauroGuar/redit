#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <cargs.h>

#include "../include/error_handler.h"
#include "../include/file_utils.h"


char *tryHelpMessage() {
    return "Try 'redit --help' for more information.";
}

int getCurrentWorkingDirectory(char cwd[PATH_MAX]) {
    if (getcwd(cwd, PATH_MAX) == NULL) {
        return ERROR_CWD;
    }

    return SUCCESS;
}


int getEffectiveUserId(uid_t *u_id) {
    const char *sudo_user = getenv("SUDO_USER");
    if (sudo_user) {
        const struct passwd *pw = getpwnam(sudo_user);
        if (pw) {
            *u_id = pw->pw_uid;
            return SUCCESS;
        }
        return ERROR_USER_NOT_FOUND;
    }
    *u_id = getuid();
    return SUCCESS;
}

int getFilePermissions(const char *file_path, mode_t *permissions) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    *permissions = file_stat.st_mode;
    return SUCCESS;
}

int getFileOwner(const char *file_path, uid_t *file_owner) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        return ERROR_FILE_NOT_FOUND;
    }
    *file_owner = file_stat.st_uid;
    return SUCCESS;
}
