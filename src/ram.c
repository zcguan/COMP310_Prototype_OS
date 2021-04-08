#include<stdio.h>
#include<string.h>

/*
Ram structure implemented as an array.
Size is 40 strings
*/
char *ram[40]; 

/*
instantiate all cells to NULL
*/
void resetRAM()
{
    for (int i = 0; i < 40; i++)
    {
        ram[i] = NULL;
    }
}

/*
remove a frame from ram
*/
void removeFrame(int frame)
{
    int start = frame * 4;
    int end = start + 4;
    for (int i = start; i <= end; i++)
    {
        ram[i] = NULL;
    }
}
