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
        .access_name = "cfile",
        .value_name = NULL,
        .description = "Copied file path"
    },
    {
        .identifier = 'D',
        .access_letters = "D",
        .access_name = "dfile",
        .value_name = NULL,
        .description = "Copied directory path"
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

struct option_info {
    const char identifier;
    const char *incompatible_flags;
};

static struct option_info flags_info[] = {
    {'C', "Ok"},
    {'O', "CdDe"},
    {'d', "D"}
};

const struct cag_option *getProgramOptions() {
    return options;
}

size_t getProgramOptionsSize() {
    return sizeof(options) / sizeof(options[0]);
}

char *tryHelpMessage() {
    return "Try 'redit --help' for more information.";
}

bool checkProgramFlags(bool copy_mode, bool overwrite_mode, bool copied_file_path, bool copied_dir_path,
                       bool e_included, bool keep_copy) {
    if (!(copy_mode || overwrite_mode)) {
        fprintf(stderr, "Error: Must use either -C or -O.\n%s\n", tryHelpMessage());
        return false;
    }

    constexpr size_t NUMBER_FLAGS = 6;

    char active_flags[NUMBER_FLAGS + 1] = {0};
    size_t flag_index = 0;

    if (copy_mode) active_flags[flag_index++] = 'C';
    if (overwrite_mode) active_flags[flag_index++] = 'O';
    if (copied_file_path) active_flags[flag_index++] = 'd';
    if (copied_dir_path) active_flags[flag_index++] = 'D';
    if (e_included) active_flags[flag_index++] = 'e';
    if (keep_copy) active_flags[flag_index++] = 'k';
    active_flags[flag_index] = '\0';

    const size_t flags_info_size = sizeof(flags_info) / sizeof(flags_info[0]);

    for (size_t i = 0; i < flag_index; ++i) {
        for (size_t j = 0; j < flags_info_size; ++j) {
            if (active_flags[i] == flags_info[j].identifier) {
                const char *incompatible_flags = flags_info[j].incompatible_flags;

                char present_incompatible_flags[NUMBER_FLAGS] = {0};
                size_t inc_index = 0;
                for (size_t ic = 0; ic < strlen(incompatible_flags); ++ic) {
                    if (strchr(active_flags, incompatible_flags[ic])) {
                        present_incompatible_flags[inc_index++] = incompatible_flags[ic];
                    }
                }
                present_incompatible_flags[inc_index] = '\0';

                if (inc_index > 0) {
                    fprintf(stderr, "Error: Flag/s ");
                    for (size_t k = 0; k < strlen(present_incompatible_flags); ++k) {
                        fprintf(stderr, "'%c'%s", present_incompatible_flags[k],
                                (k < strlen(present_incompatible_flags) - 1) ? ", " : "");
                    }
                    fprintf(stderr, " are incompatible with flag '%c'.\n%s\n", active_flags[i], tryHelpMessage());
                    return false;
                }
            }
        }
    }
    return true;
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
    return SUCCESS;
}
