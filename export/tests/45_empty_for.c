#include <stdio.h>

int Count = 0;

for (;;)
{
    Count++;
    printf("%d\n", Count);
    if (Count >= 10)
        break;
}
