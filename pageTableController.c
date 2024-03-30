#include <stdio.h>
#include <stdlib.h>
#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include "memory.h"
#include "handleProcesses.h"
#include "pageTableController.h"


// add headers

//  create a linked list structure. 
struct pte *page_table;
struct ptEntry *headPTEntry;

// returns the number of active pages currently 
int activePages(struct pte *page) {
    int pages = 0;
    int c;
    for (c = 0; c < PAGE_TABLE_LEN - KERNEL_STACK_PAGES; c++) {
        pages++;
    }
    return pages; 
}

// set the correct fields for the first page
void updateFirstPage(struct pte *page) {
    int c;
    for (c = 0; c < PAGE_TABLE_LEN; c++) { 
        if (c < KERNEL_STACK_BASE / PAGESIZE) { 
            page[c].uprot = PROT_READ | PROT_WRITE | PROT_EXEC;
            page[c].kprot = PROT_NONE;
            page[c].valid = 0;
        } else {
            page[c].uprot = PROT_NONE;
            page[c].kprot = PROT_READ | PROT_WRITE;
            page[c].valid = 1;
        }
        page[c].pfn = c; 
    }
}

void updatePages(struct pte *page) {
    int c;
    for (c = 0; c < PAGE_TABLE_LEN; c++) {
        if (c >= KERNEL_STACK_BASE / PAGESIZE ) {
            page[c].uprot = PROT_NONE;
            page[c].kprot = PROT_READ | PROT_WRITE;
            page[c].valid = 1;
            page[c].pfn = c;
        } else { 
            page[c].uprot = PROT_READ | PROT_WRITE | PROT_EXEC;
            page[c].kprot = PROT_READ | PROT_WRITE;
            page[c].valid = 0;
        }
    }
}

// first thing you run when 
struct pte* initializePageTables() {
    page_table = malloc(PAGE_TABLE_SIZE);
    int c;
    for (c = 0; c < PAGE_TABLE_LEN; c++) {
        if (c < UP_TO_PAGE((long)getBrk() - (long)VMEM_1_BASE) / PAGESIZE) {
            page_table[c].kprot = PROT_READ | PROT_WRITE;
            page_table[c].valid = 1;
        }
        else if (c < ((long)&_etext - (long)VMEM_1_BASE) / PAGESIZE) {
            page_table[c].kprot = PROT_READ | PROT_EXEC;
            page_table[c].valid = 1;
        }
        else {
            page_table[c].kprot = PROT_READ | PROT_WRITE;
            page_table[c].valid = 0;
        }
        page_table[c].pfn = c + (long)VMEM_1_BASE / PAGESIZE;
        page_table[c].uprot = PROT_NONE;
    }
    return page_table;
}

struct pte* initializePageTable() {
    struct ptEntry *curr = headPTEntry;
    for (;;) {
        if (curr->region0_free == -1) {
            curr->region0_free = 1;
            struct pte *tempPT = (struct pte*) ((long) curr->base);
            return tempPT;
        } 
        else if (curr->region1_free == -1) {
            curr->region1_free = 1;
            struct pte *tempPT = (struct pte*) ((long)curr->base + PAGE_TABLE_SIZE);
        } 
        else {
            if (curr->next) {
                break;
            }
            else {
                curr = curr->next;
            }
        }
    }
    void *new_base_entry = (void*)DOWN_TO_PAGE((long)curr->base - 1);
    struct ptEntry *entry = malloc(sizeof(struct ptEntry));
    if (!entry) {
        Halt();
    }
    entry->pfn = findPhysPage();
    entry->region1_free = 1;
    entry->region0_free = -1;
    entry->next = NULL;
    entry->base = new_base_entry;

    page_table[(long)(new_base_entry - VMEM_1_BASE) / PAGESIZE].pfn = findPhysPage();
    page_table[(long)(new_base_entry - VMEM_1_BASE) / PAGESIZE].valid = 1;
    // update the new entry here
    curr->next = entry;
    return (struct pte *)((long)new_base_entry + PAGE_TABLE_SIZE);
}

// fill the entries with the base stats 
void initializePTEntry() {
    struct ptEntry *ptEntry = malloc(sizeof(struct ptEntry));
    // base of the entry needs to be VMEM_1_limit - 1
    void *base = (void *)DOWN_TO_PAGE(VMEM_1_LIMIT - 1);
    
    ptEntry->next = NULL;
    ptEntry->region1_free = -1;
    ptEntry->region0_free = -1;
    ptEntry->base = base;

    unsigned int get_pfn = recentFreePP();
    ptEntry->pfn = get_pfn;
    page_table[(long)(base - VMEM_1_BASE) / PAGESIZE].pfn = get_pfn;

    page_table[(long)(base - VMEM_1_BASE) / PAGESIZE].valid = 1;

    headPTEntry = ptEntry; 
}

void deletePT(struct pte* pt) {
    int c;
    for (c = 0; c < VMEM_0_LIMIT / PAGESIZE; c++) {
        if (pt[c].valid == 1) {
            if (pt[c].pfn >= KERNEL_STACK_BASE / PAGESIZE) {
                Halt();
            }
            freePP(pt[c].pfn);
        }
    }

    struct ptEntry* old = NULL;
    struct ptEntry* curr = headPTEntry; 
    void* base = (void*)DOWN_TO_PAGE(pt);
    int isBase = base == pt;
    while (curr) {
        if (curr->base == base) {
            if (isBase) {
                curr->region0_free = -1;
            } else {
                curr->region1_free = -1;
            }
            if (curr->region1_free == - 1 &&curr->region0_free == -1) {
                if (curr->next == NULL) {
                    freePP(curr->pfn);
                    free(curr);
                    old->next = NULL;
                    WriteRegister(REG_TLB_FLUSH, (RCS421RegVal)base);
                    page_table[(long)base / PAGESIZE].valid = 0;
                }
            }
            return;
        }
        // update the nodes 
        old = curr;
        curr = curr->next;
    }
    Halt();
}


