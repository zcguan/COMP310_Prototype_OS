#ifndef DISK_DIRVE_H
#define DISK_DIRVE_H

void initIO();
int partition(char *name, int blocksize, int totalblocks);
int mountFS(char *name);
int openfile(char *name);
char *readBlock(int file);
int writeBlock(int file, char *data);
void closefile(int file);

#endif