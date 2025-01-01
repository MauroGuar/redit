# redit

`redit` (*root + edit*) is a command-line tool for Linux, designed to ensure a safe environment for copying, editing and
overwriting other user's files by simplifying and maintaining constant error control over that process. As its name suggests,
it combines secure root privilege management and editing capabilities into a single easy-to-use command.

The program allows the use of any editor of choice to edit privileged files without having to elevate the editor's privileges.\
For example, visual studio code does not allow you to edit root files as it does not allow you to run the editor as such.
In this case, redit comes very handy as it makes a copy of the privileged file and then makes it editable for the original user.
It also allows you to select the editor you want for editing that copy, with in the very same one command.

</br>

![Main Descriptive Image](/docs/img/main_graphic.png)


## Video

[![redit Tutorial Video](https://img.youtube.com/vi/G_OwVwlDGlM/0.jpg)](https://www.youtube.com/watch?v=G_OwVwlDGlM)

## Features  

- Safely copy and edit privileged files while ensuring user ownership and permissions. 
- Overwrite privileged files with copied content while preserving original metadata.  
- Open copied files directly in your preferred text editor using the `-e` flag.  
- Automatically validate and create missing directories for copy file paths.  

## Usage

The program has two main modes, "**copy**" and "**overwritte**". These modes tell the program what actions to perform on
the following path/s. You must select one to be able to use the program.

### Copy Mode
```sh
redit -C /path/to/privileged/file
```

The copy mode is designed to create an editable duplicate of a privileged file. This mode is activated using the `-C` flag,
which specifies that the program should copy the file located at the privileged file path to a designated copy file path.

The copy file path can be explicitly defined using the [`-d`](#flags) or [`-D`](#flags) flags.
If no [`-d`](#flags) or [`-D`](#flags) flag is provided, the program defaults to placing the copy file in the
current working directory under the same name as the privileged file.  

Once the privileged file is successfully copied to the copy file path, the program ensures that the copy file is editable
by the user who invoked the program (even if executed via `sudo`). This is achieved by modifying the ownership and
permissions of the copy file. Ownership is transferred to the effective user ID, and permissions are adjusted to allow reading
and writing, ensuring the file can be freely edited without restrictions.  

Finally, the program returns the absolute path to the copy file, which can be edited and modified as desired by the user.
If the [`-e`](#flags) flag is used, the program directly redirects the user to the editor with the copy open, without printing
the path to the terminal.

### Overwrite Mode
```bash
redit -O /path/to/privileged/file
```

The overwrite mode is used to securely replace a privileged file with a modified copy. This mode is activated using the `-O` flag,
which directs the program to overwrite the privileged file with the contents of the copy file.

Same as with the copy mode, the copy file path can be explicitly defined using the [`-d`](#flags) or [`-D`](#flags) flags. If no [`-d`](#flags) or [`-D`](#flags)
flag is provided, the program defaults to placing the copy file in the current working directory under the same name
as the privileged file.  

The overwrite process ensures that the privileged file retains its original ownership and permissions after the operation.
This mode is designed for scenarios where the user has edited a copy of the privileged file and wishes to apply
the changes back to the original file without compromising its attributes.

This mode automatically removes the copy which was used to overwrite the privileged file. The behaviour can be avoided
using the [`-k`](#flags) flag to keep the copy.

## Using an Editor

The program offers the option to open and edit the copy file directly in a text editor after it has been created.
This feature is enabled using the [`-e`](#flags) flag, which ensures seamless integration with the editor of your choice. By using this flag,
you can streamline the process of modifying files without manually opening them after the copy mode is executed.

### Behavior of the `-e` Flag  

The `-e` flag operates in two distinct scenarios, depending on whether a specific editor is provided:  

1. **When the Editor is Specified**:  
   If you supply a value with the `-e` flag, the program will use the specified editor to open the copy file. For example:  

   ```bash
   redit -C /privileged/privileged_file.txt -e vim
   ```  

   In this case, the program will copy the privileged file to the designated copy file path and open the copy file in the `vim` editor for immediate editing.  

2. **When the `-e` Flag is Used Without a Value**:  
   If no value is provided with the `-e` flag, the program will attempt to use the default editor specified in the `REDIT_EDITOR` environment variable. If this variable is not set, the program falls back to the default editor, which is `nano`. For example:  

   ```bash
   redit -C /privileged/privileged_file.txt -e
   ```  

   This command copies the privileged file to the designated copy file path and opens the copy file in the editor defined by `REDIT_EDITOR` or `nano` if the variable is null.  

### Using the Environment Variable

The program allows users to define their preferred default editor using the `REDIT_EDITOR` environment variable. This variable can be set to any editor of your choice, enabling a consistent editing experience across commands.  

To set the `REDIT_EDITOR` variable temporarily for the current session:  
```bash
export REDIT_EDITOR=vim
```  

To set it permanently, add the following line to your shell configuration file (e.g., `.bashrc`, `.zshrc`):  
```bash
# Set the default redit editor
export REDIT_EDITOR=vim
```  

### **Special Considerations when Using `sudo`**  

When executing the program with `sudo`, the environment variables may not be preserved, including `REDIT_EDITOR`. This can result in the default editor (`nano`) being used instead of the one specified in `REDIT_EDITOR`.  

To ensure the `REDIT_EDITOR` variable is preserved while using `sudo`, you have three options:  

1. **Using `sudo -E`**:  
   The `-E` flag instructs `sudo` to preserve the user’s environment variables, including `REDIT_EDITOR`. For example:  
   ```bash
   sudo -E redit -Ce /privileged/privileged_file.txt
   ```  

2. **Using `sudo --preserve-env=REDIT_EDITOR`**:  
   This approach explicitly preserves the `REDIT_EDITOR` variable. For example:  
   ```bash
   sudo --preserve-env=REDIT_EDITOR redit -Ce /privileged/privileged_file.txt
   ```  

3. **Editing the `sudoers` File**:  
   You can configure `sudo` to always preserve the `REDIT_EDITOR` variable by editing the `sudoers` file. Use `visudo` to safely make changes:  
   ```bash
   sudo visudo
   ```  
   Add the following line:  
   ```
   Defaults env_keep += "REDIT_EDITOR"
   ```  
   This ensures that `REDIT_EDITOR` is preserved for all `sudo` commands, eliminating the need to use `-E` or `--preserve-env`.  

## Flags

- `-C`, `--copy`: **Copy mode**
  - This flag initiates the copy mode. See [Copy Mode](#copy-mode) for more information.

- `-O`, `--overwrite`: **Overwrite mode**
  - This flag initiates the overwrite mode. See [Overwrite Mode](#overwrite-mode) for more information.

- `-d`, `--cfile`: **Copy file path**
  - Allows to indicate the file path of the copied file. If the path does not exist, it recursively creates it. Example:
  - ```bash
    redit [-C or -O] /path/to/copy/file /path/to/privileged/file
    ```
  
- `-D`, `--dfile`: **Copy file directory path**
  - Allows to indicate the directory path of the copied file. If the path does not exist, it recursively creates it. Example:
  - ```sh
    redit [-C or -O] /path/to/copy/directory /path/to/privileged/file

- `-e`, `--editor` `<editor>`: **Editor selection**
  - Specifies the editor to use for editing the copied file. See [Using an Editor](#using-an-editor) for more information.

- `-k`, `--keep`: **Keep copy**
  - Indicates that the copied file should be kept after overwriting the original file.

- `-h`, `--help`: **Help message**
  - Displays the help message.

</br>

**Note**: any path specified can be relative or absolute. Just make sure it is correct and you have the enough permissions.
