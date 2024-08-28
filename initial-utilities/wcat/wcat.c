#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
        int files;
        char *path;
        FILE *f;
        char buf[1024] = {0};

        if (!(argc - 1))
        {
                exit(0);
        }
        files = 1;
        while(files != argc)
        {
                path = strdup(argv[files%argc]);
                if (path == NULL)
                {
                        perror("failed to get file path");
                        exit(1);
                }

                f = fopen(path, "r");
                if (f == NULL)
                {
                        printf("wcat: cannot open file\n");
                        exit(1);
                }

                while(1)
                {
                        size_t b = fread(buf, 1, sizeof(buf) - 1, f);
                        if (b == 0) break;
                        if (b == -1)
                        {
                                printf("error reading file\n");
                                exit(1);
                        }
                        buf[sizeof(buf) -1] = '\0';
                        printf("%s", buf);
                        memset(buf, 0, sizeof(buf));
                }
                files++;
        }
        return 0;
}
