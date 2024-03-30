#pragma once
#include <comp421/hardware.h>
#include <comp421/yalnix.h>

struct pcbStruct
{
  int pid;
  struct pte *pcbPT;
  SavedContext sc;
  int delayTicks;
  void *brk;
  void *stackSize;
  int blockProcess;
  int children;
  int parentPid;
  int termReading;
  int callRead;
  int termProducing;
  int callProduce;
  struct terminateEntry *terminateProcess; 
};

struct terminateEntry
{
    int status;
    int pid;
    struct terminateEntry *next;
};

struct pcbStruct* fetchPcb(int old_pid);
struct pcbStruct* createPcb(int pid, int parentPid, struct pcbStruct* pcb);
struct terminateEntry* removeTerminatedEntry(struct pcbStruct* data);
void addTerminatedEntry(struct pcbStruct* pcb, int pid, int status);
