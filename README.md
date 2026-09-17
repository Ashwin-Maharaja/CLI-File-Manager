# CLI File Manager

A command-line file manager built using C++17 and the Standard Library.

## Features

- `ls` - List files and folders
- `cd` - Change directory
- `pwd` - Show current directory
- `mkdir` - Create a folder
- `touch` - Create a file
- `rm` - Remove a file or folder
- `rename` - Rename a file or folder
- `cp` - Copy a file
- `tree` - Display directory tree
- `search` - Search for files and folders
- `reindex` - Rebuild the file index
- `help` - Show available commands
- `exit` - Exit the program

## Features by Level

### Level 1 - Colored Terminal UI

- Colored directories and files
- Colored success and error messages

### Level 2 - Directory Tree

- Recursive directory traversal
- Unicode tree visualization

### Level 3 - File Search

- Recursive file and folder search

### Level 4 - Command Piping

- `ls | search`
- `tree | search`

### Level 5 - Command Parser

- Command parsing
- Quoted arguments
- Quote validation
- Argument validation
- Command registry

### Level 6 - File Indexing

- File indexing using `std::filesystem`
- Indexed search
- Background indexing using `std::thread`
- Thread-safe index using `std::mutex`
- Automatic index updates after file operations
- Manual index rebuilding using `reindex`

## Technologies

- C++17
- STL
- `std::filesystem`
- `std::thread`
- `std::mutex`

## Build

Compile using:

```bash
g++ -std=c++17 main.cpp -o filemanager
```

Run:

```bash
./filemanager
```

## Example

```text
===== CLI FILE MANAGER =====

D:\Git Projects\CLI File Manager > ls
[DIR]  test_data
[FILE] main.cpp
[FILE] README.md

D:\Git Projects\CLI File Manager > search main
D:\Git Projects\CLI File Manager\main.cpp

D:\Git Projects\CLI File Manager > exit
Exiting...