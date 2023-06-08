#include <dirent.h>
#include <err.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "my_find.h"

char **allocfd(int fd, char buf[])
{
    int printingtime = 0;
    char **args = malloc(200 * sizeof(char *));
    for (int i = 0; i < 200; i++)
    {
        args[i] = NULL;
    }
    args[0] = malloc(100 * sizeof(char));
    int i = 0;
    int i2 = 0;
    while ((printingtime = read(fd, buf, 1)) > 0)
    {
        if (buf[0] == '\n')
        {
            args[i][i2] = '\0';

            i++;
            args[i] = malloc(100 * sizeof(char));
            i2 = 0;
        }
        else
        {
            args[i][i2] = buf[0];
            i2++;
        }
    }
    free(args[i]);
    args[i] = NULL;
    return args;
}

int search_dir_group(char *dir_path[], gid_t gid, int *fd, int is_deleting)
{
    DIR *dir = opendir(dir_path[0]);
    if (dir == NULL)
    {
        exit(1);
    }

    struct dirent *ent;
    char path[1000];
    int found = 0;

    struct stat st;
    if (stat(dir_path[0], &st) == -1)
    {
        exit(1);
    }
    if (S_ISDIR(st.st_mode) && (st.st_gid == gid))
    {
        thingsdelete(dir_path[0], is_deleting);
        found = things(fd, dir_path[0], NULL);
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir_path[0], ent->d_name);

        if (stat(path, &st) == -1)
        {
            exit(1);
        }

        if (S_ISDIR(st.st_mode))
        {
            int subdir_prefix_len =
                snprintf(dir_path[1] + strlen(dir_path[1]),
                         1000 - strlen(dir_path[1]), "%s/", ent->d_name);
            char *orig_path = dir_path[0];
            dir_path[0] = path;
            found |= search_dir_group(dir_path, gid, fd, is_deleting);
            dir_path[1][strlen(dir_path[1]) - subdir_prefix_len] = '\0';
            dir_path[0] = orig_path;
        }
        else if (S_ISREG(st.st_mode) && st.st_gid == gid)
        {
            thingsdelete(path, is_deleting);
            found = things(fd, dir_path[1], ent);
        }
    }

    closedir(dir);
    return found;
}

char **find_group(char *dir, char *gid1, int is_deleting)
{
    struct group *grp = getgrnam(gid1);
    if (grp == NULL)
    {
        exit(1);
    }
    gid_t gid = grp->gr_gid;
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
        char *dir_path[2] = { dir, prefix };
        int found = search_dir_group(dir_path, gid, &fd[1], is_deleting);
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
