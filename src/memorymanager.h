#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <stdio.h>

int launcher(FILE *p, char *fname);
void loadPage(int pageNumber, FILE *f, int frameNumber);
int findFrame();
int findVictim(PCB *p);
int updatePageTable(PCB *p, int pageNumber, int frameNumber, int victimFrame);
#endif