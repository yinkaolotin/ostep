#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
        // command arg arg | command > arg | command < arg > arg
        char **commands;
        // three
        int  ncommands;
        // two
        int npipes;
        char **args[];
} CMD;

int ntoken(char *token, char *sep)
{
        int n = 0;
        char *t;
        while((t = strsep(&token, sep)) != NULL)
        {
                n++;
        }
        return n;
}

void trim(char *str)
{
    char *start, *end;

    if (str == NULL || *str == '\0') {
        return;
    }

    start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *str = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    memmove(str, start, end - start + 2);
}

void *parse_cmd_line(FILE *in, char **command)
{
        int n;
        size_t l = 0;

        size_t r = getline(command, &l, in);
        if (r == -1)
        {
                printf("failed to get command line\n");
                exit(1);
        }

        (*command)[r-1] = '\0';
        char *x = strdup(*command);
        n = ntoken(x, "|");
        free(x);
        CMD *c = malloc(sizeof(CMD) + (n+1) * sizeof(char *));
        c->commands = malloc(n+1);
        if (n!= 0) c->npipes = n - 1;
        c->ncommands = n;

        int i = 0;
        char *cmd;
        while((cmd = strsep(command, "|")) != NULL)
        {
                int nargs;
                char *t;

                trim(cmd);
                c->commands[i] = strdup(cmd);

                char *y = strdup(cmd);
                nargs = ntoken(y, " ");
                free(y);
                c->args[i] = malloc(nargs + 1);
                if (c->args[i] ==  NULL)
                {
                        perror("malloc failed");
                        exit(1);
                }

                int j = 0;
                while((t = strsep(&cmd, " ")) != NULL)
                {
                        c->args[i][j] = strdup(t);
                        j++;
                }
                i++;
        }


        return c;
}


int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        CMD *c;
        char *command;
        FILE *in;

        in = fdopen(STDIN_FILENO, "r");
        if (in == NULL)
        {
                perror("fdopen");
                exit(1);
        }

        while(1)
        {
                printf("wish> ");

                c = parse_cmd_line(in, &command);
                pid_t pids[c->ncommands];
                int i;

                static int saved_pipe;
                for (i = 0; i <= c->npipes; i++)
                {
                        int pid;
                        int pipefd[2];

                        if (pipe(pipefd) == -1)
                        {
                                perror("pipe");
                                exit(1);
                        }


                        pid = fork();
                        if (pid < 0)
                        {
                                perror("fork");
                                exit(1);
                        } else if (pid == 0)
                        {
                                pids[i] = pid;

                                if (i == 0)
                                {
                                        close(pipefd[0]);

                                        /*
                                        if (strstr(c->commands[i], "<"))
                                        {
                                                int input_file;
                                                char *f;
                                                int j = 0;
                                                while(c->args[i][i] != NULL)
                                                {
                                                        if (strcmp(c->args[i][j], "<") == 0)
                                                        {
                                                                // what if file is missing
                                                                file = strdup(c->args[i][j+1]);
                                                                break;
                                                        }
                                                }

                                                input_file = open(f, O_RDONLY);
                                                if (input_file == -1)
                                                {
                                                        perror("open first input file");
                                                        exit(1);
                                                }

                                                close(STDIN_FILENO);
                                                dup2(input_file, STDIN_FILENO);
                                                close(input_file);
                                        }

                                        if (strstr(c->commands[i], ">"))
                                        {
                                                int output_file;
                                                char *f;
                                                int j = 0;
                                                while(c->args[i][i] != NULL)
                                                {
                                                        if (strcmp(c->args[i][j], ">") == 0)
                                                        {
                                                                // what if file is missing
                                                                file = strdup(c->args[i][j+1]);
                                                                break;
                                                        }
                                                }

                                                output_file = open(f, O_WRONLY);
                                                if (input_file == -1)
                                                {
                                                        perror("open first output file");
                                                        exit(1);
                                                }

                                                close(STDOUT_FILENO);
                                                dup2(output_file, STDOUT_FILENO);
                                                close(output_file);

                                        }
                                        */

                                        close(STDOUT_FILENO);
                                        dup2(pipefd[1], STDOUT_FILENO);
                                        saved_pipe = pipefd[1];
                                }

                                if (i == c->npipes)
                                {
                                        close(STDIN_FILENO);
                                        dup2(saved_pipe, STDIN_FILENO);
                                }
                                execvp(c->args[i][0], c->args[i]);
                        }

                }

                for (i = 0; i < c->ncommands; i++) 
                {
                        int status;

                        (void)waitpid(pids[i], &status, 0);
                }


        }
        return 0;
}
