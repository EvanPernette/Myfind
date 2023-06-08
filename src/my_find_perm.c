#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define _XOPEN_SOURCE 500
#include <fnmatch.h>
#include <sys/stat.h>

#include "my_find.h"

int search_dir_perm(char *dir_path[], long perm, int *fd, int is_deleting)
{
    DIR *dir = opendir(dir_path[0]);
    if (dir == NULL)
    {
        exit(1);
    }

    struct dirent *ent;
    char path[1000];
    int found = 0;

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir_path[0], ent->d_name);

        struct stat st;
        if (stat(path, &st) == -1)
        {
            exit(1);
        }
        if (S_ISDIR(st.st_mode)
            && (st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) == perm)
        {
            thingsdelete(dir_path[0], is_deleting);
            found = things(fd, dir_path[0], NULL);
        }

        if (S_ISDIR(st.st_mode))
        {
            int subdir_prefix_len =
                snprintf(dir_path[1] + strlen(dir_path[1]),
                         1000 - strlen(dir_path[1]), "%s/", ent->d_name);
            char *orig_path = dir_path[0];
            dir_path[0] = path;
            found |= search_dir_perm(dir_path, perm, fd, is_deleting);
            dir_path[1][strlen(dir_path[1]) - subdir_prefix_len] = '\0';
            dir_path[0] = orig_path;
        }
        else if (S_ISREG(st.st_mode)
                 && (st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) == perm)
        {
            thingsdelete(path, is_deleting);
            found = things(fd, dir_path[1], ent);
        }
    }

    closedir(dir);
    return found;
}

char **find_perm(char *dir, char *mode, int is_deleting)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        exit(1);
    }

    long perm = strtol(mode, NULL, 8);
    if (perm == 0)
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

        char prefix[1000] = { 0 };
        snprintf(prefix, sizeof(prefix), "%s/", dir);
        char *dir_path[2] = { dir, prefix };
        int found = search_dir_perm(dir_path, perm, &fd[1], is_deleting);
        close(fd[1]);
        exit(found ? 0 : 1);
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
