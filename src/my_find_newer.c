#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "my_find.h"

int search_dir_newer(char *dir_path, char *ref_file_path, int *fd,
                     int is_deleting)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        exit(1);
    }
    int test = 0;
    int test2 = 0;
    while (dir_path[test] != '\0')
    {
        test++;
        if (dir_path[test] == '/')
        {
            test2 = 1;
        }
    }
    if (test2 == 0)
    {
        thingsdelete(dir_path, is_deleting);
        things(fd, dir_path, NULL);
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
        struct stat s2;
        if (stat(path, &s) == -1)
        {
            exit(1);
        }
        if (stat(ref_file_path, &s2) == -1)
        {
            exit(1);
        }
        if (S_ISDIR(s.st_mode))
        {
            search_dir_newer(path, ref_file_path, fd, is_deleting);
        }
        else
        {
            if (s.st_mtime >= s2.st_mtime)
            {
                if (strcmp(path, ref_file_path) == 0)
                {
                    continue;
                }
                if (is_deleting == 1)
                {
                    int status = unlink(path);
                    if (status != 0)
                    {
                        errx(1, "myfind: ");
                    }
                }
                else
                {
                    write(*fd, path, strlen(path));
                    write(*fd, "\n", 1);
                }
            }
        }
    }
    closedir(dir);
    return 0;
}

char **find_newer(char *dir, char *name, int is_deleting)
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
        search_dir_newer(dir, name, &fd[1], is_deleting);
        close(fd[1]);
        exit(0);
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
            return NULL;
        }
    }
}
