#include <comp421/yalnix.h>
#include <stdio.h>

int
main()
{
    int x = 0;
    int y;

    /*
     *  Dividing by zero here should result in a TRAP_MATH trap.
     */
    y = 5/x;
    printf("%d", y);

    Exit(1);
}
