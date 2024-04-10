#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void list_directories(const char *path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if ((dir = opendir(path)) == NULL)
    {
        perror("Ошибка открытия каталога");
        exit(EXIT_FAILURE);
    }

    printf("Содержимое каталога %s:\n", path);
    while ((entry = readdir(dir)) != NULL)
    {
        char full_path[1024];
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        if (stat(full_path, &statbuf) == -1)
        {
            perror("Ошибка получения информации о файле");
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(statbuf.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            DIR *subdir = opendir(full_path);
            struct dirent *subentry;
            int subdirectories_count = 0;

            while ((subentry = readdir(subdir)) != NULL)
                if (subentry->d_type == DT_DIR && strcmp(subentry->d_name, ".") != 0 && strcmp(subentry->d_name, "..") != 0)
                {
                    subdirectories_count++;
                    break;
                }

            closedir(subdir);

            if (subdirectories_count > 0)
                printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Неверное число аргументов: %d\n", argc);
        exit(EXIT_FAILURE);
    }

    list_directories(argv[1]);

    return 0;
}
