#include <stdio.h>
#include <stdbool.h>
#include <linux/limits.h>

#include "../include/error_handler.h"
#include "../include/flags_handler.h"
#include "../include/paths_handler.h"
#include "../include/modes_handler.h"

#define PROGRAM_DEFAULT_EDITOR "nano"

int copyMode(const char copy_file_path[PATH_MAX], const char privileged_file_path[PATH_MAX]);

int overwriteMode(const char copy_file_path[PATH_MAX], const char privileged_file_path[PATH_MAX], bool keep_copy);

int main(const int argc, char *argv[]) {
    flag_state_t flags = {0};
    const int flags_result = handleFlags(argc, argv, &flags);
    if (flags_result != SUCCESS) { return flags_result; }


    char copy_file_path[PATH_MAX];
    char privileged_file_path[PATH_MAX];
    const int paths_handle_result = resolveAndValidatePaths(argc, argv, &flags, copy_file_path, privileged_file_path);
    if (paths_handle_result != SUCCESS) { return paths_handle_result; }


    const int mode_result = executeFileMode(flags.copy_mode, copy_file_path, privileged_file_path, flags.keep_copy,
                                            flags.editor, flags.use_editor, PROGRAM_DEFAULT_EDITOR);
    if (mode_result != SUCCESS) { return mode_result; }


    return SUCCESS;
}
