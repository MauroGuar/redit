#include <stdio.h>
#include <stdbool.h>
#include <linux/limits.h>

#include "../include/error_handler.h"
#include "../include/flags_handler.h"
#include "../include/paths_handler.h"
#include "../include/modes_handler.h"

/**
 * @file main.c
 * @brief Main entry point for the `redit` program.
 *
 * The program securely edits or copies privileged files by using a temporary
 * user-editable file. It handles command-line arguments, resolves and validates paths,
 * and executes the selected mode (copy or overwrite).
 *
 * @author MauroGuar
 */

#define PROGRAM_DEFAULT_EDITOR "nano" // Default editor to be used if none is specified

/**
 * @brief Main entry point for the `redit` program.
 *
 * This function initializes the program by handling command-line arguments,
 * resolving and validating file paths, and executing the selected mode (copy or overwrite).
 * It ensures that the program operates securely and efficiently by managing temporary
 * user-editable files and privileged files.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @return int Exit code indicating the success (0) or failure (non-zero) of the operation.
 */
int main(const int argc, char *argv[]) {
    /**
     * @section Flag Handling
     * Parses command-line arguments to determine the operation mode (copy or overwrite),
     * optional settings (e.g., editor to use), and validates flag compatibility.
     */
    flag_state_t flags = {0}; // Struct to store the state of flags
    const int flags_result = handleFlags(argc, argv, &flags);
    if (flags_result == HELP_DISPLAYED) {
        return SUCCESS; // Exit after displaying help
    }
    if (flags_result != SUCCESS) {
        return flags_result; // Return the error code if flag handling fails
    }

    /**
     * @section Path Resolution and Validation
     * Resolves absolute paths for the copy and privileged files, validates the paths,
     * and ensures any necessary directories are created.
     */
    char copy_file_path[PATH_MAX]; // Path to the copy file
    char privileged_file_path[PATH_MAX]; // Path to the privileged file
    const int paths_handle_result = resolveAndValidatePaths(argc, argv, &flags, copy_file_path, privileged_file_path);
    if (paths_handle_result != SUCCESS) {
        return paths_handle_result; // Return the error code if path handling fails
    }

    /**
     * @section Mode Execution
     * Executes the selected mode (copy or overwrite). Depending on the flags provided,
     * it handles file ownership, permissions, and optionally opens the file in an editor.
     */
    const int mode_result = executeFileMode(
        flags.copy_mode, // True if copy mode is selected
        copy_file_path, // Path to the copy file
        privileged_file_path, // Path to the privileged file
        flags.keep_copy, // True if the copy should be preserved after overwriting
        flags.editor, // User-specified editor (if any)
        flags.use_editor, // True if an editor should be used
        PROGRAM_DEFAULT_EDITOR // Default editor fallback
    );
    if (mode_result != SUCCESS) {
        return mode_result; // Return the error code if mode execution fails
    }

    /**
     * @section Exit
     * The program exits with a success code if all operations are completed successfully.
     */
    return SUCCESS;
}
