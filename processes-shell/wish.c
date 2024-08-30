#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

void parse_cmd_line(FILE *in, char **command)
{
        char **args;
        size_t len = 0;
        size_t argsn = 0;
        size_t r = getline(command, &len, in);
        if (r == -1)
        {
                printf("failed to get command line\n");
                exit(1);
        }
        (*command)[r-1] = '\0';

        char *token;
        char *temp = strdup(*command);
        if (temp == NULL)
        {
                printf("strdup command\n");
                exit(1);
        }

        while((token = strsep(command, " ")) != NULL)
        {
                argsn++;
        }
        args = malloc(argsn - 1);
        if (args == NULL)
        {
                printf("malloc args\n");
                exit(1);
        }

        int i = 0;
        while((token = strsep(&temp, " ")) != NULL)
        {
                if (i == 0)
                {
                        i++;
                } else 
                {
                        (args)[i-1] = strdup(token);
                        i++;
                }
        }

        for(size_t i = 0; i < (argsn-1); i++)
        {
                printf("%s\n", args[i]);
        }
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

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

                parse_cmd_line(in, &command);
        }
        return 0;
}
