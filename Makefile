NAME		= pipex
CC			= cc
CFLAGS		= -Wall -Wextra -Werror
RM			= rm -rf

LIBFT_DIR	= libft
LIBFT		= $(LIBFT_DIR)/libft.a
INC			= -I includes -I $(LIBFT_DIR)

SRCS		= src/pipex.c \
			  src/ft_error.c \
			  src/ft_path.c \
			  src/ft_exec.c
OBJS		= $(SRCS:.c=.o)

BONUS_SRCS	= bonus/pipex_bonus.c \
			  bonus/ft_error_bonus.c \
			  bonus/ft_path_bonus.c \
			  bonus/ft_exec_bonus.c \
			  bonus/ft_here_doc_bonus.c
BONUS_OBJS	= $(BONUS_SRCS:.c=.o)

all: $(LIBFT) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME)

bonus: $(LIBFT) $(BONUS_OBJS)
	$(CC) $(CFLAGS) $(BONUS_OBJS) $(LIBFT) -o $(NAME)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	$(RM) $(OBJS) $(BONUS_OBJS)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	$(RM) $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re
