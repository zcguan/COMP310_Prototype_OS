#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include"shell.h"
#include"pcb.h"
#include"ram.h"
#include"cpu.h"
#include"interpreter.h"
#include"memorymanager.h"

/*
This is a node in the Ready Queue implemented as 
a linked list.
A node holds a PCB pointer and a pointer to the next node.
PCB: PCB
next: next node
*/
typedef struct ReadyQueueNode {
    PCB*  PCB;
    struct ReadyQueueNode* next;
} ReadyQueueNode;

ReadyQueueNode* head = NULL;
ReadyQueueNode* tail = NULL;
int sizeOfQueue = 0;

int kernel()
{
    return shellUI();
}
/*
Adds a pcb to the tail of the linked list
*/
void addToReady(struct PCB* pcb) {
    ReadyQueueNode* newNode = (ReadyQueueNode *)malloc(sizeof(ReadyQueueNode));
    newNode->PCB = pcb;
    newNode->next = NULL;
    if (head == NULL){
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    sizeOfQueue++;
}

/*
Returns the size of the queue
*/
int size(){
    return sizeOfQueue;
}

/*
Pops the pcb at the head of the linked list.
pop will cause an error if linkedlist is empty.
Always check size of queue using size()
*/
struct PCB* pop(){
    PCB* topNode = head->PCB;
    ReadyQueueNode * temp = head;
    if (head == tail){
        head = NULL;
        tail = NULL;
    } else {
        head = head->next;
    }
    free(temp);
    sizeOfQueue--;
    return topNode;
}

/*
Passes a filename
Opens the file, copies the content in the RAM.
Creates a PCB for that program.
Adds the PCB on the ready queue.
Return an errorCode:
ERRORCODE 0 : NO ERROR
ERRORCODE -3 : SCRIPT NOT FOUND
ERRORCODE -5 : NOT ENOUGH RAM (EXEC)
*/
// int myinit(char* filename){
//     // Open the filename to get FILE *
//     // call addToRam on that File *
//     // If error (check via start/end variable), return that error
//     // Else create pcb using MakePCB
//     // Then add it to the ReadyQueue
//     FILE * fp = fopen(filename,"r");
//     if (fp == NULL) return -3;
//     int start;
//     int end;
//     addToRAM(fp,&start,&end);
//     fclose(fp);
//     if (start == -1) return -5;
//     PCB* pcb = makePCB(start,end);
//     addToReady(pcb);
//     return 0;
// }

// -6 for termination
int pagefault(PCB *pcb)
{
    pcb->PC_page++;
    
    if (pcb->PC_page >= pcb->pages_max)
    {
        return -6;
    }
    if (pcb->pageTable[pcb->PC_page] == -1) // no frame
    {
        FILE *f = fopen(pcb->filename, "r");
        int victim = 0;
        int frame = findFrame();
        if (frame == -1)
        {
            frame = findVictim(pcb);
            victim = 1;
        }
        loadPage(pcb->PC_page, f, frame);
        updatePageTable(pcb, pcb->PC_page, frame, victim);
        fclose(f);
    }
    // valid
    pcb->PC = pcb->pageTable[pcb->PC_page] * 4;
    pcb->PC_offset = 0;
    addToReady(pcb);
    return 0;
}


// Page Fault ERRORCODE = -5
// Termination ERRORCODE = -6
int scheduler(){
    // set CPU quanta to default, IP to -1, IR = NULL
    CPU.quanta = DEFAULT_QUANTA;
    CPU.IP = -1;
    while (size() != 0){
        //pop head of queue
        PCB* pcb = pop();
        //copy PC of PCB to IP of CPU
        CPU.IP = pcb->PC;
        CPU.offset = pcb->PC_offset;

        int quanta = DEFAULT_QUANTA;

        int errorCode = run(quanta);

        if (errorCode == -5)
        {
            pagefault(pcb);
        }
        else if (errorCode!=0){
            for (int i = 0; i < pcb->pages_max; i++)
            {
                ram[pcb->pageTable[i]] = NULL;
            }

            free(pcb);
        } else {
            pcb->PC_offset += DEFAULT_QUANTA;
            addToReady(pcb);
        }
    }
    return 0;
}

/*
Flushes every pcb off the ready queue in the case of a load error
*/
void emptyReadyQueue(){
    while (head!=NULL){
        ReadyQueueNode * temp = head;
        head = head->next;
        free(temp->PCB);
        free(temp);
    }
    sizeOfQueue =0;
}

void boot(){
    resetRAM(); // clear ram
    system("rm -rf BackingStore; mkdir BackingStore");// prepare Backing Store
}

int main() {
    int error=0;
    boot(); // First : actions performed by boot
    error = kernel(); // Second: actions performed by kernel
    return error;
}

PCB* findPCBFromFrame(int frame){
    ReadyQueueNode *q = head;
    while (q != NULL)
    {
        for (int i = 0; i < 40; i++)
        {
            if (q->PCB->pageTable[i] == frame)
                return q->PCB;
        }
        q = q->next;
    }
}
