#include <stdio.h>
#include <comp421/hardware.h>

/**
 * Idle program for context switching, where process ID is 0.
*/
int main(){
    while(1) {
        Pause();
    }
}