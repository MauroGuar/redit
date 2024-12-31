#ifndef FILE_MODES_H
#define FILE_MODES_H

#include <stdbool.h>

/**
 * @file modes_handler.h
 * @brief Provides functionality to handle execution of `copy` and `overwrite` modes.
 *
 * This file declares the `executeFileMode` function, which determines the mode to execute
 * based on user input and invokes the corresponding internal implementation.
 */

/**
 * @brief Executes the appropriate mode (`copy` or `overwrite`) based on user input.
 *
 * This function determines whether to perform a `copy` or `overwrite` operation
 * and delegates the implementation to the respective function. It handles file
 * permissions, ownership, and optionally opens the file in an editor.
 *
 * @param is_copy Indicates whether the operation is in `copy` mode.
 * @param copy_file_path The path to the copy file.
 * @param privileged_file_path The path to the privileged file.
 * @param keep_copy Indicates whether to keep the copy file after overwriting.
 * @param editor The editor to use if specified by the user.
 * @param use_editor Indicates whether an editor should be invoked.
 * @param program_default_editor The default editor to use if none is specified.
 * @return int A status code indicating the success or failure of the operation:
 *             - `SUCCESS` on success.
 *             - An appropriate error code on failure.
 */
int executeFileMode(const bool is_copy, const char *copy_file_path, const char *privileged_file_path,
                    bool keep_copy, const char *editor, bool use_editor, const char *program_default_editor);

#endif // FILE_MODES_H
