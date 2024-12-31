#ifndef FLAGS_HANDLER_H
#define FLAGS_HANDLER_H

/**
 * @file flags_handler.h
 * @brief Header file for handling command-line flags in the `redit` program.
 *
 * This file declares the `flag_state_t` structure and the functions necessary
 * for parsing, validating, and managing flags provided via the command-line interface.
 */

/**
 * @struct flag_state_t
 * @brief Stores the state of flags provided via the command-line.
 *
 * This structure consolidates all flags into a single object for easier management
 * throughout the program.
 */
typedef struct {
    bool copy_mode; ///< Indicates if the copy mode (-C) is active.
    bool overwrite_mode; ///< Indicates if the overwrite mode (-O) is active.
    bool copied_file_path; ///< Indicates if the copy file is specified as a file (-d).
    bool copied_dir_path; ///< Indicates if the copy file is specified as a directory (-D).
    bool use_editor; ///< Indicates if an editor should be used (-e).
    bool keep_copy; ///< Indicates if the copy file should be kept after overwriting (-k).
    const char *editor; ///< Stores the editor specified with the -e flag.
    int param_index; ///< Index of the first non-flag parameter in `argv`.
} flag_state_t;

/**
 * @brief Handles and validates command-line flags.
 *
 * Parses the provided arguments, updates the `flag_state_t` structure, and ensures
 * that no incompatible flags are used together. Also displays help information if
 * requested via the -h flag.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @param flags Pointer to the `flag_state_t` structure to store parsed flags.
 * @return int A status code indicating success or failure:
 *             - HELP_DISPLAYED if help was shown.
 *             - SUCCESS if parsing succeeded.
 *             - An error code if an invalid flag combination is detected.
 */
int handleFlags(int argc, char *argv[], flag_state_t *flags);

#endif // FLAGS_HANDLER_H
