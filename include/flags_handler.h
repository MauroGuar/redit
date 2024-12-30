#ifndef FLAGS_HANDLER_H
#define FLAGS_HANDLER_H

typedef struct {
    bool copy_mode;
    bool overwrite_mode;
    bool copied_file_path;
    bool copied_dir_path;
    bool use_editor;
    bool keep_copy;
    const char *editor;
    int param_index;
} flag_state_t;

int handleFlags(int argc, char *argv[], flag_state_t *flags);

#endif
