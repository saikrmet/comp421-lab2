#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include <stdlib.h>
#pragma once

struct bufStruct *bufs;

struct bufStruct {
    char term_buffer[TERMINAL_MAX_LINE];
    int size;
    int head;
    int tail;
    
};
int newline(int terminal);
void createBufs();
int read(char *buffer, int terminal, int size);
int write(char *buffer, int terminal, int blocked, int size);
