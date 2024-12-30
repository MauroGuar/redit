#ifndef FILE_MODES_H
#define FILE_MODES_H

#include <stdbool.h>

int executeFileMode(const bool is_copy, const char *copy_file_path, const char *privileged_file_path, bool keep_copy,
                    const char *editor, bool use_editor, const char *program_default_editor);

#endif
