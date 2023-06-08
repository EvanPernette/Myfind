#ifndef PARSER_H
#define PARSER_H

#include "expression.h"

struct my_expr *parse_primary_expr(char **strs, unsigned *index,
                                   unsigned length);

void free_expr(struct my_expr *expr);

#endif /* ! PARSER_H */
