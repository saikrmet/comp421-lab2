#include "trapHandling.h"
#include "terminalHandler.h"
#include "handleProcesses.h"
#include "pcb.h"
#include "load.h"
#include "memory.h"
#include "pageTableController.h"
#include "contextSwitch.h"

void trap_kernel_handler(ExceptionInfo *exInfo) {

    int ex_type = exInfo->code;
    int currPid = getCurrPid();

    if (ex_type == YALNIX_GETPID) {
        get_pid_handler(exInfo);
    } else if (ex_type == YALNIX_WAIT) {
        wait_handler(exInfo);
    } else if (ex_type == YALNIX_EXIT) {
        exit_handler(exInfo, 0);
    } else if (ex_type == YALNIX_EXEC) {
        exec_handler(exInfo);
    } else if (ex_type == YALNIX_FORK) {
        fork_handler(exInfo);
    } else if (ex_type == YALNIX_BRK) {
        brkHandler(exInfo);
    } else if (ex_type == YALNIX_DELAY) {
        delay_handler(exInfo);
    } else if (ex_type == YALNIX_TTY_READ) {
        tty_read_handler(exInfo);
    } else if (ex_type == YALNIX_TTY_WRITE) {
        tty_write_handler(exInfo);
    } else {
        TracePrintf(1, "Unknown kernel trap");
    }

}

void get_pid_handler(ExceptionInfo *exInfo) {
    exInfo->regs[0] = getCurrPid();
}

void wait_handler(ExceptionInfo *exInfo) {
    TracePrintf(1, "wait handler \n");
    int* user_args = (int*) exInfo->regs[1];
    struct pcbEntry* activeProcess = getActivePcb();
    struct pcbStruct* prevPCB = activeProcess->data;
    //TracePrintf(1, "data: %d\n", prevPCB->children);
    //TracePrintf(1, "data: %d\n", prevPCB->terminateProcess == NULL);
    if (prevPCB->children == 0) {
        if (prevPCB->terminateProcess == NULL) {
            TracePrintf(1, "exiting, error in wait handler\n");
            TracePrintf(1, "the pid of error: %d\n", prevPCB->pid);
            exInfo->regs[0] = ERROR;
            return;
        }
    } else {
        TracePrintf(1, "terminate process: %d\n", prevPCB->terminateProcess);
        if (prevPCB->terminateProcess == NULL) {
            prevPCB->blockProcess = 1;
            createProcess(0);
        }
    }

    TracePrintf(1, "wait handler no issues, got through \n");
    struct terminateEntry* exitProcess = removeTerminatedEntry(prevPCB);
    exInfo->regs[0] = exitProcess->pid;
    *user_args = exitProcess->status;
    free(exitProcess);
}

void exit_handler(ExceptionInfo *exInfo, int wasProgramErr) {
    int currPid = getCurrPid();

    //Idle has exited
    if (currPid == 0) {
        Halt();
    }

    struct pcbEntry* activeProcess = getActivePcb();
    struct pcbStruct* prevPCB = activeProcess->data;
    int prevPid = prevPCB->parentPid;

    int status;
    if (wasProgramErr) {
        status = ERROR;
    } else {
        status = (int) exInfo->regs[1];
    }

    if (prevPid != -1) {
        prevPCB->blockProcess = 0;
        prevPCB->children--;
        addTerminatedEntry(prevPCB, currPid, status);
    }

    while (activeProcess != NULL) {
        if (activeProcess->data->parentPid == currPid) {
            activeProcess->data->parentPid = -1;
        }
        activeProcess = activeProcess->next;
    }
    handleExitProcess();
}

void exec_handler(ExceptionInfo *exInfo) {
    char* name = (char*) exInfo->regs[1];
    char** args = (char**) exInfo->regs[2];

    struct pcbEntry* start = getStartingPcb();
    TracePrintf(1, "before loading in \n");
    int loadVal = LoadProgram(name, args, start->data, exInfo);
    TracePrintf(1, "after loading with value %d \n", loadVal);

    if (loadVal == -2) {
        exit_handler(exInfo, 1);
    } else if (loadVal == -1) {
        exInfo->regs[0] = ERROR;
    }
}

void fork_handler(ExceptionInfo *exInfo) {
    TracePrintf(1, "fork called\n");
    struct pcbEntry* activeProcess = getActivePcb();
    struct pcbStruct* prevPCB = activeProcess->data;
    TracePrintf(1, "yeayeachildren, child: %d, pid: %d,\n", prevPCB->children, prevPCB->pid);

    int numPagesNeeded = KERNEL_STACK_PAGES + activePages(prevPCB->pcbPT);
    int numPagesFree = numFreePages();

    if (numPagesFree < numPagesNeeded) {
        exInfo->regs[0] = ERROR;
        return;
    }
    int nextPid = popNewPid();
    int currPid = getCurrPid();

   
    TracePrintf(1, "1prevPCB children: %d, pid: %d,\n", prevPCB->children, prevPCB->pid);
    TracePrintf(1, "2nextPid children: %d, pid: %d,\n", nextPid, prevPCB->pid);
    struct pcbStruct *nextPCB = createPcb(nextPid, currPid, prevPCB);
    activeProcess->data->children++;
    TracePrintf(1, "AFTERchildren, prevpcb: %d, pid: %d,\n", getActivePcb()->data->children, getActivePcb()->data->pid);
    TracePrintf(1, "AFTEREPREVPCBchildren, prevpcb: %d, pid: %d,\n", prevPCB->pid);
    forkPcb(prevPCB, nextPCB);
    TracePrintf(1, "children after forkpcb: %d\n", getActivePcb()->data->children);
    exInfo->regs[0] = (getCurrPid() != nextPid) ? nextPid : 0;
}

void delay_handler(ExceptionInfo *exInfo) {
    TracePrintf(1, "delay clock\n");
    int duration = exInfo->regs[1];

    //Invalid delay duration
    if (duration < 0) {
        exInfo->regs[0] = ERROR;
        return;
    }

    exInfo->regs[0] = 0;
    struct pcbEntry* start = getStartingPcb();
    struct pcbStruct* curr = start->data;
    curr->delayTicks = duration;

    if (duration > 0) {
        createProcess(0);
    }
    return;
}

void tty_read_handler(ExceptionInfo *exInfo) {
    int terminal = exInfo->regs[1];

    if (terminal >= 0 && terminal <= NUM_TERMINALS) {
        void* buffer = (void*) exInfo->regs[2];

        int size = exInfo->regs[3];
        int retNum = readBuf(buffer, terminal, size);

        exInfo->regs[0] = (retNum < 0) ? ERROR : retNum;

    } else {
        exInfo->regs[0] = ERROR;
    }
}

void tty_write_handler(ExceptionInfo *exInfo) {
    int terminal = exInfo->regs[1];

    if (terminal >= 0 && terminal <= NUM_TERMINALS) {
        void* buffer = (void*) exInfo->regs[2];
        int size = exInfo->regs[3];

        int retNum = writeBuf(buffer, terminal, 1, size);
        TtyTransmit(terminal, buffer, retNum);

        struct pcbEntry *activeProcess = getActivePcb();
        struct pcbStruct *curr = activeProcess->data;
        curr->termProducing = terminal;

        exInfo->regs[0] = (retNum < 0) ? ERROR : retNum;

    } else {
        exInfo->regs[0] = ERROR;
    }
}

void trap_tty_receive_handler(ExceptionInfo *exInfo) {
    int terminal = exInfo->code;
    char *chars = malloc(TERMINAL_MAX_LINE * sizeof(char));
    writeBuf(chars, terminal, 0, TtyReceive(terminal, chars, TERMINAL_MAX_LINE));

    if (newline(terminal)) {
        activateRead(terminal);
    }
}

void trap_tty_transmit_handler(ExceptionInfo *exInfo) {
    int terminal = exInfo->code;
    struct pcbStruct *pcb = getProducerProcess(terminal);
    if (pcb) {
        pcb->termProducing = -1;
    }
    activateProducer(terminal);
}


void trap_clock_handler(ExceptionInfo *exInfo) {
    TracePrintf(1, "trap clock \n");
    if ((minusDelay() == 1 && getCurrPid() == 0) || createClockTickPid()) {
        createProcess(0);
    }
    // decrement the "Delay ticks" of every process by 1

    // If the curent process has run for 2 clock ticks and another one is ready, switch to it
}

void trap_illegal_handler(ExceptionInfo *exInfo) {
    int terminal = exInfo->code;
    int currPid = getCurrPid();

    switch (terminal) {
        case TRAP_ILLEGAL_ILLOPC:
            printf("PID %d - Illegal Opcode\n", currPid);
            break;
        case TRAP_ILLEGAL_PRVOPC:
            printf("PID %d - Privileged Opcode\n", currPid);
            break;
        case TRAP_ILLEGAL_ILLOPN:
            printf("PID %d - Illegal Operand\n", currPid);
            break;
        case TRAP_ILLEGAL_ILLADR:
            printf("PID %d - Illegal Addr Mode\n", currPid);
            break;
        case TRAP_ILLEGAL_PRVREG:
            printf("PID %d - Privileged Reg\n", currPid);
            break;
        case TRAP_ILLEGAL_ILLTRP:
            printf("PID %d - Illegal Trap\n", currPid);
            break;
        case TRAP_ILLEGAL_COPROC:
            printf("PID %d - Coproc Error\n", currPid);
            break;
        case TRAP_ILLEGAL_BADSTK:
            printf("PID %d - Bad Stack\n", currPid);
            break;
        case TRAP_ILLEGAL_KERNELI:
            printf("PID %d - SIGILL from Kernel\n", currPid);
            break;
        case TRAP_ILLEGAL_KERNELB:
            printf("PID %d - SIGBUS from Kernel\n", currPid);
            break;
        case TRAP_ILLEGAL_USERIB:
            printf("PID %d - SIGILL/SIGBUS from User\n", currPid);
            break;
        case TRAP_ILLEGAL_ADRALN:
            printf("PID %d - Invalid Addr Align\n", currPid);
            break;
        case TRAP_ILLEGAL_ADRERR:
            printf("PID %d - No Physical Addr\n", currPid);
            break;
        case TRAP_ILLEGAL_OBJERR:
            printf("PID %d - HW Obj Error\n", currPid);
            break;
        default:
            printf("PID %d - Other Illegal Trap\n", currPid);
    }
    exit_handler(exInfo, 1);
}

void trap_math_handler(ExceptionInfo *exInfo) {
    int terminal = exInfo->code;
    int currPid = getCurrPid();

    switch (terminal) {
        case TRAP_MATH_FLTDIV:
            printf("Float div-zero, PID %d\n", currPid);
            break;
        case TRAP_MATH_FLTOVF:
            printf("Float overflow, PID %d\n", currPid);
            break;
        case TRAP_MATH_FLTUND:
            printf("Float underflow, PID %d\n", currPid);
            break;
        case TRAP_MATH_FLTRES:
            printf("Float inexact, PID %d\n", currPid);
            break;
        case TRAP_MATH_FLTINV:
            printf("Invalid float op, PID %d\n", currPid);
            break;
        case TRAP_MATH_FLTSUB:
            printf("FP subscript range, PID %d\n", currPid);
            break;
        case TRAP_MATH_INTDIV:
            printf("Int div-zero, PID %d\n", currPid);
            break;
        case TRAP_MATH_INTOVF:
            printf("Int overflow, PID %d\n", currPid);
            break;
        case TRAP_MATH_KERNEL:
            printf("SIGFPE from Kernel, PID %d\n", currPid);
            break;
        case TRAP_MATH_USER:
            printf("SIGFPE from user, PID %d\n", currPid);
            break;
        default:
            printf("Math error, PID %d\n", currPid);
    }
    exit_handler(exInfo, 1);
}

void trap_memory_handler(ExceptionInfo *exInfo) {
    struct pcbEntry* entry = getActivePcb();
    if (growUserProcessStack(exInfo, entry) != 1) {
        printf("Accessing memory illegally for process %d\n", entry->data->pid);
        exit_handler(exInfo, 1);
    }
}
