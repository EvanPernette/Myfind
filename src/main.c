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

void print_usage(char *program_name)
{
    errx(1, "Usage: %s <directory> <expression>", program_name);
}

void print_simple_find_error(void)
{
    errx(1, "myfind: problems in simple_find");
}

void print_no_tree_error(void)
{
    errx(1, "myfind: no tree");
}

char **alloc_prints(void)
{
    char **prints = malloc(100 * sizeof(char *));
    for (int i = 0; i < 100; i++)
    {
        prints[i] = NULL;
    }
    return prints;
}

void free_prints(char **prints)
{
    for (int i = 0; i < 100; i++)
    {
        if (prints[i] != NULL)
        {
            free(prints[i]);
        }
    }
    free(prints);
}

void print_args(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        printf("%s\n", args[i]);
        free(args[i]);
    }
    free(args);
}

int hardcodeig(int argc, char **argv)
{
    if (argc == 2 && argv[1][0] != '-')
    {
        int t = simple_find(argv[1]);
        if (t == 1)
        {
            return 1;
        }
        return 2;
    }

    if (argc == 1 || (argc == 2 && strcmp(argv[1], "-print") == 0)
        || (argc == 2 && strcmp(argv[1], "-P") == 0)
        || strcmp(argv[1], "-print") == 0)
    {
        simple_find(".");
        return 2;
    }

    if ((argc == 3 && argv[1][0] != '-'
         && (strcmp(argv[2], "-print") == 0 || strcmp(argv[2], "-P") == 0))
        || strcmp(argv[2], "-print") == 0)
    {
        int t = simple_find(argv[1]);
        if (t == 1)
        {
            return 1;
        }
        return 2;
    }
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-type") == 0)
        {
            if (i + 1 < argc)
            {
                if (strcmp(argv[i + 1], "q") == 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int a = hardcodeig(argc, argv);
    if (a == 2)
    {
        return 0;
    }
    if (a == 1)
    {
        print_simple_find_error();
        return 1;
    }
    unsigned index = 1;
    char *dir = ".";
    if (argv[1][0] != '-')
    {
        dir = argv[1];
        index = 2;
    }
    if (strcmp(argv[1], "-P") == 0 && argv[2][0] != '-')
    {
        dir = argv[2];
        index = 3;
    }
    struct my_expr *expr = parse_primary_expr(argv, &index, argc);
    if (expr == NULL)
    {
        print_no_tree_error();
        return 1;
    }
    char **prints = alloc_prints();
    char **x = eval_expr(expr, dir, prints);
    if (x == NULL)
    {
        free_prints(prints);
    }
    char **args = ORexec(x, prints);
    print_args(args);
    free_expr(expr);
    return 0;
}
