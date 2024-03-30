#include <stdlib.h>
#include <stdio.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#pragma once


struct pcbEntry {
    struct pcbStruct *data;
    struct pcbEntry *next;
};

void createWaitProcess(struct pcbStruct* pcb);
int createClockTickPid();
int minusDelay();
void cleanExitProcess();
void handleExitProcess();
void addPcbEntry(struct pcbStruct *pcb);
int getCurrPid();
struct pcbEntry* getStartingPcb();
struct pcbEntry* getActivePcb();
int popNewPid();
void activateProducer(int produce);
void activateRead(int read);
struct pcbStruct* getProducerProcess(int id);
void createProcess(int terminate);