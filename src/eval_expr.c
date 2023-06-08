#include <dirent.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "expression.h"
#include "my_find.h"
#include "parser.h"
#include "string.h"

char **alloc_args(void)
{
    char **args = malloc(100 * sizeof(char *));
    for (int i = 0; i < 100; i++)
    {
        args[i] = NULL;
    }
    return args;
}

void free_args(char **args)
{
    for (int i = 0; i < 100; i++)
    {
        if (args[i] != NULL)
        {
            free(args[i]);
        }
    }
    free(args);
}

void add_to_prints(char **args, char **prints, struct my_expr *expr)
{
    if (args != NULL && expr->print == 1)
    {
        int j = 0;
        for (j = 0; prints[j] != NULL; j++)
        {}

        for (int i = 0; args[i] != NULL; i++)
        {
            prints[j] = malloc(100 * sizeof(char));
            strcpy(prints[j], args[i]);
            j++;
        }
    }
}

char **ORexec(char **t, char **t2)
{
    int count = 0;
    char **args = alloc_args();

    for (int i = 0; t[i] != NULL; i++)
    {
        args[count] = malloc(100 * sizeof(char));
        strcpy(args[count], t[i]);
        count++;
    }

    int test = 0;
    for (int k = 0; t2[k] != NULL; k++)
    {
        test = 0;
        for (int j = 0; args[j] != NULL; j++)
        {
            if (strcmp(t2[k], args[j]) == 0)
            {
                test = 1;
            }
        }
        if (test == 0)
        {
            args[count] = malloc(100 * sizeof(char));
            strcpy(args[count], t2[k]);
            count++;
        }
    }

    free_args(t);
    free_args(t2);

    return args;
}

char **NOTexec(char **t, char **t2)
{
    int count = 0;
    char **args = alloc_args();

    for (int i = 0; t2[i] != NULL; i++)
    {
        int found = 0;
        for (int j = 0; t[j] != NULL; j++)
        {
            if (strcmp(t2[i], t[j]) == 0)
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            args[count] = malloc(100 * sizeof(char));
            strcpy(args[count], t2[i]);
            count++;
        }
    }

    free_args(t);
    free_args(t2);

    return args;
}

char **ANDexec(char **t, char **t2)
{
    int count = 0;
    char **args = alloc_args();

    for (int i = 0; t[i] != NULL; i++)
    {
        for (int j = 0; t2[j] != NULL; j++)
        {
            if (strcmp(t[i], t2[j]) == 0)
            {
                args[count] = malloc(100 * sizeof(char));
                strcpy(args[count], t[i]);
                count++;
            }
        }
    }

    free_args(t);
    free_args(t2);

    return args;
}

char **ANDbehavior(struct my_expr *expr, char *path, char **prints)
{
    char **args;
    if (expr->data.children.left == NULL || expr->data.children.right == NULL)
    {
        errx(1, "Invalid expression");
        return NULL;
    }
    if (expr->data.children.left->type == EXPR_AND
        && expr->data.children.right->type == EXPR_AND)
    {
        errx(1, "Invalid expression");
        return NULL;
    }
    else if (expr->data.children.left->type == EXPR_AND
             && expr->data.children.right->type == EXPR_OR)
    {
        errx(1, "Invalid expression");
        return NULL;
    }
    else if (expr->data.children.left->type == EXPR_OR
             && expr->data.children.right->type == EXPR_AND)
    {
        errx(1, "Invalid expression");
        return NULL;
    }
    else if (expr->data.children.left->type == EXPR_OR
             && expr->data.children.right->type == EXPR_OR)
    {
        errx(1, "Invalid expression");
        return NULL;
    }
    if (expr->data.children.left->type == EXPR_NOT)
    {
        args = eval_expr(expr->data.children.right, path, prints);
        args = NOTexec(args, find_name(path, "*", 0));
    }
    else
    {
        args = ANDexec(eval_expr(expr->data.children.left, path, prints),
                       eval_expr(expr->data.children.right, path, prints));
    }
    return args;
}

char **eval_expr(struct my_expr *expr, char *path, char **prints)
{
    if (expr == NULL)
    {
        return NULL;
    }

    char **args;

    if (expr->type == EXPR_NAME)
    {
        args = find_name(path, expr->data.value, expr->delete);
    }
    else if (expr->type == EXPR_TYPE)
    {
        args = find_type(path, expr->data.value[0], expr->delete);
    }
    else if (expr->type == EXPR_NEWER)
    {
        args = find_newer(path, expr->data.value, expr->delete);
    }
    else if (expr->type == EXPR_USER)
    {
        args = find_user(path, expr->data.value, expr->delete);
    }
    else if (expr->type == EXPR_GROUP)
    {
        args = find_group(path, expr->data.value, expr->delete);
    }
    else if (expr->type == EXPR_PERM)
    {
        args = find_perm(path, expr->data.value, expr->delete);
    }
    else if (expr->type == EXPR_OR)
    {
        args = ORexec(eval_expr(expr->data.children.right, path, prints),
                      eval_expr(expr->data.children.left, path, prints));
    }
    else if (expr->type == EXPR_AND)
    {
        args = ANDbehavior(expr, path, prints);
    }
    else
    {
        args = NULL;
    }

    add_to_prints(args, prints, expr);
    return args;
}
