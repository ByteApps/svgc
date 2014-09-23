//
//  c_string.h
//  SVGC
//
//  Created by Salvador Guerrero on 9/21/14.
//  Copyright (c) 2014 ByteApps. All rights reserved.
//

#ifndef SVGC_c_string_h
#define SVGC_c_string_h

#include <stdio.h>

enum indexOfOption
{
    IOO_ForwardLookup,
    IOO_ReverseLookup
};

/* indexOfChar
 *
 * Parameters:
 *  str: string where the *car* will be located
 *  car: character to look for
 *
 * Returns -1 if *car* was not found in *str*, otherwise the index of *car* in *str*
 */
int indexOfChar(const char *str, char car);

/* indexOfChar
 *
 * Parameters:
 *  str: string where the *car* will be located
 *  car: character to look for
 *  option: specifies options from *enum indexOfOption*
 * 
 * Returns -1 if *car* was not found in *str*, otherwise the index of *car* in *str*
 */
int indexOfCharWithOption(const char *str, char car, enum indexOfOption option);

/* strByRemovingChar
 *
 * Removes car character from source string and returns the result in output.
 * stops searching at null termination character.
 * Caller is responsible for freeing memory.
 */
void strByRemovingChar(char **output, const char *source, char car);

/* dtos
 *
 * returns the double conained in value in a string contained in output
 */
void dtos(char **output, double value);

#endif
