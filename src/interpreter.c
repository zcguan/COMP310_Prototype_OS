#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"shellmemory.h"
#include"shell.h"
#include"kernel.h"
#include"memorymanager.h"
#include"ram.h"

#define TRUE 1
#define FALSE 0

/*
This function takes an array of string. 
First string is the "set" command
Second string is a variable name
Third string is a value
It assigns that value to a environment variable with that variable name in the shell memory array.
Return ERRORCODE -1 if out of memory else 0
*/
int set(char * words[]){
    char* varName = words[1];
    char* value = words[2];
    int errorCode = setVariable(varName,value);
    return errorCode;
}

/*
This function takes an array of string. 
First string is the "print command".
Second string is the variable name.
It will print the value associated with that variable if it exists.
Else it will print an appropriate message.
Return 0 if successful.
*/
int print(char * words[]){

    char* varName = words[1];
    char* value = getValue(varName);

    if (strcmp(value,"_NONE_")==0) {
        // If no variable with such name exists, display this message
        printf ("Variable does not exist\n");
    } else {
        // else display the variable's value
        printf("%s\n",value);
    }
    return 0;
}

/*
This function takes an array of string.
First string is the "run" command
Second string is script filename to execute
Returns errorCode
*/
static int run(char * words[]){

    char * filename = words[1];
    FILE * fp = fopen(filename,"r");
    int errorCode = 0;
    // if file cannot be opened, return ERRORCODE -3
    if (fp==NULL) return -3;
    char buffer[1000];
    printf("/////////////////////////////// STARTING EXECUTION OF %s ///////////////////////////////\n",filename);
    while (!feof(fp)){
        fgets(buffer,999,fp);
        errorCode = parse(buffer);
        // User input the "quit" command. Terminate execution of this script file.
        if (errorCode == 1) {
            // Run command successfully executed so ERRORCODE 0. Stop reading file.
            errorCode = 0;
            break;
        } else if (errorCode != 0) {
            // An error occurred. Display it and stop reading the file.
            //removing the extra carriage return
            buffer[strlen(buffer)-2]='\0';
            displayCode(errorCode,buffer);
            break;
        }
    }
    printf("/////////////////////////////// Terminating execution of %s ///////////////////////////////\n",filename);
    fclose(fp);
    return 0;
}

// now should work with same file concurrently
int exec(char * words[]){
    char *filename[3] = {"_NONE_", "_NONE_", "_NONE_"};
    int nextFree = 0;
    int errorCode = 0;
    for (int i = 1; i <= 3; i++)
    {
        if (strcmp(words[i], "_NONE_") != 0)
        {
            filename[nextFree] = strdup(words[i]);
            nextFree++;

            // open file
            FILE *fp = fopen(words[i], "r");
            if (fp == NULL)
                return -3;

            // pass to launcher with filename
            errorCode = launcher(fp, words[i]);
            if (errorCode < 0)
            {
                displayCode(errorCode, words[i]);
                printf("EXEC COMMAND ABORTED...\n");
                emptyReadyQueue();
                resetRAM();
                return 0;
            }
            
        // We've ran through every filenames, so get out of the for loop
        }
        else
        {
            break;
        }
    }
    
    printf("|----------| ");
    printf("\tSTARTING CONCURRENT PROGRAMS ( ");
    for (int i = 0; i < 3; i++)
    {
        if ( strcmp(filename[i],"_NONE_") != 0 ){
            printf("%s ", filename[i]);
        }
    }
    printf(")\t|----------|\n");

    scheduler();

    printf("|----------| ");
    printf("\tTERMINATING ALL CONCURRENT PROGRAMS");
    printf("\t|----------|\n");
    return 0;
}

/*
This functions takes a parsed version of the user input.
It will interpret the valid commands or return a bad error code if the command failed for some reason
Returns:
ERRORCODE  0 : No error and user wishes to continue
ERRORCODE  1 : Users wishes to quit the shell / terminate script
ERRORCODE -1 : RAN OUT OF SHELL MEMORY
ERRORCODE -2 : INCORRECT NUMBER OF ARGUMENTS
ERRORCODE -3 : FILE DOES NOT EXIST
ERRORCODE -4 : UNKNOWN COMMAND. TRY "help"
*/
int interpreter(char* words[]){
    //default errorCode if no error occurred AND user did not enter the "quit" command
    int errorCode = 0;
    //At this point, we are checking for each possible commands entered
    if ( strcmp(words[0],"help") == 0 ) {
        
        // if it's the "help" command, we display the description of every commands
        printf("-------------------------------------------------------------------------------------------------------\n");
        printf("COMMANDS\t\t\tDESCRIPTIONS\n");
        printf("-------------------------------------------------------------------------------------------------------\n");
        printf("help\t\t\t\tDisplays all commands\n");
        printf("quit\t\t\t\tTerminates the shell\n");
        printf("set VAR STRING\t\t\tAssigns the value STRING to the shell memory variable VAR\n");
        printf("print VAR\t\t\tDisplays the STRING value assigned to the shell memory variable VAR\n");
        printf("run SCRIPT.TXT\t\t\tExecutes the file SCRIPT.txt\n");
        printf("exec p1 p2 p3\t\t\tExecutes programs p1 p2 p3 concurrently\n");
        printf("-------------------------------------------------------------------------------------------------------\n");

    } else if ( strcmp(words[0],"quit") == 0) {

        // if it's the "quit" command
        //errorCode is 1 when user voluntarily wants to quit the program.
        exit(1);

    } else if ( strcmp(words[0],"set") == 0 ) {
        // if it's the "set VAR STRING" command
        // check for the presence or 2 more arguments
        // If one argument missing, return ERRORCODE -2 for invalid number of arguments
        if ( ( strcmp(words[1],"_NONE_") == 0 ) || ( strcmp(words[2],"_NONE_") == 0 ) ) {
            errorCode = -2;
        } else {
            // ERRORCODE -1 : Out of Memory might occur
            errorCode = set(words);
        }
    }  else if ( strcmp(words[0],"print") == 0 ) {
        // if it's the "print VAR" command
        // if there's no second argument, return ERRORCODE -2 for invalid number of arguments
        if ( strcmp(words[1],"_NONE_") == 0 ) return -2;

        // Call the print function
        errorCode = print(words);

    } else if ( strcmp(words[0],"run") == 0 ) {
        // if it's the "run SCRIPT.TXT" command
        // check if there's a second argument, return ERRORCODE -2 for invalid number of arguments
        if ( strcmp(words[1],"_NONE_") == 0 ) return -2;

        //Error will be handled in the run function. We can assume that after the run 
        //function terminate, the errorCode is 0.
        errorCode = run(words);
    } else if ( strcmp(words[0],"exec") == 0 ) {
        // if it's the "exec" command
        // check if there's at least 2 arguments and not >= 4 arguments
        if ( strcmp(words[1],"_NONE_") == 0  || strcmp(words[4],"_NONE_") != 0 ) return -2;

        errorCode = exec(words);
    } else {
        // Error code for unknown command
        errorCode = -4;
    }

    return errorCode;
    
}