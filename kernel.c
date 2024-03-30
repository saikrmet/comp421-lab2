#include <stdlib.h>
#include <stdio.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include <comp421/loadinfo.h>

#include "memory.h"
#include "pcb.h"
#include "handleProcesses.h"
#include "pageTableController.h" 
#include "trapHandling.h"
#include "load.h"
#include "terminalHandler.h"
#include "contextSwitch.h"



void KernelStart(ExceptionInfo *info, unsigned int pmem_size, void *orig_brk, char **cmd_args){

    int initActive = 1;

    startBrk(orig_brk);
    
    createPhysicalPages(pmem_size);

    markOccupied((void*) KERNEL_STACK_BASE, (void*) KERNEL_STACK_LIMIT);

    void** interVecTable = malloc(sizeof(void*) * TRAP_VECTOR_SIZE);
    int i;
    for (i = 0; i < TRAP_VECTOR_SIZE; i++){
        if (i == TRAP_TTY_RECEIVE) {
            interVecTable[i] = trap_tty_receive_handler;
        } else if (i == TRAP_TTY_TRANSMIT) {
            interVecTable[i] = trap_tty_transmit_handler;
        } else if (i == TRAP_MATH) {
            interVecTable[i] = trap_math_handler;
        } else if (i == TRAP_KERNEL) {
            interVecTable[i] = trap_kernel_handler;
        } else if (i == TRAP_CLOCK) {
            interVecTable[i] = trap_clock_handler;
        } else if (i == TRAP_ILLEGAL) {
            interVecTable[i] = trap_illegal_handler;
        } else if (i == TRAP_MEMORY) {
            interVecTable[i] = trap_memory_handler;
        } else {
            interVecTable[i] = NULL;
        }
    }

    //REG_VECTOR_BASE privileged machine register points to interrupt vector table
    WriteRegister(REG_VECTOR_BASE, (RCS421RegVal) interVecTable);

    //Build page table for region 1 and write to register
    struct pte* kernelPT = initializePageTables();
    WriteRegister(REG_PTR1, (RCS421RegVal) kernelPT);

    initializePTEntry();

    //Create idle process
    struct pcbStruct* idle = createPcb(0, -1, NULL);
    createWaitProcess(idle);
    //Build page table for region 0 and write to register
    WriteRegister(REG_PTR0, (RCS421RegVal) idle->pcbPT);

    startVM();
    openPageSpace();
    createBufs();

    //Create init process
    struct pcbStruct* init = createPcb(1, -1, idle);
    forkPcb(idle, init);

    if (initActive){
        initActive = 0;
        if (!cmd_args[0]) {
            char* loadInputs[2] = {"init", NULL};
            if (LoadProgram(loadInputs[0], loadInputs, init, info) != 0){
            	printf("Error encountered in loading program");
            	Halt();
            }
        } else {
            if (LoadProgram(cmd_args[0], cmd_args, init, info) != 0){
            	printf("Error encountered in loading program");
            	Halt();
            }
        }
    }
}