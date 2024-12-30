#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/file_operations.h"
#include "../include/file_utils.h"
#include "../include/error_handler.h"

static int copyMode(const char *copy_file_path, const char *privileged_file_path, const char *editor, bool use_editor,
                    const char *program_default_editor);

static int overwriteMode(const char *copy_file_path, const char *privileged_file_path, bool keep_copy);

int executeFileMode(const bool is_copy, const char *copy_file_path, const char *privileged_file_path,
                    const bool keep_copy,
                    const char *editor,
                    const bool use_editor, const char *program_default_editor) {
    if (is_copy) {
        return copyMode(copy_file_path, privileged_file_path, editor, use_editor, program_default_editor);
    }
    return overwriteMode(copy_file_path, privileged_file_path, keep_copy);
}

static int copyMode(const char *copy_file_path, const char *privileged_file_path, const char *editor,
                    const bool use_editor,
                    const char *program_default_editor) {
    uid_t user_ef_id;
    const int uid_result = getEffectiveUserId(&user_ef_id);
    if (uid_result != SUCCESS) { return printError(uid_result, "getting effective user id"); }

    const int copy_result = copyFile(privileged_file_path, copy_file_path);
    if (copy_result != SUCCESS) { return printError(copy_result, "copying file"); }

    const int chown_result = changeFileOwner(copy_file_path, user_ef_id);
    if (chown_result != SUCCESS) { return printError(chown_result, "changing file owner"); }

    const mode_t new_perms = S_IRUSR | S_IWUSR;
    const int add_perms_result = addFilePermissions(copy_file_path, new_perms);
    if (add_perms_result != SUCCESS) { return printError(add_perms_result, "adding file permissions"); }

    if (use_editor) {
        const int editor_result = executeEditorCommand(editor, copy_file_path, program_default_editor);
        switch (editor_result) {
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
        printf("%s\n", copy_file_path);
    }

    return SUCCESS;
}

static int overwriteMode(const char *copy_file_path, const char *privileged_file_path, const bool keep_copy) {
    uid_t prv_file_owner;
    const int own_result = getFileOwner(privileged_file_path, &prv_file_owner);
    if (own_result != SUCCESS) { return printError(own_result, "getting file owner"); }

    mode_t prv_file_perms;
    const int perm_result = getFilePermissions(privileged_file_path, &prv_file_perms);
    if (perm_result != SUCCESS) { return printError(perm_result, "getting file permissions"); }

    const int copy_result = copyFile(copy_file_path, privileged_file_path);
    if (copy_result != SUCCESS) { return printError(copy_result, "copying file"); }

    const int chown_result = changeFileOwner(privileged_file_path, prv_file_owner);
    if (chown_result != SUCCESS) { return printError(chown_result, "changing file owner"); }

    const int ovr_perms_result = overwriteFilePermissions(privileged_file_path, prv_file_perms);
    if (ovr_perms_result != SUCCESS) { return printError(ovr_perms_result, "overwriting file permissions"); }

    if (!keep_copy) {
        if (remove(copy_file_path) == -1) {
            fprintf(stderr, "Error: Failed to remove the copy file.\n");
        }
    }

    return SUCCESS;
}
