#include <stdio.h>
// inlcude needed headers .

// create an empty pagesize space to swap the pages 
void* createPageSpace;
// has the virtual memory been started yet
int vm_enabled = 0;
// total physical pages. 
int p_pages = -1;
// check if the physical page is free or not
int *p_page_free; 
// set pointer to virtual memory 1 
void *brk = (void*)VMEM_1_BASE;

void createPhysicalPages(unsigned int page_length) {
    p_page_free = malloc((page_length / PAGESIZE) * sizeof(int));
    memset(p_page_free, 0, (page_length / PAGESIZE));

    for (int c = DOWN_TO_PAGE((void*)VMEM_1_BASE) / PAGESIZE; c < UP_TO_PAGE(brk) / PAGESIZE; c++){
        p_page_free[c] = 1;
    }
}

void startBrk(void *oldBrk) {
    brk = oldBrk;
}

void* getBrk() {
    return brk;
}



// since we need to use the number of free pages
// multiple times throughout this file. 
int numFreePages() {
    int num = 0;
    for (int c = 0; c < p_pages; c++) {
        if (p_page_free[c] == 0) {
            num++;
        }
    }
    return num;
}

void startVM() {
    WriteRegister(REG_VM_ENABLE, 1);
    vm_enabled = 1; 
}

int growUserProcessStack(ExceptionInfo *info, struct pcbEntry *head) {
    void* addr = info->addr;
    unsigned long curr = DOWN_TO_PAGE(head->pcb->userStackLimit) / PAGESIZE;
    unsigned long new = DOWN_TO_PAGE(addr) / PAGESIZE;
    if (new < curr && curr > (unsigned long)(UP_TO_PAGE(head->pcb->brk) / PAGESIZE) && (unsigned long)addr < VMEM_0_LIMIT && (unsigned long)addr > MEM_INVALID_SIZE && curr - new <= numFreePages()) {
        for (int c = 0; c < curr - new; c++) {
            WriteRegister(REG_TLB_FLUSH, (RCS421REGVAL)(curr - 1 - i));
            head->pcb->pageTable[curr - 1 - i].pfn = findPhysPage();
            head->pcb->pageTable[curr - 1 - i].uprot = PROT_READ | PROT_WRITE;
            head->pcb->pageTable[curr - 1 - i].valid = 1;
        }
        head->pcb->userStackLimit = (void*)DOWN_TO_PAGE(addr);
        return 1;
    } else {
        return -1;
}

unsigned int
findPhysPage(){
    int page_index;
    if (isVMInitialized) {
        page_index = 0;
    } else {
        page_index = MEM_INVALID_PAGES;
    }
    for (int c = page_index; c < p_pages; c++) {
        if (p_page_free[c] == 0) {
            p_page_free[c] = 1;
            return c;
        }
    }
    Halt();
}

unsigned int recentFreePP() {
    if (p_page_free[DOWN_TO_PAGE(VMEM_1_LIMIT - 1) / PAGESIZE] == 1) {
        Halt();
    }
    p_page_free[DOWN_TO_PAGE(VMEM_1_LIMIT - 1) / PAGESIZE] = 1;
    return DOWN_TO_PAGE(VMEM_1_LIMIT - 1) / PAGESIZE;
}

// free the physical page 
void freePP(unsigned int idx){
	if(p_page_free[idx] == 0){
		Halt();
	}
	p_page_free[idx] = 0;
}

int SetKernelBrk(void *addr) {
    if (vm_enabled == 1) {
        if (((long)UP_TO_PAGE(addr) - (long)brk) / PAGESIZE) <= numFreePages() {
            for (int c = 0; c < ((long)UP_TO_PAGE(addr) - (long)brk) / PAGESIZE; c++) {
                if (page_table[c + ((long)brk - VMEM_1_BASE) / PAGESIZE].valid == 1) {
                    Halt();
                }
                page_table[c + ((long)brk - VMEM_1_BASE) / PAGESIZE].valid = 1;
                page_table[c + ((long)brk - VMEM_1_BASE) / PAGESIZE].pfn = findPhysPage();
            }
            if(p_page_free != NULL){
                for (int c = DOWN_TO_PAGE(brk) / PAGESIZE; c < UP_TO_PAGE(addr) / PAGESIZE; c++){
                    p_page_free[i] = 1;
                }
            }
            brk = (void*)UP_TO_PAGE(addr);
        } else {
            //  return -1 when
            // run out of physical memory or otherwise be unable to satisfy the requirements
            return -1 
        }
    } else {
        if ((long)brk - PAGESIZE >= (long)addr) return -1;
        if(p_page_free != NULL){
                for (int c = DOWN_TO_PAGE(brk) / PAGESIZE; c < UP_TO_PAGE(addr) / PAGESIZE; c++){
                    p_page_free[i] = 1;
                }
            }
            brk = (void*)UP_TO_PAGE(addr);
    }
    return 0;
}

void brkHandler(ExceptionInfo *info) {
	struct pcbEntry *process = getRunningNode();
	struct processControlBlock *block = process->block;
	void *k_brk = block->k_brk;
	void *stack = block->stack;

	struct pte *user_page_table = block->pageTable;

    void *addr = (void*)info->regs[1];

    // account for addresses that aren't valid

    if (MEM_INVALID_SIZE >= UP_TO_PAGE(addr) || DOWN_TO_PAGE(stack) - 1 <= UP_TO_PAGE(addr)){
        info->regs[0] = ERROR;
        return; 
    }

    if (UP_TO_PAGE(k_brk) > UP_TO_PAGE(addr)) {
        // here we want to loop through the page addresses and
        // set valid to false 
        // free the current page 
        // and lastly write to the register to flush the TLB
        for (int c = 0; c < (long)UP_TO_PAGE(brk) - (long)UP_TO_PAGE(addr) / PAGESIZE; c++) {
            user_page_table[(long)UP_TO_PAGE(brk) / PAGESIZE - 1 - i].valid = 0; 
            freePhysPage(user_page_table[(long)UP_TO_PAGE(k_brk) / PAGESIZE - 1 - i].pfn);
            WriteRegister(REG_TLB_FLUSH, (RCS421RegVal)((long)UP_TO_PAGE(k_brk) / PAGESIZE - 1 - i));
        }
    } else if (UP_TO_PAGE(k_brk) < UP_TO_PAGE(addr)) {
        int numNeededPages = ((long)UP_TO_PAGE(addr) - (long)UP_TO_PAGE(k_brk))/PAGESIZE;
		// not enough physical memory
		if(freePhysPage() < ((long)UP_TO_PAGE(addr) - (long)UP_TO_PAGE(k_brk)) / PAGESIZE) {
			info->regs[0] = ERROR;
			return;
		} else {
			for(int c = 0; c < ((long)UP_TO_PAGE(addr) - (long)UP_TO_PAGE(k_brk)) / PAGESIZE; c++) {
				user_page_table[c + (long)UP_TO_PAGE(brk) / PAGESIZE].valid = 1;
				user_page_table[c + (long)UP_TO_PAGE(brk) / PAGESIZE].pfn = findPhysPage();
			}
        }
    }

    block->k_brk = (void*)UP_TO_PAGE(addr);
    info->regs[0] = 0;
}

void openPageSpace() {
    createPageSpace = malloc(PAGESIZE);
}

void* getCreatePageSpace() {
    return createPageSpace;
}

// change from virtual address to physical address 
void* vToP(void *addr) {
    // we need to get the virtual page base address 
    void* virtual_address = (void*)DOWN_TO_PAGE(addr);
    int v_pfn;
    if (virtual_address >= (void*)VMEM_1_base) {
        v_pfn = page_table[((long)virtual_address - VMEM_1_base) / PAGESIZE].pfn;
    } else {
        struct pcbEntry *currProcess = getHead();
        v_pfn = currProcess->pcb->pageTable[(long)virtual_address / PAGESIZE].pfn;
    }
    // procure the physical address needed to offset 
    void* physical_address = (void*) (long)(PAGESIZE * v_pfn);

    // add the addr given & PAGEOFFSET to get the offset 
    return (void *) (((long) physical_address) + ((long)addr & PAGEOFFSET) );
}

