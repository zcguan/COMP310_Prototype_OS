#ifndef CPU_H
#define CPU_H

#include "pcb.h"
#define DEFAULT_QUANTA 2

struct CPU { 
    int IP; 
    int offset;
    char IR[1000]; 
    int quanta; 
} CPU;

int run(int);

#endif