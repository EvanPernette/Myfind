#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "my_find.h"

int simple_find(char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        return 1;
    }
    printf("%s\n", dir_path);
    struct dirent *ent;
    char path[1000];

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, ent->d_name);

        struct stat s;
        if (stat(path, &s) == -1)
        {
            exit(1);
        }

        if (S_ISDIR(s.st_mode))
        {
            simple_find(path);
        }
        else
        {
            printf("%s\n", path);
        }
    }
    closedir(dir);
    return 0;
}
