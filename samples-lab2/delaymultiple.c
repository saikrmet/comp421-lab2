#include <stdio.h>
#include <stdlib.h>
#include <comp421/yalnix.h>
#include <comp421/hardware.h>

int
main(int argc, char **argv)
{
    TracePrintf(0, "Delaying for %d ticks in each iter\n", Delay(atoi(argv[1])));
    int i;
    for (i = 0; i < 3; ++i) {
        fprintf(stderr, "Starting delay num%d\n", i);
        Delay(atoi(argv[1]));
        fprintf(stderr, "Delay finished!\n");
    }
    

    Exit(0);
}
