#include <stdio.h>

void fred(int x)
{
    switch (x)
    {
        case 1: printf("1\n"); return;
        case 2: printf("2\n"); break;
        case 3: printf("3\n"); return;
    }

    printf("out\n");
}

fred(1);
fred(2);
fred(3);
