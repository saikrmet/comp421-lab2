#include <stdio.h>
#include <stdlib.h>
#include <comp421/yalnix.h>

void
recurse(char *who, int i)
{
    char waste[10240];	/* use up stack space in the recursion */
    //char *mem = (char *)malloc(2048); /* use up heap space */
    Delay(1);
    //printf("mem %p\n", mem);

    /*for (j = 0; j < 1024; j++) 
	waste[j] = 'a';

	waste[0] = waste[1];*/

    printf("%s %d\n", who, i);
    if (i == 0)
    {
	printf("Done with recursion\n");
	return;
    }
    else
	recurse(who, i - 1);
}

int
main(int argc, char **argv)
{
    //int pid;

    setbuf(stdout, NULL);

    printf("BEFORE\n");

    /*if ((pid = Fork()) == 0)
    {
	printf("CHILD\n");
	recurse("child", 33);
    }
    else
    {*/
	//printf("PARENT: child pid = %d\n", pid);
	printf("Before recursion\n");
	recurse("parent", 90);//33);
	printf("After recursion\n");
    //}

    Delay(0);
    printf("Finished first delay, starting second ...\n");
    Delay(1);
    printf("Finished second delay, starting exit ...\n");
    Exit(0);
}
