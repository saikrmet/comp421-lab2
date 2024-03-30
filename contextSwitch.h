#include <comp421/yalnix.h>
#include <comp421/hardware.h>
#include "pcb.h"
#pragma once

void changePcb(struct pcbStruct* activePCB, struct pcbStruct* newPCB);
void forkPcb(struct pcbStruct* activePCB, struct pcbStruct* newPCB);