#include "contextSwitch.h"
#include "pcb.h"
#include "pageTableController.h"
#include "handleProcesses.h"
#include "memory.h"

void changeReg0PT(void* newReg0PT) {
	WriteRegister(REG_PTR0, (RCS421RegVal) vToP(newReg0PT));
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
}

void forkMemory(struct pte* pt1, struct pte* pt2) {
	int temp_uprot = 0;
	int temp_kprot = 0;
	long pageNum = 0;
	void* pageSpace = getCreatePageSpace();

	while (pageNum < VMEM_0_LIMIT / PAGESIZE) {
		if (pt1[pageNum].valid != 0) {
			memcpy(pageSpace, (void*) (pageNum * PAGESIZE), PAGESIZE);
			temp_uprot = pt1[pageNum].uprot;
			temp_kprot = pt1[pageNum].kprot;
			changeReg0PT(pt2);

			pt2[pageNum].pfn = findPhysPage();
			pt2[pageNum].valid = 1;
			memcpy((void*) (pageNum * PAGESIZE), pageSpace, PAGESIZE);
			pt1[pageNum].uprot = temp_uprot;
			pt1[pageNum].kprot = temp_kprot;
			changeReg0PT(pt1);
		}
		pageNum++;
	}
}

SavedContext *changePCBFunc(SavedContext *ctxp, void *p1, void *p2) {
	struct pcbStruct* pcb1 = (struct pcbStruct*) p1;
	struct pcbStruct* pcb2 = (struct pcbStruct*) p2;
	changeReg0PT(pcb2->pcbPT);
	return &pcb2->sc;
}

void changePcb(struct pcbStruct* activePCB, struct pcbStruct* newPCB) {
    ContextSwitch(changePCBFunc, &activePCB->sc, (void*) activePCB, (void*) newPCB);
	cleanExitProcess();
}

SavedContext *forkPCBFunc(SavedContext *ctxp, void *p1, void *p2){
	struct pcbStruct* pcb1 = (struct pcbStruct*) p1;
	struct pcbStruct* pcb2 = (struct pcbStruct*) p2;
	forkMemory(pcb1->pcbPT, pcb2->pcbPT);
	changeReg0PT(pcb2->pcbPT);
	return &pcb1->sc;
}

void forkPcb(struct pcbStruct* activePCB, struct pcbStruct* newPCB) {
    ContextSwitch(forkPCBFunc, &activePCB->sc, (void*) activePCB, (void*) newPCB);
	cleanExitProcess();
}