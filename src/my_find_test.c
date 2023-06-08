#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>

#include "my_find.h"

int check_file_type(char type, struct stat s)
{
    int is_match;

    if (type == 'b')
    {
        is_match = S_ISBLK(s.st_mode);
    }
    else if (type == 'c')
    {
        is_match = S_ISCHR(s.st_mode);
    }
    else if (type == 'd')
    {
        is_match = S_ISDIR(s.st_mode);
    }
    else if (type == 'f')
    {
        is_match = S_ISREG(s.st_mode);
    }
    else if (type == 'l')
    {
        is_match = S_ISLNK(s.st_mode);
    }
    else if (type == 'p')
    {
        is_match = S_ISFIFO(s.st_mode);
    }
    else
    {
        exit(1);
        return 1;
    }

    return is_match;
}

int search_dir2(char *dir_path, char type, int *fd, int is_deleting)
{
    DIR *dir = opendir(dir_path);
    int test = 0;
    if (dir == NULL)
    {
        exit(1);
    }
    if (type == 'd')
    {
        thingsdelete(dir_path, is_deleting);
        things(fd, dir_path, NULL);
        test = 1;
    }
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
        int is_match = check_file_type(type, s);

        if (is_match && test == 0)
        {
            write(*fd, path, strlen(path));
            write(*fd, "\n", 1);
        }
        if (S_ISDIR(s.st_mode))
        {
            search_dir2(path, type, fd, is_deleting);
        }
        thingsdelete(path, is_deleting);
    }
    closedir(dir);
    return 0;
}

char **find_type(char *dir, char type, int is_del)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        exit(1);
    }
    else if (pid == 0)
    {
        close(fd[0]);
        int x = search_dir2(dir, type, &fd[1], is_del);

        close(fd[1]);
        exit(x);
    }
    else
    {
        close(fd[1]);
        char buf[1000];
        char **args = allocfd(fd[0], buf);
        close(fd[0]);
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            return args;
        }
        else
        {
            errx(1, "Invalid type");
            return NULL;
        }
    }
}
