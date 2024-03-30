#include <comp421/yalnix.h>
#include <stdio.h>

int
main()
{
    int x;

    /*
     *  The address 0x123 should be invalid, so this should result
     *  in a TRAP_MEMORY trap.
     */
    x = *(int *)0x123;
    printf("%d", x);

    Exit(1);
}
