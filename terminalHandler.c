#include <stdlib.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>

#include "trapHandling.h"
#include "handleProcesses.h"
#include "pcb.h"
#include "terminalHandler.h"

// create a helper for read to go thru the buf and see if theres a new line
int newline(int terminal) {
    int head = bufs[terminal].head;
    int c;
    for (c = 0; c < bufs[terminal].size; c++) {
        if (bufs[terminal].term_buffer[head] == '\n') return 1;
        head = (1+head) % TERMINAL_MAX_LINE;
    }
    return 0;
}

void createBufs() {
    // allocate enough so you need to multiply total terminals by size 
    bufs = malloc(NUM_TERMINALS * sizeof(struct bufStruct));
    int c;
    for (c = 0; c < NUM_TERMINALS; c++) {
        bufs[c].head = 0;
        bufs[c].tail = 0;
    }
}

int readBuf(char *buffer, int terminal, int size) {
    struct pcbStruct *pcb = getActivePcb()->data;
    // loop thru until newline
    while (!newline(terminal)) {
        pcb->callRead = terminal;
        createProcess(0);
    }
    int chars = 0;
    int head = bufs[terminal].head;
    int c;
    for (c = 0; c < size; c++) {
        if (bufs[terminal].size < 1) {
            break;
        }
        else {
            buffer[c] = bufs[terminal].term_buffer[head];
            head = (1+head) % TERMINAL_MAX_LINE;
            bufs[terminal].size -= 1;
            chars += 1;
        }
    }
    return chars;
}


int writeBuf(char *buffer, int terminal, int blocked, int size) {
    if (blocked) {
        while (!getProducerProcess(terminal)) {
            struct pcbStruct *pcb = getActivePcb()->data;
            pcb->callProduce = terminal;
            createProcess(0);
        }
    }
    int chars = 0;
    int c;
    for (c = 0; c < size; c++) {
        if (bufs[terminal].size != TERMINAL_MAX_LINE) {
            bufs[terminal].size += 1;
            int produce = bufs[terminal].tail;
            bufs[terminal].term_buffer[produce] = buffer[c];
            bufs[terminal].tail = (1 + bufs[terminal].tail) % TERMINAL_MAX_LINE;
            chars += 1;
        }
    }
    return chars;
}

