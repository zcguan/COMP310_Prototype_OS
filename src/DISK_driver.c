#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// size constants
#define MAX_PARTITIONNAME 50
#define MAX_OPEN_FILE 5
#define MAX_FILENAME 50
#define MAX_BLOCKNUM 10
#define MAX_FAT 20

struct PARTITION
{
    char name[MAX_PARTITIONNAME];
    int total_blocks;
    int block_size;
    int free_block;
} aPartition;

struct FAT
{
    char filename[MAX_FILENAME];
    int file_length;
    int blockPtrs[MAX_BLOCKNUM];
    int current_location;
} fat[MAX_FAT];

char *block_buffer;
FILE *active_file_table[MAX_OPEN_FILE];
int fat_to_aft[MAX_FAT]; // index i is index in fat[], fat_to_aft[i] gives the index in aft

void printMetadata(){
    printf("partition info:\nname: %s, total blocks: %d, block size: %d, free block: %d.\n", aPartition.name, aPartition.total_blocks, aPartition.block_size, aPartition.free_block);
    printf("\n");

    for (int i = 0; i < MAX_FAT; i++){
        printf("fat[%d] info:\nfilename: %s, file len: %d, curr loc: %d.\n", i, fat[i].filename, fat[i].file_length, fat[i].current_location);
        for (int j = 0; j < MAX_BLOCKNUM; j++)
        {
            printf("blockPtrs[%d]: %d, ", j, fat[i].blockPtrs[j]);
        }
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < MAX_OPEN_FILE; i++){
        printf("aft[%d]: %p, ", i, active_file_table[i]);
    }    
    printf("\n");

    for (int i = 0; i < MAX_FAT; i++){
        printf("map[%d]: %d, ", i, fat_to_aft[i]);
    }  
}

void initIO(){
    // partition
    aPartition.block_size = 0, aPartition.total_blocks = 0, aPartition.free_block = 0;
    for (int i = 0; i < MAX_PARTITIONNAME; i++){
        aPartition.name[i] = 0;
    }

    // fat
    for (int i = 0; i < MAX_FAT; i++)
    {
        for (int j = 0; j < MAX_FILENAME; j++){
            fat[i].filename[j] = 0;
        }
        fat[i].file_length = 0;
        for (int j = 0; j < MAX_BLOCKNUM; j++){
            fat[i].blockPtrs[j] = -1;
        }
        fat[i].current_location = 0;

        // map
        fat_to_aft[i] = -1;
    }
    
    // active file table
    for (int i = 0; i < MAX_OPEN_FILE; i++)
    {
        active_file_table[i] = NULL;
    }
    
}

/*
Create a partition folder and a partition file with name
return 0 for success, 1 for error
*/
int partition(char *name, int blocksize, int totalblocks){
    initIO();
    system("mkdir -p PARTITION");

    // set the structs
    aPartition.block_size = blocksize;
    aPartition.total_blocks = totalblocks;
    strcpy(aPartition.name, name);

    char fname[100];
    sprintf(fname, "PARTITION/%s", name);
    FILE *f = fopen(fname, "w");

    if (f == NULL) // fail to open
    {
        return 1;
    }
    
    // write partition
    int err = fwrite(&aPartition, sizeof(aPartition), 1, f);
    if (err <= 0) // fail to write
    {
        return 1;
    }

    // write fat[20]
    err = fwrite(&fat, sizeof(fat), 1, f);
    if (err <= 0) // fail to write
    {
        return 1;
    }

    // copy totalblocks * blocksize number of 0
    for (int i = 0; i < totalblocks * blocksize; i++)
    {
        fputc('0',f);        
    }
    
    fclose(f);
    block_buffer = (char *)malloc(aPartition.block_size);

    return 0;
}

/*
Mount the specified partition
return 0 for success, 1 for error
*/
int mountFS(char *name)
{
    initIO();

    char fname[100];
    sprintf(fname, "PARTITION/%s", name);
    FILE *f = fopen(fname, "r");
    if (f == NULL){
        return 1;
    }
    
    int err = fread(&aPartition, sizeof(aPartition), 1, f);
    if (err != 1){
        return 1;
    }

    err = fread(&fat, sizeof(fat), 1, f);
    if (err != 1)
    {
        return 1;
    }

    block_buffer = (char *)malloc(aPartition.block_size);
    if (block_buffer == NULL)
    {
        return 1;
    }
    printf("PARTITION %s MOUNTED\n", aPartition.name);
    fclose(f);
    return 0;
}

// return fat index, if error -1 is returned
int openfile(char *name){
    int index = -1;
    // search in fat
    for (int i = 0; i < MAX_FAT; i++){ 
        // find fat index: file not in fat -> first free or in fat -> index
        if (fat[i].filename[0] == 0 || strcmp(name, fat[i].filename) == 0){ 
            index = i;
            break;
        }
    }

    // not in fat and fat doesn't have free entry
    if (index == -1) 
    {
        return -1;
    }
    // not in fat but fat is not full
    // create a new entry in the FAT table
    // return now??
    if (fat[index].filename[0] == 0)
    {
        strcpy(fat[index].filename, name);
    }

    // check if aft has that fat
    if (fat_to_aft[index] != -1){
        return index;
    }

    // search free aft entry
    for (int i = 0; i < MAX_OPEN_FILE; i++) 
    {
        if (active_file_table[i] == NULL){ // open and reset ptrs
            char fname[100];
            sprintf(fname, "PARTITION/%s", aPartition.name);
            active_file_table[i] = fopen(fname,"r+");
            if (active_file_table[i] == NULL){
                return -1;
            }
            fseek(active_file_table[i], sizeof(aPartition)+sizeof(fat), SEEK_SET);
            fat_to_aft[index] = i;
            fat[index].current_location = 0;
            return index;
        }
    }
    return -1; //aft full
}

// close a file fat[file] in the aft
// needed to reset the block pointer
void closefile(int file){
    FILE *f = active_file_table[fat_to_aft[file]];
    if (f == NULL){ // not in aft
        return;
    }
    fclose(f);
    active_file_table[fat_to_aft[file]] = NULL;
    fat_to_aft[file] = -1;
}

/*
read one block from the file at fat[file]
*/
char *readBlock(int file){
    if(file < 0 || fat[file].current_location >= MAX_BLOCKNUM) // error or max block reached
    {
        return NULL; 
    }

    // current block in partition is the value of blockPtrs[current_location]
    int currBlock = fat[file].blockPtrs[fat[file].current_location];

    if(currBlock == -1){ // last block reached
        return NULL; 
    }

    // get file pointer from aft
    FILE *f = NULL;
    int index = fat_to_aft[file];
    if(index == -1){ // file not in aft
        return NULL; 
    }
    f = active_file_table[index];

    // seek to the block
    fseek(f, sizeof(aPartition) + sizeof(fat) + currBlock*aPartition.block_size*sizeof(char), SEEK_SET);

    // read the block
    for (int i = 0; i < aPartition.block_size; i++)
    {
        char c = fgetc(f);
        if(c == '0'){
            block_buffer[i] = '\0'; // EOF
            break;
        }
        block_buffer[i] = c;
    }
    block_buffer[aPartition.block_size] = '\0'; // NULL terminate at max len anyway
    
    fat[file].current_location += 1;
    
    return block_buffer;
}

/*
write data to file at fat[file]
return 0 for success, 1 for error
*/
int writeBlock(int file, char *data){
    if(file < 0 || fat[file].current_location >= MAX_BLOCKNUM) // error or max block reached
    {
        return 1; 
    }

    // append to the end
    fat[file].current_location = fat[file].file_length;

    // get file pointer from aft
    FILE *f = NULL;
    int index = fat_to_aft[file];
    if (index == -1)
    { // file not in aft
        return 1;
    }
    f = active_file_table[index];

    // seek to the block
    fseek(f, sizeof(aPartition) + sizeof(fat) + aPartition.free_block * aPartition.block_size * sizeof(char), SEEK_SET);
    // write data
    int blocksToWrite = (strlen(data)/aPartition.block_size) + 1;
    for (int i = 0; i < blocksToWrite; i++)
    {
        if (fat[file].current_location >= MAX_BLOCKNUM){
            printf("MAX FILE LENGTH REACHED\n");
            return 1;
        }
        if (aPartition.free_block >= aPartition.total_blocks){
            printf("PARTITION FULL\n");
            return 1;
        }

        for (int j = 0; j < aPartition.block_size; j++){
            int index = i *aPartition.block_size + j;
            char c = data[index];
            if (index >= strlen(data)){
                c = '0';
            }
            fputc(c,f);
        }
        
        // update fat
        fat[file].blockPtrs[fat[file].current_location] = aPartition.free_block++;
        fat[file].current_location += 1;
        fat[file].file_length += 1;
    }

    // update fat and partition on disk
    fseek(f, 0, SEEK_SET);
    int err = fwrite(&aPartition, sizeof(aPartition), 1, f);
    if (err <= 0) // fail to write
    {
        return 1;
    }

    err = fwrite(&fat, sizeof(fat), 1, f);
    if (err <= 0) // fail to write
    {
        return 1;
    }

    return 0;
}