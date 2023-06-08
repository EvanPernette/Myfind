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

int morethings(int is_deleting, char *dir_path, int *fd, char *name)
{
    if (fnmatch(name, dir_path, 0) == 0)
    {
        int found = 0;
        if (is_deleting == 1)
        {
            rmdir(dir_path);
        }
        else if (is_deleting == 2)
        {
            things(fd, dir_path, NULL);
            rmdir(dir_path);
        }
        else
            found = things(fd, dir_path, NULL);
        return found;
    }
    return 0;
}

int search_dir(char *dir_path, char *name[], int *fd, int is_deleting)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        exit(1);
    }

    struct dirent *ent;
    char path[1000];
    int found = 0;

    found = morethings(is_deleting, dir_path, fd, name[2]);

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir_path, ent->d_name);

        struct stat st;
        if (stat(path, &st) == -1)
        {
            exit(1);
        }

        if (S_ISDIR(st.st_mode))
        {
            char new_path[1000];
            snprintf(new_path, sizeof(new_path), "%s/%s", dir_path, name[0]);
            name[2] = new_path;
            int subdir_prefix_len =
                snprintf(name[1] + strlen(name[1]), 1000 - strlen(name[1]),
                         "%s/", ent->d_name);
            found |= search_dir(path, name, fd, is_deleting);
            name[1][strlen(name[1]) - subdir_prefix_len] = '\0';
        }
        else if (S_ISREG(st.st_mode) && fnmatch(name[0], ent->d_name, 0) == 0)
        {
            thingsdelete(path, is_deleting);
            if (is_deleting == 0)
                found = things(fd, name[1], ent);
        }
    }

    closedir(dir);
    return 0;
}

char **find_name(char *dir, char *name, int is_deleting)
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

        char prefix[1000] = { 0 };
        snprintf(prefix, sizeof(prefix), "%s/", dir);
        char *name2[3] = { name, prefix, name };
        int found = search_dir(dir, name2, &fd[1], is_deleting);
        close(fd[1]);
        exit(found);
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
