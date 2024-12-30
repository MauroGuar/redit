#include <stdio.h>
#include <stdbool.h>
#include <cargs.h>

#include "../include/flags_handler.h"

#include <string.h>

#include "../include/error_handler.h"
#include "../include/file_utils.h"

cag_option options[] = {
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

struct option_info flags_info[] = {
    {'C', "Ok"},
    {'O', "CdDe"},
    {'d', "D"}
};

const cag_option *getProgramOptions() {
    return options;
}

size_t getProgramOptionsSize() {
    return sizeof(options) / sizeof(options[0]);
}

int handleFlags(const int argc, char *argv[], flag_state_t *flags) {
    const cag_option *PROGRAM_OPTIONS = getProgramOptions();
    const size_t OPTIONS_SIZE = getProgramOptionsSize();
    cag_option_context context;

    cag_option_init(&context, PROGRAM_OPTIONS, OPTIONS_SIZE, argc, argv);
    while (cag_option_fetch(&context)) {
        switch (cag_option_get_identifier(&context)) {
            case 'C':
                flags->copy_mode = true;
                break;
            case 'O':
                flags->overwrite_mode = true;
                break;
            case 'd':
                flags->copied_file_path = true;
                break;
            case 'D':
                flags->copied_dir_path = true;
                break;
            case 'e':
                flags->use_editor = true;
                if (cag_option_get_value(&context) != NULL) {
                    flags->editor = cag_option_get_value(&context);
                }
                break;
            case 'k':
                flags->keep_copy = true;
                break;
            case '?':
                cag_option_print_error(&context, stdout);
                return ERROR_INVALID_ARGUMENT;
        }
    }

    flags->param_index = cag_option_get_index(&context);

    if (!checkProgramFlags(flags->copy_mode, flags->overwrite_mode, flags->copied_file_path,
                           flags->copied_dir_path, flags->use_editor, flags->keep_copy)) {
        return ERROR_INVALID_ARGUMENT;
    }

    return SUCCESS;
}

bool checkProgramFlags(const bool copy_mode, const bool overwrite_mode, const bool copied_file_path,
                       const bool copied_dir_path,
                       const bool e_included, const bool keep_copy) {
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
                                k < strlen(present_incompatible_flags) - 1 ? ", " : "");
                    }
                    fprintf(stderr, " are incompatible with flag '%c'.\n%s\n", active_flags[i], tryHelpMessage());
                    return false;
                }
            }
        }
    }
    return true;
}
