#include <comp421/yalnix.h>

void
force(char *addr)
{
    *addr = 42;
}

int
main()
{
    char big_buffer[20*1024];
    unsigned int i;
    for (i = 0; i < sizeof(big_buffer); i++) 
	force(big_buffer + i);

    Exit(0);
}
