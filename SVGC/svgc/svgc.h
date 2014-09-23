//
//  svgc.h
//  SVGC
//
//  Created by Salvador Guerrero on 9/21/14.
//  Copyright (c) 2014 ByteApps. All rights reserved.
//

#ifndef SVGC_svgc_h
#define SVGC_svgc_h

void svgcinit(); //needs to be called before any operation.
void svgcfree(); //needs to be called when all svgc operations are done.

/* rsvgcstring
 * 
 * compiles any opration inside <!--- --->
 * returns the result in the output argument
 * the caller is responsible of freeing output
 */
void svgcstring(char **output, const char *source);

#endif
