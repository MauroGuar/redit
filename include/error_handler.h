#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

/**
 * @file error_handler.h
 * @brief Provides centralized error handling and defines error codes for the `redit` program.
 *
 * This file includes:
 * - `error_codes` enum: A list of predefined error codes used throughout the program.
 * - `printError` function: Maps error codes to descriptive messages and outputs them.
 */

/**
 * @enum error_codes
 * @brief Defines standard error codes for the program.
 *
 * These error codes are used to communicate specific failure conditions across the program.
 */
enum error_codes {
    SUCCESS = 0, ///< Operation completed successfully.
    USER_EXIT, ///< User chose to exit the operation.
    ERROR_FILE_NOT_FOUND, ///< File not found.
    ERROR_PERMISSION_DENIED, ///< Permission denied.
    ERROR_MEMORY_ALLOCATION, ///< Memory allocation failed.
    ERROR_COPY_FAILED, ///< Copy operation failed.
    ERROR_INVALID_ARGUMENT, ///< Invalid argument provided.
    ERROR_SAME_SOURCE, ///< Copy and privileged paths are the same.
    ERROR_CWD, ///< Current working directory error.
    ERROR_RESOLVING_PATH, ///< Failed to resolve a path.
    ERROR_BUFFER_TOO_SMALL, ///< Buffer size is insufficient.
    ERROR_USER_NOT_FOUND, ///< User not found.
    ERROR_EXECUTING_COMMAND, ///< Command execution failed.
    ERROR_PATH_INVALID, ///< Invalid path provided.
    ERROR_PATH_TOO_LONG, ///< Path length exceeds the maximum limit.
    ERROR_INVALID_SOURCE, ///< Invalid copy file.
    HELP_DISPLAYED = 100, ///< Help message displayed.
    ERROR_COMMAND_NOT_FOUND = 256, ///< Command not found.
    UNKNOWN_ERROR = 666 ///< An unknown error occurred.
};

/**
 * @brief Prints a descriptive error message based on the provided error code.
 *
 * This function maps predefined error codes to human-readable messages and
 * outputs them to `stderr`. Optionally, a context string can be provided
 * to indicate the specific operation or location where the error occurred.
 *
 * @param error_code The error code to interpret. This should correspond to a
 *                   predefined error in `error_codes`.
 * @param context A string describing the context of the error (e.g., "validating path").
 *                If `NULL`, no context will be appended to the message.
 * @return int The same `error_code` that was passed in, allowing for error
 *             propagation through the program's logic.
 */
int printError(const int error_code, const char *context);

#endif // ERROR_HANDLER_H
