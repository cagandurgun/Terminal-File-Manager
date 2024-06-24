/* 
 * This application has been written by Çağan Durgun. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_MENU_SIZE 50 // Increased for more items to be added

static struct termios old, new;

// Struct for filesystem objects (directories and files)
struct FileSystemObject {
    char name[256]; // Name of directory or file (limited size)
    int isDirectory; // Is it a directory? (1) Is it a file? (0)
};

// Function to create a directory
void createDirectory(const char *dirname);

// Function to create a file
void createFile(const char *filename);

// Function to delete a filesystem object (directory or file)
void deleteFileSystemObject(const char *path);

// Function to list contents of a directory
void listDirectory(const char *path, const char *program_name, const char **menu_items, int *menu_size);

void initTermios();

void resetTermios();

char getch();

void print_menu(const char *menu_items[], int menu_size, int selected);

void add_menu_item(const char *new_item, const char **menu_items, int *menu_size, int *selected);

// Function to display help menu
void showHelp();

void changeDirectory(const char *path);

int main(int argc, char *argv[]) {
    const char *menu_items[MAX_MENU_SIZE];
    int menu_size = 0;
    int selected = 0;

    char c;

    if (argc < 1) {
        fprintf(stderr, "Unable to retrieve program name.\n");
        return 1;
    }

    while (1) {
        listDirectory(".", argv[0], menu_items, &menu_size); // List contents of current directory

        print_menu(menu_items, menu_size, selected);

        c = getch();

        if (c == '\033') {
            getch(); // Skip the "[" character
            c = getch(); // Get A, B, C, or D character

            if (c == 'A' && selected > 0) {
                selected--;
            } else if (c == 'B' && selected < menu_size - 1) {
                selected++;
            } else if (c == 'C' && selected >= 0 && selected < menu_size) { // Right arrow
                struct stat statbuf;
                if (stat(menu_items[selected], &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                    changeDirectory(menu_items[selected]);
                    selected = 0; // After changing directory, select the first item
                }
            } else if (c == 'D') { // Left arrow
                changeDirectory("..");
                selected = 0; // After going up directory, select the first item
            }
        } else if (c == 'o' || c == 'O') {
            printf("Enter the name of the directory you want to create: ");
            char new_item[100];
            scanf("%99s", new_item);

            createDirectory(new_item); // Create a new directory

            // Refresh directory listing to include the new item
            listDirectory(".", argv[0], menu_items, &menu_size);

            // Newly added item should be selected
            selected = menu_size - 1;
        } else if (c == 'p' || c == 'P') {
            printf("Enter the name of the file you want to create: ");
            char new_file[100];
            scanf("%99s", new_file);

            createFile(new_file); // Create a new file

            // Refresh directory listing to include the new item
            listDirectory(".", argv[0], menu_items, &menu_size);

            // Newly added item should be selected
            selected = menu_size - 1;
        } else if (c == 's' || c == 'S') {
            if (selected >= 0 && selected < menu_size) {
                printf("Are you sure you want to delete '%s'? (Y/N): ", menu_items[selected]);
                char confirm;
                scanf(" %c", &confirm);
                if (confirm == 'Y' || confirm == 'y') {
                    // Perform deletion
                    printf("Deleting '%s'...\n", menu_items[selected]);
                    deleteFileSystemObject(menu_items[selected]);

                    // Remove the deleted item from the menu
                    free((void *)menu_items[selected]);
                    for (int i = selected; i < menu_size - 1; ++i) {
                        menu_items[i] = menu_items[i + 1];
                    }
                    menu_items[menu_size - 1] = NULL;
                    menu_size--;

                    // Update selected item
                    if (selected >= menu_size) {
                        selected = menu_size - 1;
                    }
                } else {
                    printf("Deletion cancelled.\n");
                }
            } else {
                printf("Invalid selection, please choose an item from the menu.\n");
            }
        } else if (c == 'y' || c == 'Y') {
            showHelp(); // Show help menu
        } else if (c == 'x' || c == 'X') {
            printf("Exiting...\n");
            sleep(2); // Wait for 2 seconds
            system("clear");
            break; // Exit the program
        }
    }

    // Free allocated memory for menu items
    for (int i = 0; i < menu_size; ++i) {
        free((void *)menu_items[i]);
    }

    return 0;
}



// Function to create a directory
void createDirectory(const char *dirname) {
    if (mkdir(dirname, 0777) == 0) {
        printf("Created a new directory named '%s'.\n", dirname);
    } else {
        perror("mkdir");
    }
}

// Function to create a file
void createFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file) {
        printf("Created a new file named '%s'.\n", filename);
        fclose(file);
    } else {
        perror("fopen");
    }
}

// Function to delete a filesystem object (directory or file)
void deleteFileSystemObject(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        perror("stat");
        return;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        if (rmdir(path) == 0) {
            printf("Deleted directory '%s'.\n", path);
        } else {
            perror("rmdir");
        }
    } else {
        if (remove(path) == 0) {
            printf("Deleted file '%s'.\n", path);
        } else {
            perror("remove");
        }
    }
}

// Function to list contents of a directory
void listDirectory(const char *path, const char *program_name, const char **menu_items, int *menu_size) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }

    *menu_size = 0; // Reset menu size

    // Trim "./" from program_name
    const char *program_name_trimmed = program_name;
    if (strncmp(program_name, "./", 2) == 0) {
        program_name_trimmed = program_name + 2;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Exclude "." and ".." folders
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Ignore files starting with "."
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Get file or directory name and type
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        // Get stat information of the file or directory
        if (stat(filepath, &statbuf) == -1) {
            perror("stat");
            closedir(dir);
            return;
        }

        // List only directories and regular files (ignore symbolic links)
        if (S_ISDIR(statbuf.st_mode) || S_ISREG(statbuf.st_mode)) {
            // Check file/directory name, add if not the same as program_name_trimmed
            if (strcmp(entry->d_name, program_name_trimmed) != 0) {
                menu_items[*menu_size] = strdup(entry->d_name);
                (*menu_size)++;
                if (*menu_size >= MAX_MENU_SIZE) {
                    printf("Maximum menu size reached, additional items will not be shown.\n");
                    break;
                }
            }
        }
    }

    closedir(dir);
}

void initTermios() {
    tcgetattr(0, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new);
}

void resetTermios() {
    tcsetattr(0, TCSANOW, &old);
}

char getch() {
    char ch;
    initTermios();
    ch = getchar();
    resetTermios();
    return ch;
}

void print_menu(const char *menu_items[], int menu_size, int selected) {
    system("clear");

    // Get current directory name
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\npress [Y] for help\n\n", cwd);
    } else {
        perror("getcwd");
    }

    for (int i = 0; i < menu_size; ++i) {
        if (i == selected) {
            printf("  %s\n", menu_items[i]); // Show selected item
        } else {
            printf("%s\n", menu_items[i]);
        }
    }
}

void add_menu_item(const char *new_item, const char **menu_items, int *menu_size, int *selected) {
    if (*menu_size < MAX_MENU_SIZE) {
        menu_items[*menu_size] = strdup(new_item);
        (*menu_size)++;
        *selected = *menu_size - 1;  // Newly added item should be selected
    } else {
        printf("Menu is at maximum capacity, cannot add new item.\n");
    }
}

// Function to display help menu
void showHelp() {
    system("clear");
    printf("Help Menu:\n");
    printf(" - Up/Down arrow keys: Navigate through menu.\n");
    printf(" - 'o': Create a new directory.\n");
    printf(" - 'p': Create a new file.\n");
    printf(" - 's': Delete the selected directory or file.\n");
    printf(" - 'y': Show this help menu.\n");
    printf(" - 'x': Exit the program.\n");
    printf("\nPress any key to continue...\n");
    getch();
}

void changeDirectory(const char *path) {
    if (chdir(path) != 0) {
        perror("chdir");
    }
}

