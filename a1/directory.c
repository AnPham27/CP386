#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
    #include "dirent.h"
    #define mkdir(dirname, mode) _mkdir(dirname)
#endif

DIR *current_dir = NULL; 

void create_directory() {
    char dirname[100];
    printf("Enter directory name to create: ");
    scanf("%s", dirname);

    if (mkdir(dirname, 0777) == 0) {
        printf("Directory '%s' created successfully.\n", dirname);
    } else {
        perror("Error creating directory");
    }
}

void remove_directory() {
    char dirname[100];
    printf("Enter directory name to remove: ");
    scanf("%s", dirname);

    if (rmdir(dirname) == 0) {
        printf("Directory '%s' removed successfully.\n", dirname);
    } else {
        perror("Error removing directory");
    }
}

void print_current_directory() {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("Error getting current working directory");
    }
}

void change_directory_up() {
    if (chdir("..") == 0) {
        printf("Changed directory one level up successfully.\n");
    } else {
        perror("Error changing directory");
    }
}

void read_directory_contents() {
    char dirname[256];
    printf("Enter directory name to read: ");
    scanf("%s", dirname);

    DIR *d;
    struct dirent *dir;
    d = opendir(dirname);
    if (d) {
        printf("Contents of directory '%s':\n", dirname);
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    } else {
        perror("Error reading directory");
    }
}

void close_current_directory() {
    if (current_dir) {
        closedir(current_dir);
        current_dir = NULL;
        printf("Current directory closed successfully.\n");
    } else {
        printf("No directory is currently open.\n");
    }
}

void display_menu() {
    printf("\nSelect the option(s) appropriately by entering the number:\n");
    printf("1. Create a directory\n");
    printf("2. Remove a directory\n");
    printf("3. Print current working directory\n");
    printf("4. Change directory one level up\n");
    printf("5. Read the contents of the directory\n");
    printf("6. Close the current directory\n");
    printf("q. Exit the program\n");
   
}

int main() {
    char choice;
    while (1) {
        display_menu();
        scanf(" %c", &choice);
        
        switch (choice) {
            case '1':
                create_directory();
                break;
            case '2':
                remove_directory();
                break;
            case '3':
                print_current_directory();
                break;
            case '4':
                change_directory_up();
                break;
            case '5':
                read_directory_contents();
                break;
            case '6':
                close_current_directory();
                break;
            case 'q':
                printf("Exiting program.\n");
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}