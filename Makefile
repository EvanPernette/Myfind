CC = gcc
CFLAGS = -g -fsanitize=address -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

SRC = src/eval_expr.c src/main.c src/my_find_name.c src/my_find_test.c src/parser.c src/simple_find.c src/my_find_newer.c src/my_find_perm.c src/my_find_group.c src/my_find_user.c
OBJ = $(SRC:.c=.o)

myfind: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) myfind