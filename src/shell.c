#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interpreter.h"

/*
This functions passes an int errorCode and command string and display the 
appropriate error message for that errorCode
ERRORCODE -1 : RAN OUT OF SHELL MEMORY
ERRORCODE -2 : INCORRECT NUMBER OF ARGUMENTS
ERRORCODE -3 : SCRIPT NOT FOUND
ERRORCODE -4 : UNKNOWN COMMAND.
ERRORCODE -5 : NOT ENOUGH RAM (EXEC)
ERRORCODE -6 : FS ERROR
*/
void displayCode(int errorCode, char *command)
{
    switch (errorCode)
    {
    case -1:
        printf("ERRORCODE -1 : RAN OUT OF SHELL MEMORY\n");
        break;
    case -2:
        printf("ERRORCODE -2 : '%s' INCORRECT NUMBER OF ARGUMENTS. TRY 'help'\n", command);
        break;
    case -3:
        printf("ERRORCODE -3 : SCRIPT NOT FOUND\n");
        break;
    case -4:
        printf("ERRORCODE -4 : '%s' IS AN UNKNOWN COMMAND. TRY 'help'\n", command);
        break;
    case -5:
        printf("ERRORCODE -5 : NOT ENOUGH RAM TO ADD PROGRAM.'%s'\n", command);
        break;
    case -6:
        printf("ERRORCODE -6 : %s FS ERROR \n", command);
        break;
    }

    return;
}

int parse(char ui[])
{
    // printf("In Parser: '%s'",ui);
    char tmp[200];
    int a, b;
    char *words[10];
    int w = 0; // wordsIdx
    int string = 0;
    char openDelim[] = {'\"', '['};
    char closingDelim[] = {'\"', ']'};
    int delimi = 0;

    // init token array
    for (int i = 0; i < 10; i++)
    {
        words[i] = "_NONE_";
    }

    // skip leading white spaces
    for (a = 0; ui[a] == ' ' && a < 1000; a++); 
        

    while (ui[a] != '\0' && ui[a] != '\n' && a < 1000)
    {
        if (string)
        {
            for (b = 0; ui[a] != '\0' && ui[a] != closingDelim[delimi] && a < 1000; a++, b++)
                tmp[b] = ui[a]; // read until closing "
            tmp[b] = '\0';
            words[w] = strdup(tmp);
            string = 0; // end of string
            a++;        // skip delimiter
        }
        else
        {
            for (b = 0; ui[a] != '\0' && ui[a] != '\n' && ui[a] != '\r' && ui[a] != ' ' && a < 1000; a++, b++)
                tmp[b] = ui[a]; // extract a word
            tmp[b] = '\0';
            words[w] = strdup(tmp);
        }

        if (ui[a] == '\0' || ui[a] == '\n' || ui[a] == '\r')
            break;

        for (; ui[a] == ' ' && a < 1000; a++)
            ; // skip white spaces

        // check if beginning of string
        for (int i = 0; i < sizeof(openDelim); i++)
        {
            if (ui[a] == openDelim[i])
            {
                string = 1; // start of a string
                a++;        // skip delimiter
                delimi = i; // set delimiter index
            }
        }
        w++;
    }
    return (interpreter(words));
    //free(words);
}

/*
This is the main loop where 
users gets to use the shell.
*/
int shellUI()
{
    printf(
        "\n----------------------------------\nKernel 1.0 loaded!\nWelcome to the ZiChuan GUAN Shell!\nShell Version 2.0 Updated February 2020\n----------------------------------\n");
    char prompt[15] = {'$', '\0'};
    char userinput[1000];
    int errorCode;

    while (1)
    {
        // prints prompt
        printf("%s", prompt);
        //gets user input
        fgets(userinput, 999, stdin);
        // parses and interprets the command
        errorCode = parse(userinput);
        //If the user entered the "quit" command
        if (errorCode == 1)
        {
            printf("----------------------------------\nExiting ZiChuan GUAN Shell. Farewell...\n----------------------------------\n");
            break;
            //else if an error occurred, display what that error is
        }
        else if (errorCode != 0)
        {
            //removing the extra carriage return
            userinput[strlen(userinput) - 1] = '\0';
            displayCode(errorCode, userinput);
        }
    }
    return 0;
}
