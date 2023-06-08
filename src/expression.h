#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <time.h>

enum my_expr_type
{
    EXPR_NAME = 0,
    EXPR_TYPE,
    EXPR_NEWER,
    EXPR_EXEC,
    EXPR_EXECDIR,
    EXPR_OR,
    EXPR_AND,
    EXPR_PERM,
    EXPR_USER,
    EXPR_NOT,
    EXPR_GROUP
};

struct my_expr
{
    enum my_expr_type type;
    int delete;
    int print;
    union
    {
        char *value;
        time_t time;
        struct
        {
            struct my_expr *left;
            struct my_expr *right;
        } children;
    } data;
};

char **eval_expr(struct my_expr *expr, char *path, char **args);
char **ORexec(char **t, char **t2);

#endif /* ! EXPRESSION_H */
