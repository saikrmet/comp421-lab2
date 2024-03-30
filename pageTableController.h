#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#include <stdlib.h>

struct ptEntry{
    void *base;
    int region1_free;
    int region0_free;
    struct ptEntry *next;
    int pfn;
};
extern struct pte *page_table;
int activePages(struct pte *page);
void updateFirstPage(struct pte *page);
void updatePages(struct pte *page);
struct pte* initializePageTables();
struct pte* initializePageTable();
void initializePTEntry();
void deletePT(struct pte* pt);
