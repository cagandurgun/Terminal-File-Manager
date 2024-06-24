# Terminal File Manager

## Features

- **List Directory Contents:** Navigate through directories and display files and subdirectories.
- **Create Directory:** Create a new directory within the current working directory.
- **Create File:** Generate a new file within the current working directory.
- **Delete File or Directory:** Remove selected files or directories (non-recursive).
- **Help Menu:** View a help menu with usage instructions.
- **Exit Program:** Terminate the program and return to the shell.

## Getting Started

### Prerequisites

- C compiler (e.g., gcc)
- Unix-like operating system (Linux, macOS)

### Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd file-directory-management
   ```
Compile the program:
```bash
gcc AytarOS.c -o aytar
```
Usage
Run the program:

bash
```bash
./aytar
```
Interface Navigation
Arrow keys (Up/Down): Navigate through the menu items.
Right arrow (->): Enter a directory.
Left arrow (<-): Go up one directory.
'o': Create a new directory.
'p': Create a new file.
's': Delete the selected directory or file.
'y': Show the help menu.
'x': Exit the program.
Contributing
Contributions are welcome! Please fork the repository and submit a pull request.
