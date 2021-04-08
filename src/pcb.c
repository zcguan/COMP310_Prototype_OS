#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
PCB has 3 fields
PC : Points to the the current line reached for that program
start: Points to first line of that program
end: Points to last line of that program
*/
typedef struct PCB
{
    int PC;
    int pageTable[10];
    int PC_page, PC_offset, pages_max;
    int pid;
    char *filename;
}PCB;


PCB* makePCB(int pid, char *fname, int totalPages){
    PCB* pcb = (PCB*)malloc(sizeof(PCB));
    pcb->PC = 0;
    pcb->PC_offset = 0;
    pcb->PC_page = 0;
    pcb->pages_max = totalPages;
    pcb->pid = pid;
    pcb->filename = strdup(fname);
    for (size_t i = 0; i < 10; i++)
    {
        pcb->pageTable[i] = -1;
    }
    
    return pcb;
}


