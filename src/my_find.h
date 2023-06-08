#ifndef MY_FIND_H
#define MY_FIND_H

char **find_name(char *dir, char *name, int is_deleting);
char **find_type(char *dir, char type, int is_deleting);
int simple_find(char *dir_path);
char **find_newer(char *dir, char *name, int is_deleting);
char **find_perm(char *dir, char *mode, int is_deleting);
char **find_user(char *dir, char *user, int is_deleting);
char **find_group(char *dir, char *group, int is_deleting);
int thingsdelete(char *path, int is_deleting);
int things(int *fd, char *dir_path, struct dirent *ent);
char **allocfd(int fd, char buf[]);

#endif
