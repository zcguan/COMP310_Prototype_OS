#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ram.h"
#include "kernel.h"

int pcount = 0;

int countTotalPages(FILE *f)
{
    rewind(f);
    int c = 0;
    char buffer[1000];
    while(!feof(f))
    {
        fgets(buffer, 999, f);
        c++;
    }
    rewind(f);
    int num = c/4;
    if (c % 4 != 0) num++;
    return num;
}

void loadPage(int pageNumber, FILE *f, int frameNumber){
    rewind(f);
    char buffer[1000];
    int i = 0;

    // move to pageNumber * 4 line (a page is 4 lines)
    while (!feof(f) && i < pageNumber*4){
        fgets(buffer,999,f);
        i++;
    }

    // load 4 lines to ram at frameNumber*4 + i
    i = 0;
    while (!feof(f) && i < 4)
    {
        fgets(buffer, 999, f);
        ram[frameNumber * 4 + i] = strdup(buffer);
        i++;
    }
    rewind(f);
}

// find first free frame index, otherwise return -1
int findFrame(){
    int i;
    for (i = 0; i < 10; i++)
    {
        if (ram[i*4] == NULL)
            return i;
    }
    return -1;
}

// check if n is in arr
int inPageTable(int n, int pageTable[]){
    for (int i = 0; i < 40; i++)
    {
        if (pageTable[i] == n)
            return 1;
    }
    return 0;
}

int findVictim(PCB *p){
    int frameNumber = rand() % 10;

    while(1){
        while (inPageTable(frameNumber, p->pageTable))
            frameNumber = (frameNumber + 1) % 10;

        PCB *victimPCB = findPCBFromFrame(frameNumber);        

        // victim dont use the frame
        if (victimPCB->pageTable[victimPCB->PC_page] != frameNumber)
        {
            return frameNumber;
        }
        else
        {
            frameNumber = (frameNumber + 1) % 10;
        }
    }
    return frameNumber;
}


int updatePageTable(PCB *p, int pageNumber, int frameNumber, int victimFrame){
    p->pageTable[pageNumber] = frameNumber;
    
    if (victimFrame){
        // need to update victime pcb page table
        PCB *victim = findPCBFromFrame(frameNumber);
        for (int i = 0; i < 40; i++)
        {
            if (victim->pageTable[i] == frameNumber) // set it to -1
                victim->pageTable[i] == -1;
        }
    }
    return 0;
}

// TODO create pcb, find victime, update page table
int launcher(FILE *p, char *fname)
{
    // find a valid name
    int i = 1;
    char c;
    char bkstrFile[100];
    sprintf(bkstrFile, "BackingStore/%s_%d.backingStore", fname, pcount);
    FILE *fbkstr = fopen(bkstrFile, "w");

    if (fbkstr == NULL)
        return -3;

    // copy the content
    c = fgetc(p);
    while (c != EOF)
    {
        fputc(c, fbkstr);
        c = fgetc(p);
    }
    // close files
    fclose(p);
    fclose(fbkstr);

    // open to read
    FILE *f = fopen(bkstrFile, "r");

    if (f == NULL) return -3;

    // make PCB and
    // load two pages of the program into RAM
    int totalPages = countTotalPages(f);
    PCB *pcb = makePCB(pcount, bkstrFile, totalPages);

    if (totalPages > 2)
        totalPages = 2;
    for (int i = 0; i < totalPages; i++)
    {
        int victim = 0;
        int frame = findFrame();
        if (frame == -1){
            frame = findVictim(pcb);
            victim = 1;
        }
        removeFrame(frame);
        loadPage(i, f, frame);
        updatePageTable(pcb, i, frame, victim);

        if (i==0) // update PC for first frame
            pcb->PC = frame * 4;
    }
    pcount++;
    addToReady(pcb);
    fclose(f);
    return 0;
}