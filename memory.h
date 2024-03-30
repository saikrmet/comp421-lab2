#include <string.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include <stdlib.h>



void createPhysicalPages(unsigned int page_length);
void startBrk(void *oldBrk);
void* getBrk();
void markOccupied(void* ptr1, void* ptr2);
int numFreePages();
void startVM();
int growUserProcessStack(ExceptionInfo *exInfo, struct pcbEntry *head);
unsigned int findPhysPage();
unsigned int recentFreePP();
void freePP(unsigned int idx);
int SetKernelBrk(void *addr);
void brkHandler(ExceptionInfo *exInfo);
void openPageSpace();
void* getCreatePageSpace();
void* vToP(void *addr);
