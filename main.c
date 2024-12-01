#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <bits/getopt_core.h>

#include "file_utils.h"
#include "file_operations.h"

#define COPY_BUF_SIZE 4096

int main(int argc, char *argv[]) {
    int opt;
    bool copy_mode = false;
    bool overwrite_mode = false;

    while ((opt == getopt(argc, argv, "CO")) != -1) {
        switch (opt) {
            case 'C':
                copy_mode = 1;
                break;
            case 'O':
                overwrite_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s -C /path/to/file /path/to/destination\n", argv[0]);
                fprintf(stderr, "       %s -O /path/to/edited/file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}
