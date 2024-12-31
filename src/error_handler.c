#include <stdio.h>
#include "../include/error_handler.h"

/**
 * @file error_handler.c
 * @brief Implementation of the centralized error handling mechanism.
 *
 * This file contains the `printError` function, which interprets error codes
 * and prints corresponding messages to `stderr`.
 */

/**
 * @brief Prints a descriptive error message based on the provided error code.
 *
 * The function uses a switch-case construct to match the given error code with
 * its corresponding message. If the `context` is provided, it is appended to
 * the message for additional clarity. If the error code is not recognized, a
 * generic "unknown error" message is displayed.
 *
 * @param error_code The error code to interpret.
 * @param context A string describing the context of the error. Can be `NULL`.
 * @return int The input `error_code` for propagation.
 */
int printError(const int error_code, const char *context) {
    // Determine whether to include a blank space before the context
    char *blank_space = " ";
    if (context == NULL) {
        blank_space = "";
    }

    // Match error code to a specific message
    switch (error_code) {
        case USER_EXIT:
            return USER_EXIT;
        case ERROR_FILE_NOT_FOUND:
            fprintf(stderr, "Error%s%s: File not found.\n", blank_space, context);
            return ERROR_FILE_NOT_FOUND;
        case ERROR_PERMISSION_DENIED:
            fprintf(stderr, "Error%s%s: Permission denied.\n", blank_space, context);
            return ERROR_PERMISSION_DENIED;
        case ERROR_MEMORY_ALLOCATION:
            fprintf(stderr, "Error%s%s: Memory allocation failed.\n", blank_space, context);
            return ERROR_MEMORY_ALLOCATION;
        case ERROR_COPY_FAILED:
            fprintf(stderr, "Error%s%s: Copy failed.\n", blank_space, context);
            return ERROR_COPY_FAILED;
        case ERROR_INVALID_ARGUMENT:
            fprintf(stderr, "Error%s%s: Invalid argument.\n", blank_space, context);
            return ERROR_INVALID_ARGUMENT;
        case ERROR_SAME_SOURCE:
            fprintf(stderr, "Error%s%s: Copy and privileged paths are the same.\n", blank_space, context);
            return ERROR_SAME_SOURCE;
        case ERROR_CWD:
            fprintf(stderr, "Error%s%s: Current working directory error.\n", blank_space, context);
            return ERROR_CWD;
        case ERROR_RESOLVING_PATH:
            fprintf(stderr, "Error%s%s: Resolving path failed.\n", blank_space, context);
            return ERROR_RESOLVING_PATH;
        case ERROR_BUFFER_TOO_SMALL:
            fprintf(stderr, "Error%s%s: Buffer too small.\n", blank_space, context);
            return ERROR_BUFFER_TOO_SMALL;
        case ERROR_USER_NOT_FOUND:
            fprintf(stderr, "Error%s%s: User not found.\n", blank_space, context);
            return ERROR_USER_NOT_FOUND;
        case ERROR_EXECUTING_COMMAND:
            fprintf(stderr, "Error%s%s: Executing command failed.\n", blank_space, context);
            return ERROR_EXECUTING_COMMAND;
        case ERROR_PATH_INVALID:
            fprintf(stderr, "Error%s%s: Invalid path.\n", blank_space, context);
            return ERROR_PATH_INVALID;
        case ERROR_PATH_TOO_LONG:
            fprintf(stderr, "Error%s%s: Path too long.\n", blank_space, context);
            return ERROR_PATH_TOO_LONG;
        case ERROR_INVALID_SOURCE:
            fprintf(stderr, "Error%s%s: Invalid copy file.\n", blank_space, context);
            return ERROR_INVALID_SOURCE;
        case ERROR_COMMAND_NOT_FOUND:
            fprintf(stderr, "Error%s%s: Command not found.\n", blank_space, context);
            return ERROR_COMMAND_NOT_FOUND;
        default:
            fprintf(stderr, "Error%s%s: Unknown error code %d.\n", blank_space, context, error_code);
            return UNKNOWN_ERROR;
    }
}
