# redit

`redit` (root + edit) is a command-line tool for Linux, designed to simplify the process of copying, editing, and overwriting files from other users. As the name suggests, it combines root privileges and editing capabilities into a single, easy-to-use command.

![Main Graphic](img/main_graphic_white.jpg)


## About This Project

- I am new to developing programs for the Linux terminal and also to managing repositories in a semi-professional manner.
- This project is a **personal endeavor**, tailored to my own needs and preferences. As such, I cannot guarantee it will be comfortable or useful for everyone, but I hope it will be a good starting point for others with similar needs.

## Features

- Copy files with root privileges and retain original ownership and permissions.
- Overwrite files while preserving original metadata.
- Choose your preferred editor for editing copied files.

## Usage

```sh
redit -C /path/to/original/file
redit -O /path/to/edited/file
```

#### Flags

- `-C`: **Copy mode**
  - This flag initiates the copy mode. It copies the specified file to a location where it can be edited with root privileges. After copying, the file's ownership and permissions are adjusted for editing.

- `-O`: **Overwrite mode**
  - This flag starts the overwrite mode. It overwrites the original file with the edited copy, preserving the original file's ownership and permissions. This mode can optionally keep the copied file after the overwrite process if the `-k` flag is used.

- `-d`: **Copied file path**
  - Allows to indicate the file path of the copied file (ex: redit [-C or -O] /path/to/copy /path/to/original).

- `-e=[editor]`: **Editor selection**
  - Specifies the editor to use for editing the copied file. If no editor is specified, the default is `code`. It is necessary to use the equal sign attached to the flag for it to work correctly (ex: -e=vim).

- `-k`: **Keep copy**
  - This flag indicates that the copied file should be kept after overwriting the original file.

## Contributing

I am completely open to suggestions of any kind. Feel free to submit issues or pull requests. Please note that as this is a personal project, I might not be able to address all contributions immediately, but I highly appreciate your input!

## License

This project is licensed under the GNU General Public License v3.0 License.

## Contact

For any inquiries or suggestions, feel free to contact me through GitHub.
