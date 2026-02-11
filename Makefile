# Makefile for pipex project
# Author: your_login

CC = cc
CFLAGS = -Wall -Wextra -Werror
NAME = pipex
NAME_BONUS = pipex_bonus

# Source files for mandatory part
SRCS = pipex.c \
       execute.c \
       utils.c \
       cleanup.c \
       string_utils.c

# Source files for bonus part
BONUS_SRCS = pipex.c \
             execute.c \
             utils.c \
             cleanup.c \
             string_utils.c \
             pipex_bonus.c \
             here_doc.c

# Object files
OBJS = $(SRCS:.c=.o)
BONUS_OBJS = $(BONUS_SRCS:.c=.o)

# Default target
all: $(NAME)

# Compile mandatory part
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

# Compile bonus part
bonus: $(NAME_BONUS)

$(NAME_BONUS): $(BONUS_OBJS)
	$(CC) $(CFLAGS) -o $(NAME_BONUS) $(BONUS_OBJS)

# Create object files
%.o: %.c pipex.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJS) $(BONUS_OBJS)

# Clean all generated files
fclean: clean
	rm -f $(NAME) $(NAME_BONUS)

# Recompile everything
re: fclean all

# Declare phony targets
.PHONY: all clean fclean re bonus