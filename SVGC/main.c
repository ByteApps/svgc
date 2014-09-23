//
//  main.c
//  SVGC
//
//  Created by Salvador Guerrero on 9/23/14.
//  Copyright (c) 2014 ByteApps. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "svgc/svgc.h"
#include "Utilities/c_string.h"

#define SVGC_EXTENSION  ".svgc"

void newFilename(const char *currentFilename, char **newFilename);

int main(int argc, const char * argv[])
{
    const char *filename;
    char *outFilename;

    FILE *inFile = NULL;
    FILE *outFile = NULL;

    //variables for reading the file

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

#ifdef DEBUG
    int lineNumber = 0;
#endif

    if (argc < 2)
    {
        puts("Usage: SVGC <path to your svg file>\n\tOutput filename: file.svg -> file.svgc.svg\n\n"
             "Searches and executes any operation inside <!--- ---> blocks\n"
             "Examples:\n"
             "\t<!--- 5 * 5 ---> is replaced by 25\n"
             "\t<!--- var val1 = 10 + 15; ---> stores the value 25 in the heap and can be accessed using val1\n"
             "\t<!--- val1 ---> is replaced by 25\n"
             "\t<!--- var val2 = 10; val1 = val2 + 10; ---> creates val2 and val1 value is now 45.\n");
        return 1;
    }

    filename = argv[1];

    if(access(filename, R_OK ) != 0)
    {
        printf("SVGC: The file (%s) doesn't exists or you don't have read permissions.\n", filename);
        return 1;
    }

    //get the extension so we can insert our extension, example: file.svg outputs file.svgc.svg

    newFilename(filename, &outFilename);

    if ((outFile = fopen(outFilename, "w")))
    {
        if ((inFile = fopen(filename, "r")))
        {
            /*
             * searches and executes any operation inside <!--- ---> blocks
             * examples:
             * <!--- 5 * 5 ---> is replaced by 25
             * <!--- var val1 = 10 + 15; ---> stores the value 25 in the heap and can be accessed using val1
             * <!--- val1 ---> is replaced by 25
             * <!--- var val2 = 10; val1 = val2 + 10; ---> creates val2 and val1 value is now 45.
             */

            svgcinit();

            while ((read = getline(&line, &len, inFile)) != -1)
            {
#ifdef DEBUG
                printf("Line: %d\t", ++lineNumber);
#endif

                char *output = NULL;
                svgcstring(&output, line);

                if (output)
                {
                    //something came back, print it to the file

                    fputs(output, outFile);
                }

                free(output);
            }

            svgcfree();
            
            fclose(inFile);
        }
        
        fclose(outFile);
    }
    
    free(outFilename);
    
    return 0;
}

void newFilename(const char *currentFilename, char **newFilename)
{
    if (!currentFilename)
    {
        return;
    }

    *newFilename = malloc(strlen(currentFilename) + strlen(SVGC_EXTENSION));

    int indexOfExt = indexOfCharWithOption(currentFilename, '.', IOO_ReverseLookup);

    if (indexOfExt == -1)
    {
        /* no extension, append to last of filename. */

        //first copy everything

        strcpy(*newFilename, currentFilename);

        //now append svgc extension

        strcat(*newFilename, SVGC_EXTENSION);
    }
    else
    {
        /* has extension, insert before extension. */

        //firts copy everyting before the original extension.

        strncpy(*newFilename, currentFilename, indexOfExt-1);

        //append null termination so strcat can work worrectly.

        (*newFilename)[indexOfExt] = '\0';

        //append svgc extension

        strcat(*newFilename, SVGC_EXTENSION);

        //append the original extension
        
        strcat(*newFilename, strrchr(currentFilename, '.'));
    }
}