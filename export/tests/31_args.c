#include <stdio.h>

int test(int argc, char **argv)
{
    int Count;

    printf("hello world %d\n", argc);
    for (Count = 0; Count < argc; Count++)
        printf("arg %d: %s\n", Count, argv[Count]);

    return 0;
}

char* argv[] = {"-", "arg1", "arg2", "arg3", "arg4"};

test(5, argv);
