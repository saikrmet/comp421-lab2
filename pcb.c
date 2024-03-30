#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include "pageTableController.h"
#include "handleProcesses.h"
#include "pcb.h"


struct pcbStruct* fetchPcb(int old_pid) {
    struct pcbEntry *start = getStartingPcb();
    while (start) {
        // if pid is the same then catch and return the pcb 
        if (start->data->pid == old_pid) return start->data; 
        start = start->next;
    }
    // couldn't catch anything, just return null
    return NULL;
}

struct pcbStruct* createPcb(int pid, int parentPid, struct pcbStruct* pcb) {
    struct pcbStruct *entry = malloc(sizeof(struct pcbStruct));
    if (!entry) {
        // Handle memory allocation failure
        return NULL;
    }
    entry->terminateProcess = NULL;
    entry->delayTicks = 0;
    entry->children = 0;
    entry->blockProcess = 0;
    entry->callRead = -1;
    entry->callProduce = -1;
    entry->termProducing = -1;
    entry->termReading = -1;
    entry->pcbPT = initializePageTable();
    entry->pid = pid;
    entry->parentPid = parentPid;
    // implement page table functions here 
    if (pid != 0){
        // add to schedule
        addPcbEntry(entry);
        // update pages with the correct fields
        updatePages(entry->pcbPT);
        // update the stack size to the inputted pcb as well as the break
        entry->stackSize = pcb->stackSize;
        entry->brk = pcb->brk;
    }
    else {
        updateFirstPage(entry->pcbPT);
    }
    return entry;
}

struct terminateEntry* removeTerminatedEntry(struct pcbStruct* data) {
    struct terminateEntry *temp = data->terminateProcess;
    data->terminateProcess = temp->next;
    // need a temp variable to keep track of the old terminated process. 
    return temp; 
}

void addTerminatedEntry(struct pcbStruct* pcb, int pid, int status) {

    // grab the next entry 
    struct terminateEntry *temp = malloc(sizeof(struct terminateEntry));
    temp->next = NULL;
    temp->status = status;
    temp->pid = pid;

    // grab current entry 
    struct terminateEntry *entry = pcb->terminateProcess;
    if (!entry) {
        pcb->terminateProcess = temp;
    } else {
        while (entry->next) {
            entry = entry->next;
        }
        entry->next = temp; 
    }
}
