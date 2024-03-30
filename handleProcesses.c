#include <stdlib.h>
#include <stdio.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include "trapHandling.h"
#include "pcb.h"
#include "pageTableController.h"
#include "contextSwitch.h"
#include "handleProcesses.h"


int currPid = 2; 
struct pcbEntry *start = NULL;
struct pcbEntry *waitEntry = NULL;
struct pcbStruct* waitPcb;
struct pcbEntry* exitProcess = NULL;
int clockTick = -1;

int isWaiting = 0;

void switchIdle(struct pcbStruct* newPcb);
void createWaitProcess(struct pcbStruct* pcb) {
    waitEntry = malloc(sizeof(struct pcbEntry));
    waitPcb = pcb;
    waitEntry->data = waitPcb;
    waitEntry->next = NULL;
}
int checkBlocking(struct pcbStruct* entry) {
    return entry->callRead != -1 || entry->callProduce != -1 || entry->termProducing != -1 || entry->termReading != -1 || entry->blockProcess != 0 || entry->delayTicks >= 1;
}
int createClockTickPid() {
    int new_pid;
    if (isWaiting == 1) {
        new_pid = waitPcb->pid;
    } else {
        new_pid = start->data->pid;
    }
    int isEqual = new_pid == clockTick;
    clockTick = new_pid;
    return isEqual;
}

int minusDelay() {
    struct pcbEntry* entry = start;
    int noDelay = 0;
    while (entry) {
        if (entry->data->delayTicks >= 1) {
            entry->data->delayTicks -= 1;
            if (entry->data->delayTicks == 0) {
                noDelay = 1;
            }
        }
        entry = entry->next;
    }
    return noDelay;
}

void cleanExitProcess() {
    if (!exitProcess) {
        return;
    }
    deletePT(exitProcess->data->pcbPT);
    // free page table and process.
    free(exitProcess->data);
    free(exitProcess);
    exitProcess = NULL;
}

void createProcess(int terminate) {
    if (terminate) {
        isWaiting = 1;
        clockTick = 0;
        struct pcbEntry* entry = start;
        start = start->next;
        changePcb(waitPcb, start->data);
    }
    else {
        if (!checkBlocking(start->data) && isWaiting == 1) {
            isWaiting = 0;
            changePcb(waitPcb, start->data);
        
            return;
        }
        if (!(start->next)) {
            switchIdle(start->data);
            return;
        }
        struct pcbEntry* process = start->next;
        while (process) {
            int not_blocked = !checkBlocking(process->data);
            if (not_blocked) {
                break;
            }
            process = process->next;
        }
        if (!process) {
            switchIdle(getActivePcb()->data);
            return;
        }
        struct pcbEntry* curr_entry = start;
        struct pcbEntry* activePcb = getActivePcb();
        while (curr_entry->next) {
            curr_entry = curr_entry->next;
        }
        while (process != start) {
            curr_entry->next = start;
            curr_entry = curr_entry->next;
            start = start->next;
            curr_entry->next = NULL;
        }
        isWaiting = 0;
        changePcb(activePcb->data, start->data);
        return;
    }
}

void handleExitProcess() {
    if (exitProcess) {
        Halt();
    }
    exitProcess = start;
    if (!exitProcess->next) {
        Halt();
    }
    createProcess(1);
}

void addPcbEntry(struct pcbStruct *pcb) {
    struct pcbEntry *newpcb = malloc(sizeof(struct pcbEntry));
    newpcb->data = pcb;
    // set the next to current process running right now. 
    newpcb->next = start;
    start = newpcb; 
}

// get the pid from current pcb 
int getCurrPid() {
    if (!isWaiting) {
        return start->data->pid; 
    }
    return waitPcb->pid;
}


struct pcbEntry* getStartingPcb() {
    return start;
}

struct pcbEntry* getActivePcb() {
    if (!isWaiting) {
        return start;
    } 
    return waitEntry;
}

int popNewPid() {
    currPid++;
    return currPid;
}

void activateProducer(int produce) {
    struct pcbEntry *entry = getStartingPcb();
    while (entry) {
        struct pcbStruct *new_pcb = entry->data;
        if (new_pcb->callProduce == produce) {
            new_pcb->callProduce = - 1;
            return;
        }
        entry = entry->next;
    }
    return;
}

void activateRead(int read) {
    struct pcbEntry *entry = getStartingPcb();
    while (entry) {
        struct pcbStruct *new_pcb = entry->data;
        if (new_pcb->callRead == read) {
            new_pcb->callRead = -1;
            return;
        }
        entry = entry->next;
    }
    return;
}


struct pcbStruct* getProducerProcess(int id) {
    struct pcbEntry *starting = getStartingPcb();

    while (starting) {
        struct pcbStruct *new_pcb = starting->data;
        if (new_pcb->termProducing == id) {
            return new_pcb;
        }
        starting = starting->next;
    }
    return NULL;
}




void switchIdle(struct pcbStruct* newPcb) {
    if (isWaiting != 0) {
        if (waitPcb == newPcb) {
            return;
        }
        int not_blocked = !checkBlocking(newPcb);
        if (not_blocked) {
            isWaiting = 0;
            changePcb(waitPcb, newPcb);
        }
    }
    else {
        if (checkBlocking(newPcb)) {
            isWaiting = 1;
            changePcb(newPcb, waitPcb);
        }
    }
}
