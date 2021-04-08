#ifndef RAM_H
#define RAM_H

#include<stdio.h>
/*
Ram structure implemented as an array.
Size is 40 strings
*/
char *ram[40];

/*
instantiate all cells to NULL
*/
void resetRAM();

/*
remove a frame from ram
*/
void removeFrame(int frame);

#endif