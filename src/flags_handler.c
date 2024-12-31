#include <stdio.h>
#include <stdbool.h>
#include <cargs.h>

#include "../include/flags_handler.h"

#include <string.h>

#include "../include/error_handler.h"
#include "../include/file_utils.h"

/**
 * @file flags_handler.c
 * @brief Handles command-line flags for the `redit` program.
 * 
 * This file provides functionality for parsing, validating, and managing
 * command-line flags. It ensures compatibility between flags and sets up
 * program options for further processing. Additionally, it provides help
 * messages to guide the user on how to use the command-line tool effectively.
 */

/**
 * @brief Defines the available command-line options for the `redit` program.
 */
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

/**
 * @brief Displays a detailed help message for the `redit` program.
 */
void displayHelp();

/**
 * @brief Specifies incompatible flag combinations for validation.
 * 
 * It is used to check if the active flags are compatible with each other.
 */
struct option_info {
    const char identifier;
    const char *incompatible_flags;
};

/**
 * @brief Specifies incompatible flag combinations for validation.
 */
struct option_info flags_info[] = {
    {'C', "Ok"},     // Copy mode is incompatible with overwrite and overwrite-related flags
    {'O', "CdDe"},   // Overwrite mode is incompatible with copy and copy-related flags
    {'d', "D"}       // -d (file) and -D (directory) are mutually exclusive
};

/**
 * @brief Returns the available program options.
 * 
 * @return Pointer to the array of cag_option structures defining the options.
 */
const cag_option *getProgramOptions() {
    return options;
}

/**
 * @brief Returns the size of the program options array.
 * 
 * @return Number of options defined in the program.
 */
size_t getProgramOptionsSize() {
    return sizeof(options) / sizeof(options[0]);
}

/**
 * @brief Parses and handles command-line flags.
 * 
 * It processes the provided command-line arguments, updates the flag state structure,
 * and ensures that no incompatible flags are used together. If the help flag is provided,
 * it displays the help message.
 * 
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param flags Pointer to the structure storing the parsed flag states.
 * @return `SUCCESS` if flags are handled successfully, or an error code otherwise.
 */
int handleFlags(const int argc, char *argv[], flag_state_t *flags) {
    const cag_option *PROGRAM_OPTIONS = getProgramOptions();
    const size_t OPTIONS_SIZE = getProgramOptionsSize();
    cag_option_context context;

    cag_option_init(&context, PROGRAM_OPTIONS, OPTIONS_SIZE, argc, argv);
    while (cag_option_fetch(&context)) { // Parse the options
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
                flags->use_editor = true; // Editor is included
                if (cag_option_get_value(&context) != NULL) { // Get the editor value if specified
                    flags->editor = cag_option_get_value(&context);
                }
                break;
            case 'k':
                flags->keep_copy = true;
                break;
            case 'h':
                displayHelp(); // Display help message if 'h' flag is provided
                return HELP_DISPLAYED;
            case '?':
                cag_option_print_error(&context, stdout); // Print error message for invalid option
                return ERROR_INVALID_ARGUMENT;
        }
    }

    flags->param_index = cag_option_get_index(&context); // Get the index of the first non-flag parameter

    // Check for incompatible flag combinations
    if (!checkProgramFlags(flags->copy_mode, flags->overwrite_mode, flags->copied_file_path,
                           flags->copied_dir_path, flags->use_editor, flags->keep_copy)) {
        return ERROR_INVALID_ARGUMENT;
    }

    return SUCCESS; // Flags are valid
}

/**
 * @brief Validates compatibility between active flags.
 * 
 * @param copy_mode Indicates if copy mode is active.
 * @param overwrite_mode Indicates if overwrite mode is active.
 * @param copied_file_path Indicates if a specific file is set as the copy path.
 * @param copied_dir_path Indicates if a directory is set as the copy path.
 * @param e_included Indicates if an editor is specified.
 * @param keep_copy Indicates if the copy should be kept after overwriting.
 * @return `true` if the flags are valid, `false` if there are conflicts.
 */
bool checkProgramFlags(const bool copy_mode, const bool overwrite_mode, const bool copied_file_path,
                       const bool copied_dir_path,
                       const bool e_included, const bool keep_copy) {

    // Check if either copy or overwrite mode is active
    if (!(copy_mode || overwrite_mode)) {
        fprintf(stderr, "Error: Must use either -C or -O.\n%s\n", tryHelpMessage());
        return false;
    }

    constexpr size_t NUMBER_FLAGS = 6; // Number of flags

    char active_flags[NUMBER_FLAGS + 1] = {0}; // Array to store active flags
    size_t flag_index = 0; // Index to track the number of active flags

    // Store the active flags in the array
    if (copy_mode) active_flags[flag_index++] = 'C';
    if (overwrite_mode) active_flags[flag_index++] = 'O';
    if (copied_file_path) active_flags[flag_index++] = 'd';
    if (copied_dir_path) active_flags[flag_index++] = 'D';
    if (e_included) active_flags[flag_index++] = 'e';
    if (keep_copy) active_flags[flag_index++] = 'k';
    active_flags[flag_index] = '\0';

    const size_t flags_info_size = sizeof(flags_info) / sizeof(flags_info[0]); // Number of flag combinations
    

    /**
     * @section Check for incompatible flag combinations.
     * 
     * Iterates through the active flags and compares them with the incompatible
     * flags specified in the `flags_info` array. If any incompatible flags are found,
     * an error message is displayed, and the function returns `false`.
    */
    for (size_t i = 0; i < flag_index; ++i) { // Iterate through active flags
        for (size_t j = 0; j < flags_info_size; ++j) { // Iterate through incompatible flags
            if (active_flags[i] == flags_info[j].identifier) { // Check if the flag is active
                // Get the incompatible flags for the current flag
                const char *incompatible_flags = flags_info[j].incompatible_flags;

                // Array to store incompatible flags present
                char present_incompatible_flags[NUMBER_FLAGS] = {0};
                size_t inc_index = 0;

                // Iterate through the active flags to find incompatible flags
                for (size_t ic = 0; ic < strlen(incompatible_flags); ++ic) {
                    // Check if the active flag and incompatible flag are the same
                    if (strchr(active_flags, incompatible_flags[ic])) {
                        // Store the incompatible flag in the array
                        present_incompatible_flags[inc_index++] = incompatible_flags[ic];
                    }
                }
                present_incompatible_flags[inc_index] = '\0';

                if (inc_index > 0) { // If incompatible flags are present
                    // Display error message with incompatible flags
                    fprintf(stderr, "Error: Flag/s ");
                    for (size_t k = 0; k < strlen(present_incompatible_flags); ++k) {
                        fprintf(stderr, "'%c'%s", present_incompatible_flags[k],
                                k < strlen(present_incompatible_flags) - 1 ? ", " : "");
                    }
                    fprintf(stderr, " are incompatible with flag '%c'.\n%s\n", active_flags[i], tryHelpMessage());
                    return false; // Return false if incompatible flags are found
                }
            }
        }
    }
    return true; // Return true if no incompatible flags are found
}

/**
 * @brief Displays a detailed help message for the user.
 */
void displayHelp() {
    printf("Usage: redit [OPTIONS] <copy_file> [privileged_file]\n");
    printf("\n");
    printf("A command-line tool for editing or copying privileged files securely.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -C, --copy              Copy the privileged file to the copy file destination.\n");
    printf("  -O, --overwrite         Overwrite the copy file over the privileged file.\n");
    printf("  -d, --cfile             Specify the copy file destination as a file.\n");
    printf("  -D, --dfile             Specify the copy file destination as a directory.\n");
    printf("  -e, --editor <editor>   Use the specified editor for the operation.\n");
    printf("                          Defaults to the value of the REDIT_EDITOR environment variable\n");
    printf("                          or the program's default editor.\n");
    printf("  -k, --keep              Keep the copy file after overwriting.\n");
    printf("  -h, --help              Display this help message.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  redit -C /privileged/privileged.txt\n");
    printf("      Copy 'privileged.txt' to the current working directory.\n");
    printf("\n");
    printf("  redit -O /privileged/privileged.txt\n");
    printf("      Overwrite '/privileged/privileged.txt' with a copy stored with the same file name\n");
    printf("      stored in the current working directory.\n");
    printf("\n");
    printf("  redit -Cd -e vim source.txt /privileged/destination.txt\n");
    printf("      Copy '/privileged/destination.txt' to './source.txt' and open it with Vim.\n");
    printf("\n");
    printf("Environment Variables:\n");
    printf("  REDIT_EDITOR            Specifies the default editor to use when the -e flag is omitted.\n");
    printf("\n");
    printf("IMPORTANT:\n");
    printf("  - The command must be executed with sufficient privileges to access the privileged file.\n");
    printf("  - If using sudo, make sure to use 'sudo -E' to preserve the environment variables.\n");
    printf("  - You can also use 'sudo --preserve-env=REDIT_EDITOR' to preserve only the REDIT_EDITOR variable.\n");
    printf("  - For more information, visit: https://github.com/MauroGuar/redit\n");
    printf("\n");
}
