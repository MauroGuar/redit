#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/file_operations.h"
#include "../include/file_utils.h"
#include "../include/error_handler.h"

/**
 * @file modes_handler.c
 * @brief Handles the execution of copy and overwrite modes for the `redit` program.
 *
 * This file implements the logic for managing two primary modes: copying files
 * (`copyMode`) and overwriting files (`overwriteMode`). It ensures proper file
 * handling, user ownership, and permission management. Additionally, it allows
 * for editing files with a specified or default editor.
 */

// Function prototypes
static int copyMode(const char *copy_file_path, const char *privileged_file_path, const char *editor,
                    bool use_editor,
                    const char *program_default_editor);

static int overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy);

/**
 * @brief Executes the appropriate mode based on the specified parameters.
 *
 * @param is_copy Indicates if the operation is in copy mode (`true`) or overwrite mode (`false`).
 * @param copy_file_path Path to the copy file used in the operation.
 * @param privileged_file_path Path to the privileged file involved in the operation.
 * @param keep_copy Indicates if the copy file should be kept after overwriting.
 * @param editor Editor to use if editor value is especified.
 * @param use_editor Indicates if an editor should be launched.
 * @param program_default_editor Default editor to use if no editor is specified.
 * @return `SUCCESS` if the operation completes successfully, or an error code otherwise.
 */
int executeFileMode(const bool is_copy, const char *copy_file_path, const char *privileged_file_path,
                    const bool keep_copy,
                    const char *editor,
                    const bool use_editor, const char *program_default_editor) {
    if (is_copy) {
        return copyMode(copy_file_path, privileged_file_path, editor, use_editor, program_default_editor);
    }
    return overwriteMode(copy_file_path, privileged_file_path, keep_copy);
}

/**
 * @brief Handles the file copying operation.
 *
 * @param copy_file_path Path to the destination copy file.
 * @param privileged_file_path Path to the source privileged file.
 * @param editor Editor to use if editor value is especified.
 * @param use_editor Indicates if an editor should be launched.
 * @param program_default_editor Default editor to use if no editor is specified.
 * @return `SUCCESS` if the operation completes successfully, or an error code otherwise.
 *
 * @details
 * - Copies the privileged file to the destination path.
 * - Changes the ownership of the copied file to the effective user.
 * - Updates the permissions of the copied file to allow editing.
 * - Optionally launches an editor to modify the copied file.
 */
static int copyMode(const char *copy_file_path, const char *privileged_file_path, const char *editor,
                    const bool use_editor,
                    const char *program_default_editor) {
    // Retrieve the effective user ID
    uid_t user_ef_id;
    const int uid_result = getEffectiveUserId(&user_ef_id);
    if (uid_result != SUCCESS) {
        return printError(uid_result, "getting effective user id");
    }

    // Copy the privileged file to the destination path
    const int copy_result = copyFile(privileged_file_path, copy_file_path);
    if (copy_result != SUCCESS) {
        return printError(copy_result, "copying file");
    }

    // Change the ownership of the copied file to the effective user
    const int chown_result = changeFileOwner(copy_file_path, user_ef_id);
    if (chown_result != SUCCESS) {
        return printError(chown_result, "changing file owner");
    }

    // Add write permissions for the user to the copied file
    const mode_t new_perms = S_IRUSR | S_IWUSR;
    const int add_perms_result = addFilePermissions(copy_file_path, new_perms);
    if (add_perms_result != SUCCESS) {
        return printError(add_perms_result, "adding file permissions");
    }

    // Launch the editor if requested
    if (use_editor) {
        const int editor_result = executeEditorCommand(editor, copy_file_path, program_default_editor);
        switch (editor_result) {
            // Handle editor execution errors
            case ERROR_USER_NOT_FOUND:
                return printError(editor_result, "getting user id for the editor command");
            case ERROR_MEMORY_ALLOCATION:
                fprintf(stderr, "Error allocating memory for editor command.\nProceeding without the editor.\n");
                printf("\n%s\n", copy_file_path);
                break;
            case ERROR_COMMAND_NOT_FOUND:
                fprintf(stderr, "Proceeding without the editor.\n");
                printf("\n%s\n", copy_file_path);
                break;
            case -1:
                fprintf(stderr, "Proceeding without the editor.\n");
                printf("\n%s\n", copy_file_path);
                break;
        }
    } else {
        // Output the copy file path for the user
        printf("%s\n", copy_file_path);
    }

    return SUCCESS;
}

/**
 * @brief Handles the file overwriting operation.
 *
 * @param copy_file_path Path to the source copy file.
 * @param privileged_file_path Path to the destination privileged file.
 * @param keep_copy Indicates if the copy file should be kept after overwriting.
 * @return `SUCCESS` if the operation completes successfully, or an error code otherwise.
 *
 * @details
 * - Retrieves the owner and permissions of the privileged file.
 * - Overwrites the privileged file with the content of the copy file.
 * - Restores the original owner and permissions of the privileged file.
 * - Optionally removes the copy file after overwriting.
 */
static int overwriteMode(const char *copy_file_path, const char *privileged_file_path, const bool keep_copy) {
    // Retrieve the owner of the privileged file
    uid_t prv_file_owner;
    const int own_result = getFileOwner(privileged_file_path, &prv_file_owner);
    if (own_result != SUCCESS) {
        return printError(own_result, "getting file owner");
    }

    // Retrieve the permissions of the privileged file
    mode_t prv_file_perms;
    const int perm_result = getFilePermissions(privileged_file_path, &prv_file_perms);
    if (perm_result != SUCCESS) {
        return printError(perm_result, "getting file permissions");
    }

    // Overwrite the privileged file with the copy file
    const int copy_result = copyFile(copy_file_path, privileged_file_path);
    if (copy_result != SUCCESS) {
        return printError(copy_result, "copying file");
    }

    // Restore the original owner of the privileged file
    const int chown_result = changeFileOwner(privileged_file_path, prv_file_owner);
    if (chown_result != SUCCESS) {
        return printError(chown_result, "changing file owner");
    }

    // Restore the original permissions of the privileged file
    const int ovr_perms_result = overwriteFilePermissions(privileged_file_path, prv_file_perms);
    if (ovr_perms_result != SUCCESS) {
        return printError(ovr_perms_result, "overwriting file permissions");
    }

    // Remove the copy file if the `keep_copy` flag is not set
    if (!keep_copy) {
        if (remove(copy_file_path) == -1) {
            fprintf(stderr, "Error: Failed to remove the copy file.\n");
        }
    }

    return SUCCESS;
}
