#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_FILES 100
#define MAX_FILENAME 256

typedef struct {
    char c_file[MAX_FILENAME];
    char h_file[MAX_FILENAME];
} SourceFile;

int main() {
    DIR *dir;
    struct dirent *entry;
    SourceFile files[MAX_FILES];
    char headers[MAX_FILES][MAX_FILENAME];
    int file_count = 0, header_count = 0;
    char main_file[MAX_FILENAME] = {0};
    const char *self_file = "autoMake.c"; // Numele fișierului generator

    // Deschide directorul curent
    dir = opendir(".");
    if (dir == NULL) {
        perror("Nu s-a putut deschide directorul curent");
        return 1;
    }

    // Parcurge fișierele din director
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".c") != NULL) {
            if (strcmp(entry->d_name, "main.c") == 0) {
                strncpy(main_file, entry->d_name, MAX_FILENAME);
            } else if (strcmp(entry->d_name, self_file) != 0) {
                strncpy(files[file_count].c_file, entry->d_name, MAX_FILENAME);
                files[file_count].h_file[0] = '\0'; // Inițial fără header
                file_count++;
                if (file_count >= MAX_FILES) {
                    fprintf(stderr, "Prea multe fișiere sursă!\n");
                    break;
                }
            }
        } else if (strstr(entry->d_name, ".h") != NULL) {
            strncpy(headers[header_count], entry->d_name, MAX_FILENAME);
            header_count++;
        }
    }
    closedir(dir);

    if (strlen(main_file) == 0) {
        fprintf(stderr, "Nu s-a găsit main.c în director.\n");
        return 1;
    }

    // Asociază fișiere header cu fișiere sursă
    for (int i = 0; i < file_count; i++) {
        char h_file[MAX_FILENAME];
        strncpy(h_file, files[i].c_file, MAX_FILENAME);
        h_file[strlen(h_file) - 2] = '\0'; // Elimină extensia .c
        strcat(h_file, ".h");
        for (int j = 0; j < header_count; j++) {
            if (strcmp(h_file, headers[j]) == 0) {
                strncpy(files[i].h_file, h_file, MAX_FILENAME);
                break;
            }
        }
    }

    // Creează fișierul Makefile
    FILE *makefile = fopen("Makefile", "w");
    if (makefile == NULL) {
        perror("Nu s-a putut crea fișierul Makefile");
        return 1;
    }

    // Scrie target-ul principal
    fprintf(makefile, "CC = clang\n");
    fprintf(makefile, "CFLAGS = -Wall\n\n");

    fprintf(makefile, "OBJS =");
    for (int i = 0; i < file_count; i++) {
        char obj_file[MAX_FILENAME];
        snprintf(obj_file, sizeof(obj_file), "%s", files[i].c_file);
        obj_file[strlen(obj_file) - 2] = '\0'; // Elimină extensia .c
        fprintf(makefile, " %s.o", obj_file);
    }
    fprintf(makefile, " main.o\n\n");

    fprintf(makefile, "TARGET = main\n\n");

    // Regula pentru compilarea finală
    fprintf(makefile, "all: $(TARGET)\n\n");
    fprintf(makefile, "$(TARGET): $(OBJS)\n");
    fprintf(makefile, "\t$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)\n\n");

    // Reguli pentru compilare individuală
    for (int i = 0; i < file_count; i++) {
        char obj_file[MAX_FILENAME];
        snprintf(obj_file, sizeof(obj_file), "%s", files[i].c_file);
        obj_file[strlen(obj_file) - 2] = '\0'; // Elimină extensia .c

        fprintf(makefile, "%s.o: %s", obj_file, files[i].c_file);
        if (strlen(files[i].h_file) > 0) {
            fprintf(makefile, " %s", files[i].h_file);
        }
        for (int j = 0; j < header_count; j++) {
            fprintf(makefile, " %s", headers[j]);
        }
        fprintf(makefile, "\n\t$(CC) -c %s\n\n", files[i].c_file);
    }

    // Regula pentru main.o
    fprintf(makefile, "main.o: main.c");
    for (int i = 0; i < header_count; i++) {
        fprintf(makefile, " %s", headers[i]);
    }
    fprintf(makefile, "\n\t$(CC) -c main.c\n\n");

    // Regula pentru curățare
    fprintf(makefile, "clean:\n");
    fprintf(makefile, "\trm -f $(OBJS) $(TARGET)\n");

    fclose(makefile);
    printf("Fișierul Makefile a fost generat cu succes, excluzând %s!\n", self_file);

    return 0;
}
