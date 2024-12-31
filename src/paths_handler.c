#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <errno.h>
#include <pwd.h>
#include <regex.h>
#include <ctype.h>

#include "../include/error_handler.h"
#include "../include/paths_handler.h"
#include "../include/flags_handler.h"
#include "../include/file_operations.h"
#include "../include/file_utils.h"

/**
 * @file paths_handler.c
 * @brief Provides functions to resolve, validate, and handle file paths for the `redit` program.
 *
 * This file contains utilities to process paths for copying or overwriting files.
 * It includes normalization, absolute path resolution, directory creation, and path validation.
 * The goal is to ensure robust and error-free handling of paths for file operations.
 */

/**
 * @brief Resolves and validates paths for copy and overwrite operations.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param flags Pointer to the structure storing the parsed flag states.
 * @param copy_file_path Buffer to store the resolved copy file path.
 * @param privileged_file_path Buffer to store the resolved privileged file path.
 * @return `SUCCESS` if paths are resolved and validated successfully, or an error code otherwise.
 *
 * @details
 * - Handles both file and directory copy paths.
 * - Resolves absolute paths for both source and destination files.
 * - Validates that paths exist and meet access requirements.
 */
int resolveAndValidatePaths(const int argc, char *argv[], const flag_state_t *flags,
                            char copy_file_path[PATH_MAX], char privileged_file_path[PATH_MAX]) {
    // Checks if the 'd' or 'D' flag is set
    // This means that the user has specified a directory or file path for the copy file
    if (flags->copied_file_path || flags->copied_dir_path) {
        // Check if there is the right number of arguments
        if (flags->param_index + 1 >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/copy/file /path/to/original/file\n%s\n", argv[0], tryHelpMessage());
            return ERROR_INVALID_ARGUMENT;
        }

        // Get the absolute path of the privileged file
        const int prv_path_result = getAbsolutePath(argv[flags->param_index + 1], privileged_file_path);
        if (prv_path_result != SUCCESS) {
            return printError(prv_path_result, "resolving privileged file path");
        }

        // Validate the privileged file path
        const int prv_valid_result = validatePath(privileged_file_path, true, false);
        if (prv_valid_result != SUCCESS) {
            return printError(prv_valid_result, "validating privileged file path");
        }

        // Get the absolute path of the copy file
        const int cpy_path_result = getAbsolutePathFuture(argv[flags->param_index], copy_file_path);
        if (cpy_path_result != SUCCESS) {
            return printError(cpy_path_result, "resolving copy file path");
        }

        // Check whether the user has specified a directory path for the copy file
        if (flags->copied_dir_path) {
            // Since the path is a directory, we need to get the base name of the privileged file
            // and append it to the copy file path
            const char *file_base_name = basename(privileged_file_path); // Get the base name of the privileged file
            const int abs_file_path_result = getAbsFilePathFromDir(copy_file_path, file_base_name);
            // Append the base name to the copy file path
            if (abs_file_path_result != SUCCESS) {
                return printError(abs_file_path_result, "getting absolute file path from directory");
            }
        }

        // Validate the copy file path and create it if it doesn't exist
        const int validation_result = validateOrCreatePath(copy_file_path, true, false);
        if (validation_result != SUCCESS) {
            return printError(validation_result, "validating copy file path");
        }
    } else {
        // Check if there is the right number of arguments
        if (flags->param_index >= argc) {
            fprintf(stderr, "Usage: %s -C /path/to/original/file\n%s\n", argv[0], tryHelpMessage());
            return ERROR_INVALID_ARGUMENT;
        }

        // Resolve current working directory
        char cwd[PATH_MAX];
        const int cwd_result = getCurrentWorkingDirectory(cwd);
        if (cwd_result != SUCCESS) {
            return printError(cwd_result, "resolving current working directory");
        }

        // Get the absolute path of the privileged file
        const int prv_path_result = getAbsolutePath(argv[flags->param_index], privileged_file_path);
        if (prv_path_result != SUCCESS) {
            return printError(prv_path_result, "resolving privileged file path");
        }

        // Validate the privileged file path
        const int prv_validation_result = validatePath(privileged_file_path, true, false);
        if (prv_validation_result != SUCCESS) {
            return printError(prv_validation_result, "validating privileged file path");
        }

        // Since the user has not specified a copy file path, we need to create one
        // by appending the base name of the privileged file to the current working directory
        const char *base_name = basename(privileged_file_path); // Get the base name of the privileged file
        snprintf(copy_file_path, strlen(cwd) + strlen(base_name) + 2, "%s/%s", cwd, base_name);
        // Append the base name to the current working directory

        // Validate the copy file path and create it if it doesn't exist
        const int validation_result = validatePath(copy_file_path, false, true);
        if (validation_result != SUCCESS) {
            return printError(validation_result, "validating copy file path");
        }
    }

    return SUCCESS;
}

/**
 * @brief Normalizes slashes in a file path.
 *
 * @param input_path The input path to normalize.
 * @param normalized_path Buffer to store the normalized path.
 *
 * @details
 * - Removes duplicate slashes in the path.
 * - Ensures that the path ends with a single slash.
 *
 * @example
 * - Input: `/path////to//file//`
 * - Output: `/path/to/file/`
 */
void normalizeSlashes(const char *input_path, char normalized_path[PATH_MAX]) {
    int input_index = 0, output_index = 0; // Indexes for input and output paths

    // Loop through the input path until the end (null terminator)
    while (input_path[input_index] != '\0') {
        normalized_path[output_index++] = input_path[input_index]; // Copy the current character to the output path

        // Check if the current character is a slash
        if (input_path[input_index] == '/') {
            // Skip any subsequent slashes
            while (input_path[input_index + 1] == '/') {
                input_index++;
            }
        }
        input_index++; // Move to the next character in the input path
    }

    // Ensures that the path does not end with any slashes
    if (normalized_path[output_index - 1] == '/') {
        normalized_path[output_index - 1] = '\0';
    } else {
        normalized_path[output_index] = '\0';
    }
}

/**
 * @brief Resolves the absolute path of a file.
 *
 * @param original_path The original file path.
 * @param resolved_path Buffer to store the resolved absolute path.
 * @return `SUCCESS` if resolved successfully, or an error code otherwise.
 *
 * @details
 * - Converts a relative path to an absolute path.
 * - It only works for existing files/directories.
 */
int getAbsolutePath(const char *original_path, char resolved_path[PATH_MAX]) {
    if (realpath(original_path, resolved_path) == NULL) {
        if (errno == ENOENT) {
            return ERROR_FILE_NOT_FOUND;
        }
        return ERROR_RESOLVING_PATH;
    }
    return SUCCESS;
}

/**
 * @brief Resolves an absolute path with future-proof normalization.
 *
 * @param original_path The original file path.
 * @param resolved_path Buffer to store the resolved path.
 * @return `SUCCESS` if resolved successfully, or an error code otherwise.
 *
 * @details
 * - It is designed to handle future-proof path normalization.
 * - Handles special cases like `.` (current directory), `..` (parent directory), `~` (home directory), and environment variables.
 */
int getAbsolutePathFuture(const char *original_path, char resolved_path[PATH_MAX]) {
    // Normalize the slashes in the original path
    // It also removes the end slashes if present
    char normalized_path[PATH_MAX];
    normalizeSlashes(original_path, normalized_path);

    // Path begins with '.' (current directory)
    if (normalized_path[0] == '.' && (normalized_path[1] == '\0' || normalized_path[1] == '/')) {
        // Get the absolute path of the current directory
        if (realpath(".", resolved_path) == NULL) {
            return ERROR_RESOLVING_PATH;
        }
        // If the path is just '.', return the current directory
        if (normalized_path[1] == '\0') {
            return SUCCESS;
        }

        // Append the rest of the path to the current directory
        snprintf(resolved_path + strlen(resolved_path), PATH_MAX - strlen(resolved_path), "/%s", normalized_path + 2);
        return SUCCESS;
    }

    // Paths begins with a letter (e.g., "path/to/file")
    // It is treated as if a "./" is prepended to the path (e.g., "./path/to/file")
    if (isalpha(normalized_path[0])) {
        // Get the absolute path of the current directory
        if (realpath(".", resolved_path) == NULL) {
            return ERROR_RESOLVING_PATH;
        }

        // Append the path to the current directory
        snprintf(resolved_path + strlen(resolved_path), PATH_MAX - strlen(resolved_path), "/%s", normalized_path);
        return SUCCESS;
    }

    // Path begins with '..' (parent directory)
    if (normalized_path[0] == '.' && normalized_path[1] == '.' && (
            normalized_path[2] == '\0' || normalized_path[2] == '/')) {
        // Get every '..' in the path
        int last_index = 0;
        // Loop through the path until a letter is found
        for (int i = 0; !isalpha(normalized_path[i]) && normalized_path[i] != '\0'; ++i) {
            resolved_path[i] = normalized_path[i]; // Copy the current character to the resolved path
            last_index = i; // Update the last index
        }
        resolved_path[last_index + 1] = '\0';

        // Get the absolute path of the all the '..' in the path
        if (realpath(resolved_path, resolved_path) == NULL) {
            return ERROR_RESOLVING_PATH;
        }

        // Checks if there is a letter after the all the '..'
        if (isalpha(normalized_path[last_index + 1])) {
            // Append the rest of the path to the parent directory
            snprintf(resolved_path + strlen(resolved_path), PATH_MAX - strlen(resolved_path), "%s",
                     normalized_path + last_index);
        }
        return SUCCESS;
    }

    // Path begins with one '~' (home directory)
    if (normalized_path[0] == '~' && (normalized_path[1] == '/' || normalized_path[1] == '\0')) {
        const char *home = getenv("HOME"); // Get the home directory from the environment variables
        if (home == NULL) {
            return ERROR_RESOLVING_PATH;
        }

        // Append the rest of the path to the home directory
        snprintf(resolved_path, PATH_MAX, "%s%s", home, normalized_path + 1);
        return SUCCESS;
    }

    // Path begins with '~' followed by a letter (e.g., "~user/path/to/file")
    // It means that the path is relative to another user's home directory
    if (normalized_path[0] == '~' && isalpha(normalized_path[1])) {
        char other_user[PATH_MAX];
        sscanf(normalized_path, "~%[^/]", other_user); // Get the other user's name
        const struct passwd *pw = getpwnam(other_user); // Get the other user's data
        if (pw == NULL) {
            return ERROR_RESOLVING_PATH;
        }

        // Append the rest of the path to the other user's home directory
        snprintf(resolved_path, PATH_MAX, "%s%s", pw->pw_dir, normalized_path + strlen(other_user) + 1);
        return SUCCESS;
    }

    // Path begins with an environment variable (e.g., "$HOME/path/to/file")
    if (normalized_path[0] == '$' && isalpha(normalized_path[1])) {
        char env_path[256];
        sscanf(normalized_path + 1, "%[^/]", env_path); // Get the environment variable name
        const char *env_value = getenv(env_path); // Get the environment variable value
        if (env_value == NULL) {
            return ERROR_RESOLVING_PATH; // Return an error if the environment variable is not found
        }

        // Normalize and get the absolute path of the environment variable value
        char abs_env_value[PATH_MAX];
        const int env_result = getAbsolutePathFuture(env_value, abs_env_value);
        if (env_result != SUCCESS) { return env_result; }

        // Append the rest of the path to the environment variable value
        snprintf(resolved_path, PATH_MAX, "%s%s", abs_env_value, normalized_path + strlen(env_path) + 1);
        return SUCCESS;
    }

    // Path begins with a slash followed by a letter (e.g., "/path/to/file")
    // It is treated as an absolute path. No further processing is needed
    if (normalized_path[0] == '/' && isalpha(normalized_path[1])) {
        // Copy the normalized path to the resolved path
        snprintf(resolved_path, PATH_MAX, "%s", normalized_path);
        return SUCCESS;
    }

    return ERROR_PATH_INVALID;
}

/**
 * @brief Combines a directory path and a file name into a single path.
 *
 * @param path The directory path.
 * @param file_name The file name to append.
 * @return `SUCCESS` if combined successfully, or an error code otherwise.
 */
int getAbsFilePathFromDir(char path[PATH_MAX], const char *file_name) {
    // Check if the combined path will exceed the maximum path length
    if (strlen(path) + strlen(file_name) + 2 >= PATH_MAX) {
        return ERROR_PATH_TOO_LONG;
    }

    // Append the file name to the directory path
    const int last_index = strlen(path) - 1;
    if (path[last_index] != '/') {
        path[last_index + 1] = '/';
        path[last_index + 2] = '\0';
    }
    snprintf(path + strlen(path), PATH_MAX - strlen(path), "%s", file_name);
    return SUCCESS;
}

/**
 * @brief Validates a path for read and/or write permissions.
 *
 * @param path The path to validate.
 * @param check_read Check for read permissions.
 * @param check_write Check for write permissions.
 * @return `SUCCESS` if valid, or an error code otherwise.
 */
int validatePath(const char path[PATH_MAX], const bool check_read, const bool check_write) {
    char path_copy[PATH_MAX];
    strcpy(path_copy, path);
    const char *path_dir = dirname(path_copy);

    // Check if the path has read permissions
    if (check_read && access(path_dir, R_OK) == -1) {
        return ERROR_PERMISSION_DENIED;
    }

    // Check if the path has write permissions
    if (check_write && access(path_dir, W_OK) == -1) {
        return ERROR_PERMISSION_DENIED;
    }

    return SUCCESS;
}

/**
 * @brief Creates directories recursively along the specified path.
 *
 * @param path The path for which directories will be created.
 * @return `SUCCESS` if created successfully, or an error code otherwise.
 */
int createDirRecursively(const char *path) {
    // Make a copy of the path to avoid modifying the original
    char temp_path[PATH_MAX];
    strncpy(temp_path, path, sizeof(temp_path));

    // Ensure that the path ends with a slash
    const int last_index = strlen(temp_path);
    if (temp_path[last_index - 1] != '/') {
        // Check if the path is too long
        if (strlen(temp_path) + 2 > PATH_MAX) {
            return ERROR_PATH_TOO_LONG;
        }
        // Append a slash to the end of the path
        temp_path[last_index] = '/';
        temp_path[last_index + 1] = '\0';
    }

    size_t mkdir_counter = 0; // Counter for the number of directories created

    // Loop through the path and create directories as needed
    for (size_t pos = 1; temp_path[pos] != '\0'; ++pos) {
        // Check if a slash is found to create a directory
        if (temp_path[pos] == '/') {
            temp_path[pos] = '\0'; // Temporarily truncate the path at the slash

            // Create the directory if it doesn't exist and user has permission
            if (mkdir(temp_path, 0755) == SUCCESS) {
                // Get the effective user ID to set as the owner of the directory
                uid_t ef_uid;
                const int user_result = getEffectiveUserId(&ef_uid);
                if (user_result == ERROR_USER_NOT_FOUND) {
                    return ERROR_USER_NOT_FOUND;
                }

                // Change the owner of the directory to the effective user
                const int chown_result = changeFileOwner(temp_path, ef_uid);
                if (chown_result == ERROR_PERMISSION_DENIED) {
                    return ERROR_PERMISSION_DENIED;
                }

                mkdir_counter++; // Increment the directory counter
            }
            temp_path[pos] = '/'; // Restore the slash in the path
        }
    }

    // Check if no directories were created
    if (mkdir_counter == 0) {
        return ERROR_PATH_INVALID;
    }
    return SUCCESS; // Return success if directories were created
}

/**
 * @brief Validates a path or creates it if it doesn't exist.
 *
 * @param path The path to validate or create.
 * @param check_read Check for read permissions.
 * @param check_write Check for write permissions.
 * @return `SUCCESS` if valid or created successfully, or an error code otherwise.
 */
int validateOrCreatePath(const char path[PATH_MAX], const bool check_read, const bool check_write) {
    struct stat path_stat;
    char path_copy[PATH_MAX];
    strcpy(path_copy, path); // Make a copy of the path to avoid modifying the original
    const char *path_dir = dirname(path_copy); // Get the directory of the path

    // Check if the path exists
    if (stat(path_dir, &path_stat) == -1) {
        if (errno == ENOENT) {
            // Prompt the user to create the directory if it doesn't exist
            printf("The path '%s' does not exist. Do you want to create it? (y/n): ", path_copy);
            char response;
            while (scanf(" %c", &response) != 1 || (
                       response != 'y' && response != 'Y' && response != 'n' && response != 'N')) {
                printf("Invalid input. Please enter 'y' or 'n': ");
            }
            if (response == 'n' || response == 'N') {
                return USER_EXIT;
            }

            // Create the directory recursively
            const int create_result = createDirRecursively(path_dir);
            switch (create_result) {
                case ERROR_PATH_TOO_LONG:
                    return ERROR_PATH_TOO_LONG;
                case ERROR_PATH_INVALID:
                    return ERROR_PATH_INVALID;
            }
            return SUCCESS;
        }
        return ERROR_PATH_INVALID; // If the errno is not ENOENT, the path is invalid
    }

    // Validate the path if it exists
    const int val_result = validatePath(path, check_read, check_write);
    if (val_result == ERROR_PERMISSION_DENIED) {
        return ERROR_PERMISSION_DENIED;
    }

    return SUCCESS;
}
