# Pipex — Guía completa de implementación en C

> Proyecto de 42. Replica el comportamiento de la tubería de shell `< file1 cmd1 | cmd2 > file2` usando las syscalls `pipe`, `fork`, `dup2` y `execve`.

---

## Índice

1. [Qué hace este proyecto](#1-qué-hace-este-proyecto)
2. [Estructura de archivos](#2-estructura-de-archivos)
3. [Conceptos clave del sistema operativo](#3-conceptos-clave-del-sistema-operativo)
4. [Parte obligatoria: paso a paso](#4-parte-obligatoria-paso-a-paso)
   - 4.1 [libft — funciones de utilidad](#41-libft--funciones-de-utilidad)
   - 4.2 [includes/pipex.h](#42-includespipexh)
   - 4.3 [src/ft\_error.c](#43-srcft_errorc)
   - 4.4 [src/ft\_path.c](#44-srcft_pathc)
   - 4.5 [src/ft\_exec.c](#45-srcft_execc)
   - 4.6 [src/pipex.c](#46-srcpipexc)
   - 4.7 [Makefile raíz](#47-makefile-raíz)
5. [Parte bonus: paso a paso](#5-parte-bonus-paso-a-paso)
   - 5.1 [includes/pipex\_bonus.h](#51-includespipex_bonush)
   - 5.2 [bonus/ft\_error\_bonus.c](#52-bonusft_error_bonusc)
   - 5.3 [bonus/ft\_path\_bonus.c y ft\_exec\_bonus.c](#53-bonusft_path_bonusc-y-ft_exec_bonusc)
   - 5.4 [bonus/ft\_here\_doc\_bonus.c](#54-bonusft_here_doc_bonusc)
   - 5.5 [bonus/pipex\_bonus.c](#55-bonuspipex_bonusc)
6. [Compilar y ejecutar](#6-compilar-y-ejecutar)
7. [Casos de prueba completos](#7-casos-de-prueba-completos)
8. [Errores frecuentes y cómo evitarlos](#8-errores-frecuentes-y-cómo-evitarlos)
9. [Gestión de memoria y descriptores de fichero](#9-gestión-de-memoria-y-descriptores-de-fichero)

---

## 1. Qué hace este proyecto

### Parte obligatoria

```bash
./pipex file1 cmd1 cmd2 file2
```

Es equivalente a:

```bash
< file1 cmd1 | cmd2 > file2
```

El programa abre `file1` como entrada estándar del primer comando, conecta la
salida de ese comando con la entrada del segundo mediante una tubería del
kernel, y redirige la salida final hacia `file2`.

### Parte bonus

```bash
# Múltiples comandos encadenados
./pipex file1 cmd1 cmd2 cmd3 ... cmdn file2

# Heredoc: equivale a cmd1 << LIMITER | cmd2 >> file
./pipex here_doc LIMITER cmd1 cmd2 file
```

---

## 2. Estructura de archivos

```
pipex/
├── Makefile                  ← reglas: all / bonus / clean / fclean / re
├── includes/
│   ├── pipex.h               ← cabecera parte obligatoria
│   └── pipex_bonus.h         ← cabecera parte bonus
├── libft/
│   ├── Makefile
│   ├── libft.h
│   ├── ft_strlen.c
│   ├── ft_strncmp.c
│   ├── ft_strcmp.c
│   ├── ft_putstr_fd.c
│   ├── ft_strjoin.c
│   ├── ft_substr.c
│   └── ft_split.c
├── src/                      ← parte obligatoria
│   ├── pipex.c
│   ├── ft_error.c
│   ├── ft_path.c
│   └── ft_exec.c
└── bonus/                    ← parte bonus
    ├── pipex_bonus.c
    ├── ft_error_bonus.c
    ├── ft_path_bonus.c
    ├── ft_exec_bonus.c
    └── ft_here_doc_bonus.c
```

---

## 3. Conceptos clave del sistema operativo

Antes de escribir una sola línea es imprescindible entender cuatro syscalls:

### `pipe(int fd[2])`

Crea un canal de comunicación unidireccional dentro del kernel.

- `fd[0]` → extremo de **lectura**
- `fd[1]` → extremo de **escritura**

Los datos escritos en `fd[1]` se pueden leer desde `fd[0]`. Si todos los
descriptores de escritura se cierran, la lectura devuelve 0 (EOF). Si quedan
descriptores de escritura abiertos (incluso en el proceso padre) la lectura
**bloqueará para siempre**.

```c
int fd[2];
pipe(fd);  // fd[0]=leer, fd[1]=escribir
```

### `fork()`

Crea un proceso hijo que es copia exacta del padre. El espacio de memoria se
duplica (copy-on-write). Los descriptores de fichero abiertos se heredan.

| Valor de retorno | Significado |
|---|---|
| `-1` | Error — no se creó el hijo |
| `0` | Estamos en el **proceso hijo** |
| `> 0` | Estamos en el **proceso padre**; el valor es el PID del hijo |

```c
pid_t pid = fork();
if (pid == -1)  { /* error */ }
if (pid == 0)   { /* código del hijo */ }
/* código del padre continúa aquí */
```

### `dup2(int oldfd, int newfd)`

Duplica `oldfd` en `newfd`. Si `newfd` ya estaba abierto, lo cierra primero.
Después de la llamada, `newfd` apunta al mismo recurso que `oldfd`.

Se usa para redirigir stdin (fd=0) y stdout (fd=1):

```c
dup2(infile,  STDIN_FILENO);   // stdin  → infile
dup2(fd[1],   STDOUT_FILENO);  // stdout → extremo escritura del pipe
```

Regla: después de `dup2`, cierra el descriptor original (`oldfd`), porque ya
tienes el alias `newfd` y no necesitas dos entradas.

### `execve(const char *path, char **argv, char **envp)`

Reemplaza el proceso actual por el programa indicado en `path`. Si tiene
éxito, **no vuelve**; la función solo retorna si hubo un error.

```c
execve("/bin/ls", argv, envp);
// Si llegamos aquí, execve falló
perror("execve");
exit(EXIT_FAILURE);
```

---

## 4. Parte obligatoria: paso a paso

### 4.1 libft — funciones de utilidad

La biblioteca está en `libft/` y compila como archivo estático `libft.a`.
Se enlaza tanto con la parte obligatoria como con la bonus.

**`libft/libft.h`**

```c
#ifndef LIBFT_H
# define LIBFT_H

# include <stdlib.h>
# include <unistd.h>

size_t   ft_strlen(const char *s);
int      ft_strncmp(const char *s1, const char *s2, size_t n);
int      ft_strcmp(char *s1, char *s2);
void     ft_putstr_fd(char *s, int fd);
char    *ft_strjoin(char const *s1, char const *s2);
char    *ft_substr(char const *s, unsigned int start, size_t len);
char   **ft_split(char const *s, char c);

#endif
```

**`libft/ft_strlen.c`** — longitud de cadena

```c
#include "libft.h"

size_t  ft_strlen(const char *s)
{
    size_t  i;

    i = 0;
    while (s[i])
        i++;
    return (i);
}
```

**`libft/ft_strncmp.c`** — comparación de N bytes

```c
#include "libft.h"

int ft_strncmp(const char *s1, const char *s2, size_t n)
{
    size_t  i;

    i = 0;
    if (n == 0)
        return (0);
    while (i < n - 1 && s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
```

**`libft/ft_strcmp.c`** — comparación completa (usada en bonus)

```c
#include "libft.h"

int ft_strcmp(char *s1, char *s2)
{
    int i;

    i = 0;
    while (s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
```

**`libft/ft_putstr_fd.c`** — escribe cadena en un descriptor

```c
#include "libft.h"

void    ft_putstr_fd(char *s, int fd)
{
    if (!s)
        return ;
    write(fd, s, ft_strlen(s));
}
```

**`libft/ft_strjoin.c`** — concatena dos cadenas en nuevo malloc

```c
#include "libft.h"

char    *ft_strjoin(char const *s1, char const *s2)
{
    char    *str;
    size_t   i;
    size_t   j;

    if (!s1 || !s2)
        return (NULL);
    str = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
    if (!str)
        return (NULL);
    i = 0;
    while (s1[i])
    {
        str[i] = s1[i];
        i++;
    }
    j = 0;
    while (s2[j])
        str[i++] = s2[j++];
    str[i] = 0;
    return (str);
}
```

**`libft/ft_substr.c`** — extrae subcadena

```c
#include "libft.h"

char    *ft_substr(char const *s, unsigned int start, size_t len)
{
    char    *sub;
    size_t   slen;
    size_t   i;

    if (!s)
        return (NULL);
    slen = ft_strlen(s);
    if (start >= (unsigned int)slen)
        len = 0;
    else if (len > slen - start)
        len = slen - start;
    sub = malloc(len + 1);
    if (!sub)
        return (NULL);
    i = 0;
    while (i < len)
    {
        sub[i] = s[start + i];
        i++;
    }
    sub[i] = 0;
    return (sub);
}
```

**`libft/ft_split.c`** — divide cadena por un carácter delimitador

Esta función es la más compleja. Necesita tres auxiliares:

- `count_words` — cuenta el número de palabras.
- `word_len` — longitud de la siguiente palabra.
- `free_all` — libera el array en caso de fallo de malloc.

```c
#include "libft.h"

static size_t   count_words(char const *s, char c)
{
    size_t  cnt;
    size_t  i;

    cnt = 0;
    i = 0;
    while (s[i])
    {
        if (s[i] != c && (i == 0 || s[i - 1] == c))
            cnt++;
        i++;
    }
    return (cnt);
}

static char **free_all(char **arr)
{
    size_t  i;

    i = 0;
    while (arr[i])
    {
        free(arr[i]);
        i++;
    }
    free(arr);
    return (NULL);
}

static size_t   word_len(char const *s, size_t i, char c)
{
    size_t  len;

    len = 0;
    while (s[i + len] && s[i + len] != c)
        len++;
    return (len);
}

char    **ft_split(char const *s, char c)
{
    char    **res;
    size_t    i;
    size_t    j;
    size_t    len;

    if (!s)
        return (NULL);
    res = malloc(sizeof(char *) * (count_words(s, c) + 1));
    if (!res)
        return (NULL);
    i = 0;
    j = 0;
    while (j < count_words(s, c))
    {
        while (s[i] == c)
            i++;
        len = word_len(s, i, c);
        res[j] = ft_substr(s, i, len);
        if (!res[j])
            return (free_all(res));
        i += len;
        j++;
    }
    res[j] = NULL;
    return (res);
}
```

**`libft/Makefile`**

```makefile
NAME    = libft.a
CC      = cc
CFLAGS  = -Wall -Wextra -Werror
AR      = ar rcs
RM      = rm -rf

SRCS    = ft_strlen.c ft_strncmp.c ft_strcmp.c ft_putstr_fd.c \
          ft_strjoin.c ft_substr.c ft_split.c
OBJS    = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
$(AR) $(NAME) $(OBJS)

clean:
$(RM) $(OBJS)

fclean: clean
$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
```

---

### 4.2 `includes/pipex.h`

Centraliza todos los includes del sistema y las macros de error, más los
prototipos de las funciones distribuidas en varios archivos.

```c
#ifndef PIPEX_H
# define PIPEX_H

# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include "libft.h"

# define ERR_ARGS  "Usage: ./pipex file1 cmd1 cmd2 file2\n"
# define ERR_PIPE  "pipe"
# define ERR_FORK  "fork"
# define ERR_OPEN  "open"
# define ERR_CMD   "command not found"
# define ERR_EXEC  "execve"

/* ft_error.c */
void    err_exit(char *msg);
void    free_tab(char **tab);

/* ft_path.c */
char   *get_path(char *cmd, char **envp);

/* ft_exec.c */
void    exec_cmd(char *arg, char **envp);

/* pipex.c */
void    child1(int fd[2], char **argv, char **envp);
void    child2(int fd[2], char **argv, char **envp);

#endif
```

Por qué una macro por error: `perror` imprime el prefijo que le pasamos y
añade `: <descripción del errno>` automáticamente, así los mensajes son
informativos sin añadir código extra.

---

### 4.3 `src/ft_error.c`

```c
#include "pipex.h"

/* Imprime el error del sistema y termina el proceso. */
void    err_exit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/*
** Libera un array de cadenas terminado en NULL (resultado de ft_split).
** Primero libera cada cadena, luego el array que las contiene.
*/
void    free_tab(char **tab)
{
    int i;

    i = 0;
    while (tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}
```

`free_tab` existe porque `ft_split` crea un array de punteros en el heap.
Para no filtrar memoria hay que liberar cada elemento y después el propio
array.

---

### 4.4 `src/ft_path.c`

El objetivo es obtener la ruta absoluta de un comando (por ejemplo `cat`)
buscando en las entradas de la variable de entorno `PATH`.

```
PATH=/usr/local/bin:/usr/bin:/bin
```

El algoritmo:

1. Recorre `envp[]` hasta encontrar la entrada que empieza por `"PATH="`.
2. Divide la lista de directorios con `ft_split(..., ':')`.
3. Para cada directorio prueba `directorio + "/" + comando` con `access`.
4. Devuelve la primera ruta válida, o `NULL` si no existe.

```c
#include "pipex.h"

/*
** Prueba cada directorio en dirs[]. Libera dirs y devuelve la ruta
** que sea ejecutable, o NULL si ninguna lo es.
*/
static char *check_paths(char **dirs, char *cmd)
{
    char    *temp;
    char    *path;
    int      i;

    i = 0;
    while (dirs[i])
    {
        temp = ft_strjoin(dirs[i], "/");
        path = ft_strjoin(temp, cmd);
        free(temp);
        if (access(path, F_OK | X_OK) == 0)
        {
            free_tab(dirs);
            return (path);
        }
        free(path);
        i++;
    }
    free_tab(dirs);
    return (NULL);
}

char    *get_path(char *cmd, char **envp)
{
    char    **path_env;
    int       i;

    i = 0;
    while (envp[i] && ft_strncmp(envp[i], "PATH=", 5))
        i++;
    if (!envp[i])
        return (NULL);
    path_env = ft_split(envp[i] + 5, ':');   /* salta "PATH=" */
    if (!path_env)
        return (NULL);
    return (check_paths(path_env, cmd));
}
```

**Puntos críticos:**

- `envp[i] + 5` salta los cinco caracteres de `"PATH="`.
- `check_paths` recibe la propiedad del array `dirs` y siempre lo libera
  (tanto en el camino de éxito como en el de fallo), así no hay fugas.

---

### 4.5 `src/ft_exec.c`

Divide el argumento de comando (p. ej. `"ls -l"`) en tokens, busca la ruta
y llama a `execve`.

```c
#include "pipex.h"

void    exec_cmd(char *arg, char **envp)
{
    char    **cmd;
    char     *path;

    /* "ls -l" → ["ls", "-l", NULL] */
    cmd = ft_split(arg, ' ');
    if (!cmd || !cmd[0])
    {
        free_tab(cmd);
        err_exit(ERR_CMD);
    }
    path = get_path(cmd[0], envp);
    if (!path)
    {
        free_tab(cmd);
        err_exit(ERR_CMD);
    }
    execve(path, cmd, envp);
    /* Si llegamos aquí, execve falló */
    free_tab(cmd);
    free(path);
    err_exit(ERR_EXEC);
}
```

`execve` recibe:
- `path` — ruta absoluta del ejecutable.
- `cmd` — argv del programa nuevo (el primer elemento es el nombre del programa).
- `envp` — variables de entorno heredadas del proceso padre.

Si `execve` tiene éxito el proceso se transforma: las líneas siguientes nunca
se ejecutan. Si falla, las líneas siguientes limpian la memoria e informan del
error.

---

### 4.6 `src/pipex.c`

Es el corazón del proyecto. Crea el pipe, lanza los dos procesos hijo y espera
a que terminen.

```c
#include "pipex.h"

/*
** Hijo 1: abre file1, redirige stdin hacia él,
**         redirige stdout hacia el extremo de escritura del pipe,
**         ejecuta cmd1.
*/
void    child1(int fd[2], char **argv, char **envp)
{
    int infile;

    infile = open(argv[1], O_RDONLY);
    if (infile == -1)
        err_exit(ERR_OPEN);
    dup2(infile, STDIN_FILENO);   /* stdin  ← file1 */
    dup2(fd[1],  STDOUT_FILENO);  /* stdout → pipe[escritura] */
    close(fd[0]);   /* el hijo no lee del pipe */
    close(fd[1]);   /* ya está duplicado en stdout */
    close(infile);  /* ya está duplicado en stdin */
    exec_cmd(argv[2], envp);
}

/*
** Hijo 2: abre file2 para escritura (creándolo o truncándolo),
**         redirige stdin hacia el extremo de lectura del pipe,
**         redirige stdout hacia file2,
**         ejecuta cmd2.
*/
void    child2(int fd[2], char **argv, char **envp)
{
    int outfile;

    outfile = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfile == -1)
        err_exit(ERR_OPEN);
    dup2(fd[0],   STDIN_FILENO);   /* stdin  ← pipe[lectura] */
    dup2(outfile, STDOUT_FILENO);  /* stdout → file2 */
    close(fd[0]);
    close(fd[1]);
    close(outfile);
    exec_cmd(argv[3], envp);
}

int main(int argc, char **argv, char **envp)
{
    int     fd[2];
    pid_t   pid1;
    pid_t   pid2;

    if (argc != 5)
    {
        ft_putstr_fd(ERR_ARGS, 2);
        return (EXIT_FAILURE);
    }
    if (pipe(fd) == -1)
        err_exit(ERR_PIPE);
    pid1 = fork();
    if (pid1 == -1)
        err_exit(ERR_FORK);
    if (pid1 == 0)
        child1(fd, argv, envp);   /* sólo el hijo llega aquí */
    pid2 = fork();
    if (pid2 == -1)
        err_exit(ERR_FORK);
    if (pid2 == 0)
        child2(fd, argv, envp);   /* sólo el hijo llega aquí */
    /*
    ** El padre DEBE cerrar los dos extremos del pipe.
    ** Si no, child2 nunca recibe EOF porque el padre sigue
    ** con fd[1] abierto, y el proceso se bloquea para siempre.
    */
    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);  /* espera a cmd1 */
    waitpid(pid2, NULL, 0);  /* espera a cmd2 */
    return (EXIT_SUCCESS);
}
```

**Flujo de descriptores** (diagrama):

```
 parent/child1                          parent/child2
 ─────────────                          ─────────────
 infile → dup2 → stdin(0)              pipe[0] → dup2 → stdin(0)
 pipe[1] → dup2 → stdout(1)            outfile → dup2 → stdout(1)
 close(pipe[0])                         close(pipe[1])
 close(pipe[1])  ← ya en stdout         close(pipe[0])  ← ya en stdin
 close(infile)   ← ya en stdin          close(outfile)  ← ya en stdout
          │ execve(cmd1)                          │ execve(cmd2)
          │                                       │
          └──────── pipe kernel ─────────────────►│
```

---

### 4.7 Makefile raíz

```makefile
NAME        = pipex
CC          = cc
CFLAGS      = -Wall -Wextra -Werror
RM          = rm -rf

LIBFT_DIR   = libft
LIBFT       = $(LIBFT_DIR)/libft.a
INC         = -I includes -I $(LIBFT_DIR)

SRCS        = src/pipex.c \
              src/ft_error.c \
              src/ft_path.c \
              src/ft_exec.c
OBJS        = $(SRCS:.c=.o)

BONUS_SRCS  = bonus/pipex_bonus.c \
              bonus/ft_error_bonus.c \
              bonus/ft_path_bonus.c \
              bonus/ft_exec_bonus.c \
              bonus/ft_here_doc_bonus.c
BONUS_OBJS  = $(BONUS_SRCS:.c=.o)

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
```

---

## 5. Parte bonus: paso a paso

La parte bonus extiende el binario para soportar:

1. **Múltiples comandos**: cualquier número de comandos enlazados en cadena.
2. **`here_doc`**: lee de stdin hasta el limitador y usa `O_APPEND` en el fichero de salida.

Los ficheros bonus son completamente independientes de los obligatorios: mismo
binario final (`pipex`), pero compilado desde `bonus/`.

---

### 5.1 `includes/pipex_bonus.h`

```c
#ifndef PIPEX_BONUS_H
# define PIPEX_BONUS_H

# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include "libft.h"

# define ERR_ARGS   "Usage:\n  ./pipex file1 cmd1 ... cmdn file2\n" \
                    "  ./pipex here_doc LIMITER cmd1 cmd2 file\n"
# define ERR_PIPE   "pipe"
# define ERR_FORK   "fork"
# define ERR_OPEN   "open"
# define ERR_CMD    "command not found"
# define ERR_EXEC   "execve"
# define HEREDOC_TMP  ".heredoc_tmp"    /* fichero temporal del here_doc */

/* ft_error_bonus.c */
void    err_exit(char *msg);
void    free_tab(char **tab);

/* ft_path_bonus.c */
char   *get_path(char *cmd, char **envp);

/* ft_exec_bonus.c */
void    exec_cmd(char *arg, char **envp);

/* ft_here_doc_bonus.c */
void    here_doc(char *limiter);

/* pipex_bonus.c */
void    child_process(int fd_in, int fd_out, char *cmd, char **envp);
void    pipex_multiple(int ac, char **av, char **envp);

#endif
```

La diferencia respecto a `pipex.h` son la macro `HEREDOC_TMP`, el prototipo
`here_doc` y el nombre del include guard.

---

### 5.2 `bonus/ft_error_bonus.c`

Igual que la versión obligatoria, con un detalle extra: `err_exit` llama a
`unlink(HEREDOC_TMP)` antes de salir para no dejar el fichero temporal en
disco si el programa falla mientras lo estaba usando.

```c
#include "pipex_bonus.h"

void    err_exit(char *msg)
{
    perror(msg);
    unlink(HEREDOC_TMP);   /* limpia el fichero temporal si existe */
    exit(EXIT_FAILURE);
}

void    free_tab(char **tab)
{
    int i;

    i = 0;
    while (tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}
```

---

### 5.3 `bonus/ft_path_bonus.c` y `ft_exec_bonus.c`

Idénticos a sus homólogos en `src/`, salvo que incluyen `pipex_bonus.h`.
Se repiten porque la parte bonus debe compilar de forma autónoma.

```c
/* bonus/ft_path_bonus.c — igual lógica que src/ft_path.c */
#include "pipex_bonus.h"

static char *check_paths(char **dirs, char *cmd) { /* ... igual ... */ }
char        *get_path(char *cmd, char **envp)     { /* ... igual ... */ }
```

```c
/* bonus/ft_exec_bonus.c — igual lógica que src/ft_exec.c */
#include "pipex_bonus.h"

void    exec_cmd(char *arg, char **envp) { /* ... igual ... */ }
```

---

### 5.4 `bonus/ft_here_doc_bonus.c`

El modo `here_doc` simula `<<` del shell: lee líneas de stdin hasta que
aparece el limitador exacto y las almacena en un fichero temporal.

```c
#include "pipex_bonus.h"

/*
** Lee una línea completa del descriptor fd, un byte a la vez.
** Devuelve NULL si el descriptor se cierra sin datos (EOF inmediato).
*/
static char *read_line(int fd)
{
    char    *line;
    char     c;
    int      i;

    line = malloc(4096);
    if (!line)
        return (NULL);
    i = 0;
    while (i < 4095 && read(fd, &c, 1) > 0)
    {
        line[i++] = c;
        if (c == '\n')
            break ;
    }
    if (i == 0)
    {
        free(line);
        return (NULL);
    }
    line[i] = 0;
    return (line);
}

/*
** Comprueba si la línea leída es exactamente el limitador.
** Se compara longitud + el carácter siguiente debe ser '\n' o '\0'
** para evitar que "EOFMORE" dispare el limitador "EOF".
*/
static int  is_limiter(char *line, char *limiter, size_t len)
{
    if (!line)
        return (1);
    if (ft_strncmp(line, limiter, len) == 0
        && (line[len] == '\n' || line[len] == '\0'))
        return (1);
    return (0);
}

/*
** Lee stdin mostrando "> " como prompt hasta que aparece el limitador.
** Todo lo leído se escribe en HEREDOC_TMP.
*/
void    here_doc(char *limiter)
{
    int     fd;
    char    *line;
    size_t   len;

    len = ft_strlen(limiter);
    fd = open(HEREDOC_TMP, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        err_exit("here_doc: open");
    while (1)
    {
        ft_putstr_fd("> ", 1);
        line = read_line(0);
        if (is_limiter(line, limiter, len))
        {
            free(line);
            break ;
        }
        ft_putstr_fd(line, fd);
        free(line);
    }
    close(fd);
}
```

---

### 5.5 `bonus/pipex_bonus.c`

Este archivo contiene la lógica de múltiples pipes y del modo `here_doc`.

#### Función auxiliar: `child_process`

Redirige `fd_in` → stdin y `fd_out` → stdout, cierra ambos y ejecuta el
comando. Es el bloque reutilizable para todos los hijos.

```c
void    child_process(int fd_in, int fd_out, char *cmd, char **envp)
{
    dup2(fd_in,  STDIN_FILENO);
    dup2(fd_out, STDOUT_FILENO);
    close(fd_in);
    close(fd_out);
    exec_cmd(cmd, envp);
}
```

#### Función auxiliar: `pipe_child`

Crea un pipe, hace fork y lanza el hijo para un comando intermedio. El padre
recibe el array `fd` relleno para poder encadenar el siguiente.

```c
static void pipe_child(int in_fd, int *fd, char *cmd, char **envp)
{
    pid_t   pid;

    if (pipe(fd) == -1)
        err_exit(ERR_PIPE);
    pid = fork();
    if (pid == -1)
        err_exit(ERR_FORK);
    if (pid == 0)
    {
        close(fd[0]);                          /* el hijo no lee de este pipe */
        child_process(in_fd, fd[1], cmd, envp);
    }
    /* el padre continúa aquí */
}
```

#### Función auxiliar: `setup_pipeline`

Itera sobre todos los comandos excepto el último. En cada iteración:

1. Llama a `pipe_child` para lanzar el hijo `i`.
2. Cierra el extremo de escritura (`fd[1]`) ya que sólo lo usa el hijo.
3. Cierra `in_fd` (el extremo de lectura del pipe anterior) para no filtrarlo.
4. Guarda el nuevo extremo de lectura (`fd[0]`) como entrada del siguiente hijo.

```c
static int  setup_pipeline(int ac, char **av, char **envp)
{
    int     fd[2];
    int     in_fd;
    int     i;

    in_fd = open(av[1], O_RDONLY);
    if (in_fd == -1)
        err_exit(ERR_OPEN);
    i = 2;
    while (i < ac - 2)           /* todos menos el último comando */
    {
        pipe_child(in_fd, fd, av[i], envp);
        close(fd[1]);            /* padre no necesita escribir */
        close(in_fd);            /* ya pasado al hijo anterior */
        in_fd = fd[0];           /* próxima entrada es este extremo lector */
        i++;
    }
    return (in_fd);              /* extremo lector del último pipe creado */
}
```

#### Función auxiliar: `last_child`

Lanza el último comando conectando su salida directamente al fichero de salida.

```c
static void last_child(int in_fd, char *outfile, char *cmd, char **envp)
{
    int     out_fd;
    pid_t   pid;

    out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1)
        err_exit(ERR_OPEN);
    pid = fork();
    if (pid == -1)
        err_exit(ERR_FORK);
    if (pid == 0)
        child_process(in_fd, out_fd, cmd, envp);
    close(in_fd);
    close(out_fd);
}
```

#### Función principal del modo múltiple: `pipex_multiple`

```c
void    pipex_multiple(int ac, char **av, char **envp)
{
    int in_fd;

    in_fd = setup_pipeline(ac, av, envp);
    last_child(in_fd, av[ac - 1], av[ac - 2], envp);
    while (wait(NULL) > 0)    /* espera a TODOS los hijos */
        ;
}
```

`wait(NULL)` en bucle recoge todos los zombies sin importar el orden de
finalización. Devuelve `-1` cuando no quedan más hijos.

#### Modo `here_doc`: `here_doc_mode`

```c
static void here_doc_mode(char **av, char **envp)
{
    int     in_fd;
    int     out_fd;
    int     fd[2];
    pid_t   pid1;
    pid_t   pid2;

    here_doc(av[2]);                  /* rellena HEREDOC_TMP */
    in_fd  = open(HEREDOC_TMP, O_RDONLY);
    if (in_fd == -1)
        err_exit(ERR_OPEN);
    out_fd = open(av[5], O_WRONLY | O_CREAT | O_APPEND, 0644); /* >> */
    if (out_fd == -1)
        err_exit(ERR_OPEN);
    if (pipe(fd) == -1)
        err_exit(ERR_PIPE);
    pid1 = fork();
    if (pid1 == -1)
        err_exit(ERR_FORK);
    if (pid1 == 0)
    {
        close(fd[0]);
        child_process(in_fd, fd[1], av[3], envp);   /* cmd1 */
    }
    pid2 = fork();
    if (pid2 == -1)
        err_exit(ERR_FORK);
    if (pid2 == 0)
    {
        close(fd[1]);
        child_process(fd[0], out_fd, av[4], envp);  /* cmd2 */
    }
    close(fd[0]);
    close(fd[1]);
    close(in_fd);
    close(out_fd);
    wait(NULL);
    wait(NULL);
    unlink(HEREDOC_TMP);  /* borra el fichero temporal */
}
```

Por qué `O_APPEND`: el enunciado del proyecto establece que `here_doc` es
equivalente a `>> file` (modo añadir), no a `> file` (modo truncar).

#### `main` de la parte bonus

```c
int main(int ac, char **av, char **envp)
{
    if (ac < 5)
    {
        ft_putstr_fd(ERR_ARGS, 2);
        return (EXIT_FAILURE);
    }
    if (ft_strcmp(av[1], "here_doc") == 0)
    {
        if (ac != 6)
        {
            ft_putstr_fd(ERR_ARGS, 2);
            return (EXIT_FAILURE);
        }
        here_doc_mode(av, envp);
    }
    else
        pipex_multiple(ac, av, envp);
    return (EXIT_SUCCESS);
}
```

---

## 6. Compilar y ejecutar

```bash
# Compilar parte obligatoria
make

# Compilar parte bonus (reemplaza el binario pipex)
make bonus

# Limpiar objetos
make clean

# Limpiar objetos y binario
make fclean

# Recompilar desde cero
make re
```

---

## 7. Casos de prueba completos

### Parte obligatoria

```bash
# Preparar fichero de entrada
echo "hello 42 barcelona" > infile

# Prueba básica
./pipex infile "cat" "wc -w" outfile
cat outfile                         # debe imprimir 3
< infile cat | wc -w               # comprobación shell: debe coincidir

# Comando con argumentos
./pipex infile "grep 42" "wc -c" outfile
cat outfile
< infile grep 42 | wc -c

# Fichero de entrada inexistente (debe mostrar error, no bloquear)
./pipex no_existe "cat" "wc -w" outfile
echo "Código de salida: $?"

# Comando inexistente (debe mostrar error, no bloquear)
./pipex infile "comandofalso" "wc -w" outfile
echo "Código de salida: $?"

# Sin permisos de lectura
touch bloqueado && chmod 000 bloqueado
./pipex bloqueado "cat" "wc -w" outfile
chmod 644 bloqueado && rm bloqueado

# Número incorrecto de argumentos
./pipex
./pipex infile "cat"
```

### Parte bonus — múltiples pipes

```bash
echo "test 1 2 3" > infile2

# Tres comandos
./pipex infile2 "cat" "grep test" "wc -w" outfile
cat outfile                                 # debe imprimir 4
< infile2 cat | grep test | wc -w          # comprobación shell

# Cuatro comandos
./pipex infile2 "cat" "cat" "grep 1" "wc -c" outfile
cat outfile
< infile2 cat | cat | grep 1 | wc -c
```

### Parte bonus — here_doc

```bash
# Prueba básica (escribe las líneas, luego escribe el limitador)
./pipex here_doc EOF cat "wc -w" outfile
# Escribe cuando aparezca ">":
#   hello world
#   EOF
cat outfile     # debe imprimir 2

# Comprobar que es modo APPEND (ejecutar dos veces)
./pipex here_doc STOP "cat" "wc -l" outfile
# > line1
# > line2
# > STOP
cat outfile     # debe tener el resultado del wc anterior + el nuevo

# Limitador que es prefijo de otro (no debe dispararse antes)
./pipex here_doc EOF cat "wc -l" outfile
# > EOFMORE    ← no debe disparar el limtador
# > EOF        ← éste sí
cat outfile
```

---

## 8. Errores frecuentes y cómo evitarlos

| Síntoma | Causa más probable | Solución |
|---|---|---|
| El programa se bloquea para siempre | El padre no cierra `fd[0]` y `fd[1]` tras los forks | Añadir `close(fd[0]); close(fd[1]);` en el padre |
| El pipe intermedio no cierra en el bonus | `in_fd` no se cierra en el bucle padre | Llamar `close(in_fd)` antes de reasignar `in_fd = fd[0]` |
| Proceso zombie | `wait`/`waitpid` no se llama para todos los hijos | Usar `while (wait(NULL) > 0);` en la versión bonus |
| Fugas de memoria | `ft_split` y `ft_strjoin` no se liberan | Llamar siempre a `free_tab` y `free` en todos los caminos de salida |
| Comando no encontrado aunque existe | PATH no se lee correctamente | Verificar que `envp[i] + 5` salta `"PATH="` y que `ft_split` usa `':'` |
| Fichero temporal `.heredoc_tmp` persiste | `unlink` no se llama en los caminos de error | Llamar `unlink(HEREDOC_TMP)` dentro de `err_exit` en la versión bonus |
| `execve` falla silenciosamente | No se comprueba el retorno | Las líneas tras `execve` siempre indican un fallo; llamar a `err_exit` |
| Limitador del here_doc dispara con prefijos | Comparación sólo con `strncmp` | Comprobar que el carácter en `line[len]` sea `'\n'` o `'\0'` |

---

## 9. Gestión de memoria y descriptores de fichero

### Regla de oro para descriptores

> Cada `open`, `pipe` y `dup2` que creas, tú eres responsable de cerrarlo.

Mapa mental:

```
Acción                    →  Descriptor creado   →  Cuándo cerrarlo
─────────────────────────────────────────────────────────────────────
open(file1)               →  infile              →  después de dup2
pipe(fd)                  →  fd[0], fd[1]        →  hijo: los no usados
                                                     padre: ambos tras forks
dup2(infile, STDIN)       →  alias en fd=0       →  el original infile
dup2(fd[1], STDOUT)       →  alias en fd=1       →  el original fd[1]
```

### Regla de oro para memoria

> Cada `malloc` (incluyendo los de `ft_split`, `ft_strjoin`, `ft_substr`) debe
> tener un `free` correspondiente en **todos** los caminos de ejecución, tanto
> en el éxito como en el error.

```c
/* Ejemplo correcto: liberar en el camino de error */
path = get_path(cmd[0], envp);
if (!path)
{
    free_tab(cmd);    /* ← liberar antes de salir */
    err_exit(ERR_CMD);
}
execve(path, cmd, envp);
/* execve no vuelve si tiene éxito, así que no hay fuga */
free_tab(cmd);  /* sólo si execve falla */
free(path);
err_exit(ERR_EXEC);
```

### Verificar con Valgrind

```bash
echo "test" > infile
valgrind --leak-check=full --show-leak-kinds=all \
    ./pipex infile "cat" "wc -w" outfile
```

La salida debería mostrar `0 bytes in 0 blocks` en la sección
`definitely lost`.

---

*Proyecto pipex — 42 Barcelona*
