#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "file_utils.h"
#include "file_operations.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        
    }
    // TODO make argv verification

    const char *CWD = getCurrentWorkingDirectory();
    const char *ABS_PATH = getAbsolutePath(argv[1]);
    const char *SRC_FILE_NAME = basename(ABS_PATH);
    const char *DEST_FILE_PATH = strcat(strcat(CWD, "/"), SRC_FILE_NAME);
    const uid_t USER_EF_ID = getEffectiveUserId();
    const mode_t ORIGINAL_PERMS = getFilePermissions(ABS_PATH);

    // copyFile(argv[1], DEST_FILE_PATH);

    free(CWD);
    free(ABS_PATH);
}