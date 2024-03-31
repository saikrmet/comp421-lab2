#include "contextSwitch.h"
#include "pcb.h"
#include "pageTableController.h"
#include "handleProcesses.h"
#include "memory.h"

void* vToP(void *addr);

void changeReg0PT(void* newReg0PT) {
	WriteRegister(REG_PTR0, (RCS421RegVal) vToP(newReg0PT));
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
}

// change from virtual address to physical address 
void* vToP(void *addr) {
    // we need to get the virtual page base address 
    void* virtual_address = (void*)DOWN_TO_PAGE(addr);
    int v_pfn;
    if (virtual_address >= (void*)VMEM_1_BASE) {
        v_pfn = page_table[((long)virtual_address - VMEM_1_BASE) / PAGESIZE].pfn;
    } else {
        struct pcbEntry *currProcess = getStartingPcb();
        v_pfn = currProcess->data->pcbPT[((long)virtual_address) / PAGESIZE].pfn;
    }
    // procure the physical address needed to offset 
    void* physical_address = (void*) (long)(v_pfn * PAGESIZE);

    // add the addr given & PAGEOFFSET to get the offset 
    return (void *) (((long) physical_address) + ((long)addr & PAGEOFFSET));
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
	TracePrintf(1, "change pcb \n");
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
	TracePrintf(1, "children before context switch: %d, pid: %d\n", getActivePcb()->data->children, getActivePcb()->data->pid );
    ContextSwitch(forkPCBFunc, &activePCB->sc, (void*) activePCB, (void*) newPCB);
	TracePrintf(1, "children after context switch: %d, pid: %d\n", getActivePcb()->data->children, getActivePcb()->data->pid );
	cleanExitProcess();
}