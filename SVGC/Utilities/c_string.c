//
//  c_string.c
//  SVGC
//
//  Created by Salvador Guerrero on 9/21/14.
//  Copyright (c) 2014 ByteApps. All rights reserved.
//

#include "c_string.h"

#include <string.h>
#include <stdlib.h>

/* indexOfChar
 *
 * Parameters:
 *  str: string where the *car* will be located
 *  car: character to look for
 *
 * Returns -1 if *car* was not found in *str*, otherwise the index of *car* in *str*
 */
int indexOfChar(const char *str, char car)
{
    return indexOfCharWithOption(str, car, IOO_ForwardLookup);
}

/* indexOfChar
 *
 * Parameters:
 *  str: string where the *car* will be located
 *  car: character to look for
 *  option: specifies options from *enum indexOfOption*
 *
 * Returns -1 if *car* was not found in *str*, otherwise the index of *car* in *str*
 */
int indexOfCharWithOption(const char *str, char car, enum indexOfOption option)
{
    char *carptr;

    if (!str | !car)
    {
        return -1;
    }

    switch (option)
    {
        case IOO_ForwardLookup:
        {
            //strchr: Locates first occurrence of character in string

            if((carptr = strchr(str, car)))
            {
                return (int)(carptr - str + 1);
            }

            break;
        }
        case IOO_ReverseLookup:
        {
            //strrchr: Locates last occurrence of character in string

            if((carptr = strrchr(str, car)))
            {
                return (int)(carptr - str + 1);
            }

            break;
        }
    }

    return -1;
}

/* strWithoutChar
 *
 * Removes car character from source string and returns the result in output.
 * stops searching at null termination character.
 * Caller is responsible for freeing memory.
 */
void strByRemovingChar(char **output, const char *source, char car)
{
    char currentchar;
    int count = 0;
    int sourcelen = (int)strlen(source);

    if (!source || !car)
    {
        return;
    }

    *output = calloc(1, sourcelen + 1);

    while ((currentchar = *source++))
    {
        if (currentchar == car)
        {
            continue;
        }

        *((*output) + count++) = currentchar;
    }

    *((*output) + count) = '\0';
}

/* dtos
 *
 * returns the double conained in value in a string contained in output
 */
void dtos(char **output, double value)
{
    *output = calloc(1, 50);
    sprintf(*output, "%.2f", value);
}
