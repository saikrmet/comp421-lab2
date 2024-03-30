#include <stdio.h>
#include "handleProcesses.h"
#include "pcb.h"
#include "pageTableController.h"
#include "memory.h"


// create an empty pagesize space to swap the pages 
void* createPageSpace;
// has the virtual memory been started yet
int vm_enabled = 0;
// total physical pages. 
int p_pages = -1;
// check if the physical page is occupied or not
int *p_page_occ = NULL; 
// set pointer to virtual memory 1 
void *user_brk = (void*)VMEM_1_BASE;

void createPhysicalPages(unsigned int page_length) {
    TracePrintf(1, "create physical pages\n");
    p_pages = page_length / PAGESIZE;
    p_page_occ = malloc(sizeof(int) * p_pages);
    memset(p_page_occ, 0, p_pages);

    markOccupied((void*) VMEM_1_BASE, user_brk);
}

void startBrk(void *oldBrk) {
    user_brk = oldBrk;
}

void* getBrk() {
    return user_brk;
}

void markOccupied(void* ptr1, void* ptr2) {
    int c;
    for (c = DOWN_TO_PAGE(ptr1) / PAGESIZE; c < UP_TO_PAGE(ptr2) / PAGESIZE; c++){
        p_page_occ[c] = 1;
    }
}

// since we need to use the number of free pages
// multiple times throughout this file. 
int numFreePages() {
    int num = 0;
    int c;
    for (c = 0; c < p_pages; c++) {
        if (p_page_occ[c] == 0) {
            num++;
        }
    }
    return num;
}

unsigned int findPhysPage(){
    int page_index;
    if (vm_enabled) {
        page_index = 0;
    } else {
        page_index = MEM_INVALID_PAGES;
    }
    int c;
    for (c = page_index; c < p_pages; c++) {
        if (p_page_occ[c] == 0) {
            p_page_occ[c] = 1;
            return c;
        }
    }
    printf("unable to grab physical page");
    Halt();
}

unsigned int recentFreePP() {
    if (p_page_occ[DOWN_TO_PAGE(VMEM_1_LIMIT - 1) / PAGESIZE] == 1) {
        printf("unable to get most recent physical page");
        Halt();
    }
    p_page_occ[DOWN_TO_PAGE(VMEM_1_LIMIT - 1) / PAGESIZE] = 1;
    return DOWN_TO_PAGE(VMEM_1_LIMIT - 1) / PAGESIZE;
}

// free the physical page 
void freePP(unsigned int idx){
	if(p_page_occ[idx] == 0){
        printf("can't free already freed page");
		Halt();
	}
	p_page_occ[idx] = 0;
}

void brkHandler(ExceptionInfo *exInfo) {
    void *addr = (void*)exInfo->regs[1];
    if (MEM_INVALID_SIZE >= UP_TO_PAGE(addr)) {
        exInfo->regs[0] = ERROR;
        return; 
    }
	struct pcbEntry *process = getActivePcb();
	struct pcbStruct *block = process->data;
	void *k_brk = block->brk;
	void *stack = block->stackSize;

	struct pte *user_page_table = block->pcbPT;



    // account for addresses that aren't valid

    if (DOWN_TO_PAGE(stack) - 1 <= UP_TO_PAGE(addr)){
        exInfo->regs[0] = ERROR;
        return; 
    }

    if (UP_TO_PAGE(k_brk) > UP_TO_PAGE(addr)) {
        // here we want to loop through the page addresses and
        // set valid to false 
        // free the current page 
        // and lastly write to the register to flush the TLB
        int c;
        for (c = 0; c < (long)UP_TO_PAGE(k_brk) - (long)UP_TO_PAGE(addr) / PAGESIZE; c++) {
            user_page_table[(long)UP_TO_PAGE(k_brk) / PAGESIZE - 1 - c].valid = 0; 
            freePP(user_page_table[(long)UP_TO_PAGE(k_brk) / PAGESIZE - 1 - c].pfn);
            WriteRegister(REG_TLB_FLUSH, (RCS421RegVal)((long)UP_TO_PAGE(k_brk) / PAGESIZE - 1 - c));
        }
    } else if (UP_TO_PAGE(k_brk) < UP_TO_PAGE(addr)) {
		// not enough physical memory
		if(numFreePages() < ((long)UP_TO_PAGE(addr) - (long)UP_TO_PAGE(k_brk)) / PAGESIZE) {
			exInfo->regs[0] = ERROR;
			return;
		} else {
            int c;
			for(c = 0; c < ((long)UP_TO_PAGE(addr) - (long)UP_TO_PAGE(k_brk)) / PAGESIZE; c++) {
				user_page_table[c + (long)UP_TO_PAGE(k_brk) / PAGESIZE].valid = 1;
				user_page_table[c + (long)UP_TO_PAGE(k_brk) / PAGESIZE].pfn = findPhysPage();
			}
        }
    }

    block->brk = (void*)UP_TO_PAGE(addr);
    exInfo->regs[0] = 0;
}

int SetKernelBrk(void *addr) {
    TracePrintf(1, "starting setkernelbrk\n");
    if (vm_enabled == 1) {
        if ((((long)UP_TO_PAGE(addr) - (long)user_brk) / PAGESIZE) <= numFreePages()) {
            int c;
            for (c = 0; c < ((long)UP_TO_PAGE(addr) - (long)user_brk) / PAGESIZE; c++) {
                unsigned int pp = findPhysPage();
                if (page_table[c + ((long)user_brk - VMEM_1_BASE) / PAGESIZE].valid == 1) {
                    Halt();
                }
                page_table[c + ((long)user_brk - VMEM_1_BASE) / PAGESIZE].valid = 1;
                page_table[c + ((long)user_brk - VMEM_1_BASE) / PAGESIZE].pfn = pp;
            }
            if(p_page_occ != NULL){
                markOccupied(user_brk, addr);
            }
            user_brk = (void*)UP_TO_PAGE(addr);
        } else {
            //  return -1 when
            // run out of physical memory or otherwise be unable to satisfy the requirements
            return -1; 
        }
    } else {
        if ((long)user_brk - PAGESIZE >= (long)addr) return -1;
        if (p_page_occ != NULL){
                markOccupied(user_brk, addr);
        }
        user_brk = (void*)UP_TO_PAGE(addr);
    }
    return 0;
}

void startVM() {
    WriteRegister(REG_VM_ENABLE, 1);
    vm_enabled = 1; 
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

void openPageSpace() {
    createPageSpace = malloc(PAGESIZE);
}

void* getCreatePageSpace() {
    return createPageSpace;
}

// curr > brk
// new < curr
// VMEM_0_LIMIT > (unsigned long) addr
// MEM_INVALID_SIZE < (unsigned long) addr
// diff > 
int growUserProcessStack(ExceptionInfo *exInfo, struct pcbEntry *entry) {
    TracePrintf(1, "user process grow start\n");
    unsigned long temp_brk = UP_TO_PAGE(entry->data->brk) / PAGESIZE;
    void* addr = exInfo->addr;
    unsigned long curr = DOWN_TO_PAGE(entry->data->stackSize) / PAGESIZE;
    unsigned long new = DOWN_TO_PAGE(addr) / PAGESIZE;
    int diff = curr - new;
    if (curr > temp_brk && new < curr && VMEM_0_LIMIT > (unsigned long) addr && MEM_INVALID_SIZE < (unsigned long) addr && diff <= numFreePages()) {
        int c;
        for (c = 0; c < diff; c++) {
            entry->data->pcbPT[curr - 1 - c].pfn = findPhysPage();
            entry->data->pcbPT[curr - 1 - c].uprot = PROT_READ | PROT_WRITE;
            entry->data->pcbPT[curr - 1 - c].valid = 1;
            WriteRegister(REG_TLB_FLUSH, (RCS421RegVal)(curr - 1 - c));
        }
        entry->data->stackSize = (void*)DOWN_TO_PAGE(addr);
        return 1;
    } else {
        return 0;
    }

}





