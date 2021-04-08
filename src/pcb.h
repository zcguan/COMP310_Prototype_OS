#ifndef PCB_H
#define PCB_H

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


PCB* makePCB(int, char*,int);

#endif