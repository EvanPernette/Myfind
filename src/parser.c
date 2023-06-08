#include "parser.h"

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "expression.h"

enum my_token_type
{
    TOKEN_OR = 0,
    TOKEN_AND,
    TOKEN_NOT,
    TOKEN_LEFT_PARENTHESIS,
    TOKEN_RIGHT_PARENTHESIS,
    TOKEN_NAME,
    TOKEN_TYPE,
    TOKEN_NEWER,
    TOKEN_DELETE,
    TOKEN_EXEC,
    TOKEN_EXECDIR
};

struct my_token
{
    enum my_token_type type;
    char *value;
};

struct my_tokens
{
    struct my_token *data;
    unsigned length;
};

struct token_model
{
    const char *str;
    enum my_token_type token_type;
};

struct my_expr *new_expr(void)
{
    struct my_expr *expr = malloc(sizeof(struct my_expr));
    expr->delete = 0;
    return expr;
}

struct my_expr *parse_name_expr(char **strs, unsigned *index)
{
    struct my_expr *expr = malloc(sizeof(struct my_expr));
    expr->type = EXPR_NAME;
    expr->delete = 0;
    expr->print = 0;
    expr->data.value = strs[*index];
    (*index)++;
    if (expr->data.value == NULL)
    {
        errx(1, "Invalid name");
    }
    if (expr->data.value[0] == '-')
    {
        errx(1, "Invalid userdfqzfqf");
    }
    for (unsigned i = 0; i < strlen(expr->data.value); i++)
    {
        if (expr->data.value[i] == '/')
        {
            warnx("Invalid name");
        }
    }
    if (strs[*index] != NULL && strcmp(strs[*index], "-print") == 0)
    {
        (*index)++;
        expr->print = 1;
        if (strs[*index] != NULL && strcmp(strs[*index], "-delete") == 0)
        {
            (*index)++;
            expr->delete = 2;
        }
    }
    if (strs[*index] != NULL && strcmp(strs[*index], "-delete") == 0)
    {
        (*index)++;
        expr->delete = 1;
        if (strs[*index] != NULL && strcmp(strs[*index], "-print") == 0)
        {
            (*index)++;
            expr->delete = 2;
        }
    }
    return expr;
}

struct my_expr *parse_not_expr(unsigned *index, unsigned length)
{
    assert(*index < length);

    struct my_expr *expr = malloc(sizeof(struct my_expr));
    expr->type = EXPR_NOT;
    expr->delete = 0;
    expr->print = 0;
    return expr;
}

struct my_expr *parse_user_expr(char **strs, unsigned *index,
                                enum my_expr_type type)
{
    struct my_expr *expr = malloc(sizeof(struct my_expr));
    expr->type = type;
    expr->delete = 0;
    expr->print = 0;
    expr->data.value = strs[*index];
    (*index)++;
    if (expr->data.value == NULL)
    {
        errx(1, "Invalid name");
    }
    if (expr->data.value[0] == '-')
    {
        errx(1, "Invalid user");
    }
    if (type == EXPR_TYPE)
    {
        if (strlen(expr->data.value) != 1)
        {
            errx(1, "Invalid type");
        }
    }
    if (strs[*index] != NULL && strcmp(strs[*index], "-print") == 0)
    {
        (*index)++;
        expr->print = 1;
    }
    if (strs[*index] != NULL && strcmp(strs[*index], "-delete") == 0)
    {
        (*index)++;
        expr->delete = 1;
    }
    return expr;
}

unsigned *looking_for_OR(char **strs, unsigned index, unsigned length)
{
    unsigned *indexes = malloc(sizeof(unsigned) * length);
    unsigned i = 0;
    while (index < length)
    {
        if (!strcmp(strs[index], "-o"))
        {
            indexes[i] = index;
            ++i;
        }
        ++(index);
    }
    indexes[i] = length;
    return indexes;
}

struct my_expr *parse_OR_expr(char **strs, unsigned *index, unsigned length,
                              unsigned *indexes)
{
    struct my_expr *expr = new_expr();
    expr->type = EXPR_OR;
    unsigned i = 0;
    while (indexes[i] < *index && indexes[i] < length)
    {
        i++;
    }
    expr->data.children.left = parse_primary_expr(strs, index, indexes[i]);
    (*index)++;
    expr->data.children.right = parse_primary_expr(strs, index, length);
    free(indexes);
    return expr;
}

struct my_expr *parse_primary_expr_for_arg(char **strs, unsigned *index,
                                           unsigned length, const char *arg)
{
    struct my_expr *expr = NULL;
    if (!strcmp(arg, "-a"))
    {
        if (strs[*index] == NULL)
            errx(1, "missing argument to '-a'");
        if (strcmp(strs[*index], "-a") == 0 || strcmp(strs[*index], "-o") == 0)
            errx(1, "missing argument to '-a'");
        return parse_primary_expr(strs, index, length);
    }
    else if (!strcmp(arg, "-name"))
    {
        expr = parse_name_expr(strs, index);
    }
    else if (!strcmp(arg, "-type"))
    {
        expr = parse_user_expr(strs, index, EXPR_TYPE);
    }
    else if (!strcmp(arg, "-newer"))
    {
        expr = parse_user_expr(strs, index, EXPR_NEWER);
    }
    else if (!strcmp(arg, "-exec"))
    {
        expr = parse_user_expr(strs, index, EXPR_EXEC);
    }
    else if (!strcmp(arg, "!"))
    {
        if (strs[*index] == NULL)
            errx(1, "missing argument to '!'");
        expr = parse_not_expr(index, length);
    }
    else if (!strcmp(arg, "-perm"))
    {
        expr = parse_user_expr(strs, index, EXPR_PERM);
    }
    else if (!strcmp(arg, "-user"))
    {
        expr = parse_user_expr(strs, index, EXPR_USER);
    }
    else if (!strcmp(arg, "-group"))
    {
        expr = parse_user_expr(strs, index, EXPR_GROUP);
    }
    else
    {
        errx(1, "Unrecognized primary expression: %s", arg);
    }
    return expr;
}

struct my_expr *parse_primary_expr(char **strs, unsigned *index,
                                   unsigned length)
{
    assert(*index < length);

    struct my_expr *expr = NULL;
    const char *arg = strs[*index];
    ++(*index);
    unsigned *indexes = looking_for_OR(strs, *index, length);
    if (indexes[0] != length)
    {
        (*index)--;
        return parse_OR_expr(strs, index, length, indexes);
    }
    free(indexes);
    expr = parse_primary_expr_for_arg(strs, index, length, arg);
    while (*index < length)
    {
        struct my_expr *right = parse_primary_expr(strs, index, length);
        if (right != NULL)
        {
            struct my_expr *new_expr = malloc(sizeof(struct my_expr));
            new_expr->type = EXPR_AND;
            new_expr->print = 0;
            new_expr->data.children.left = expr;
            new_expr->data.children.right = right;
            expr = new_expr;
        }
    }
    return expr;
}

void free_expr(struct my_expr *expr)
{
    if (expr == NULL)
        return;

    switch (expr->type)
    {
    case EXPR_OR:
        free_expr(expr->data.children.left);
        free_expr(expr->data.children.right);
        break;
    case EXPR_AND:
        free_expr(expr->data.children.left);
        free_expr(expr->data.children.right);
        break;
    case EXPR_NAME:
    case EXPR_TYPE:
    case EXPR_NEWER:
    case EXPR_NOT:
    case EXPR_EXEC:
    case EXPR_USER:
    case EXPR_GROUP:
    case EXPR_PERM:
        break;
    case EXPR_EXECDIR:
        break;
    default:
        fprintf(stderr, "Unknown expression type %d\n", expr->type);
        break;
    }
    free(expr);
    expr = NULL;
}
