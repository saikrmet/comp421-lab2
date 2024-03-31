#include <string.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include <stdlib.h>
#include "handleProcesses.h"
#pragma once


void startBrk(void *oldBrk);
void* getBrk();
void createPhysicalPages(unsigned int page_length);
int numFreePages();
void markOccupied(void* ptr1, void* ptr2);
unsigned int findPhysPage();
unsigned int recentFreePP();
void freePP(unsigned int idx);
void brkHandler(ExceptionInfo *exInfo);
void startVM();
void openPageSpace();
void* getCreatePageSpace();
int growUserProcessStack(ExceptionInfo *exInfo, struct pcbEntry *head);
