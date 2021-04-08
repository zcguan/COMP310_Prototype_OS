#include<string.h>
#include"ram.h"
#include"interpreter.h"
#include"shell.h"
#include"pcb.h"

#define DEFAULT_QUANTA 2

struct CPU { 
    int IP; 
    int offset;
    char IR[1000]; 
    int quanta; 
} CPU;


/*
This method passes a quanta.
It will then executes quanta number of lines in RAM as from the address
pointed by the CPU IP.
Returns an errorCode.
*/
int run(int quanta){
    for (int i = 0; i < quanta; i++)
    {
        char *instruction = ram[CPU.IP + CPU.offset];

        if (instruction == NULL) // terminated
            return -6;
        
        strcpy(CPU.IR, instruction);
        int errorCode = parse(CPU.IR);
        
        // Do error checking and if error, return error
        if (errorCode != 0){
            // Display error message if fatal
            if (errorCode < 0){
                ram[CPU.IP][strlen(ram[CPU.IP]) - 2] = '\0';
                displayCode(errorCode, ram[CPU.IP]);
            }
            return errorCode;
        }
        CPU.offset ++;

        // page fault interrupt -5
        if (CPU.offset == 4)
        {
            return -5;
        }
    }
    return 0;
}
