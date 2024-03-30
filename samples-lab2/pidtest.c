#include <stdio.h>
#include <comp421/yalnix.h>
#include <comp421/hardware.h>

int
main()
{
    int x = GetPid();
    TracePrintf(0, "Pid returned: %d\n", x);

    Exit(0);
}
