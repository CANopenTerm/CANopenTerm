#include <stdio.h>

int fred()
{
    printf("fred\n");
    return 0;
}

int joe()
{
    printf("joe\n");
    return 1;
}

printf("%d\n", fred() && joe());
printf("%d\n", fred() || joe());
printf("%d\n", joe() && fred());
printf("%d\n", joe() || fred());
printf("%d\n", fred() && (1 + joe()));
printf("%d\n", fred() || (0 + joe()));
printf("%d\n", joe() && (0 + fred()));
printf("%d\n", joe() || (1 + fred()));
