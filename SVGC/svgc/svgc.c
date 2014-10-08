//
//  svgc.c
//  SVGC
//
//  Created by Salvador Guerrero on 9/21/14.
//  Copyright (c) 2014 ByteApps. All rights reserved.
//

#include "svgc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "../utilities/c_string.h"

#define LEFT_ID         "<!---"
#define RIGHT_ID        "--->"

char token_delimeters[] =
{
    '+',
    '-',
    '*',
    '/',
    '(',
    ')',
    '='
};

int isTokenDelim(char car)
{
    int i;

    for (i = 0; i < sizeof(token_delimeters)/sizeof(token_delimeters[1]); i++)
    {
        if (car == token_delimeters[i])
        {
            return 1;
        }
    }

    return 0;
}

/*************
 * Variables *
 *************/

struct VariableNode
{
    char *key;
    double value;
    struct VariableNode *next;
};

struct VariableNode *variables = NULL;

struct VariableNode* findVariableWithName(char *name);
void freeVariable(struct VariableNode *variable);

void addVariable(struct VariableNode **newVariable)
{
    struct VariableNode *lastVariable = NULL;

    if (!variables)
    {
        variables = *newVariable;

        return;
    }

    //if the variable exists, just replace the value and free the newVariable

    if ((lastVariable = findVariableWithName((*newVariable)->key)))
    {
        lastVariable->value = (*newVariable)->value;
        freeVariable(*newVariable);
        *newVariable = lastVariable;

        return;
    }

    //find the last token that doesn't have a next token and append it.

    lastVariable = variables;

    while ((lastVariable->next))
    {
        lastVariable = lastVariable->next;
    }

    lastVariable->next = *newVariable;
}

struct VariableNode * addVariableWithName(char *name, double value)
{
    if (!name)
    {
        return NULL;
    }

    struct VariableNode *variable = calloc(1, sizeof(struct VariableNode));
    variable->key = strdup(name);
    variable->value = value;
    addVariable(&variable);

    return variable;
}

struct VariableNode* findVariableWithName(char *name)
{
    struct VariableNode* variable;

    if (!name)
    {
        return NULL;
    }

    for (variable = variables; variable; variable = variable->next)
    {
        if (strcmp(name, variable->key) == 0)
        {
            return variable;
        }
    }

    return NULL;
}

void freeVariable(struct VariableNode *variable)
{
    if (variable->key)
    {
        free(variable->key);
    }

    free(variable);
}

void freeVariablesArray()
{
    if (variables)
    {
        struct VariableNode *variable = variables;

        for (;variable;)
        {
            struct VariableNode *nextVariable = variable->next;
            freeVariable(variable);
            variable = nextVariable;
        }
    }
}

/**********
 * Tokens *
 **********/

enum TokenId
{
    TID_Operation,
    TID_Value
};

struct Token
{
    char *value;
    char operationId;

    enum TokenId tokenId;
    struct Token *next;
    struct Token *previous;
};

void addToken(struct Token **tokens, struct Token *newToken)
{
    struct Token *lastToken = NULL;

    if (!*tokens)
    {
        *tokens = newToken;

        return;
    }

    //find the last token that doesn't have a next token

    lastToken = *tokens;

    while ((lastToken->next))
    {
        lastToken = lastToken->next;
    }

    lastToken->next = newToken;
    newToken->previous = lastToken;
}

struct Token* findLastToken(struct Token *tokens)
{
    struct Token *lastToken = tokens;

    while (lastToken->next)
    {
        lastToken = lastToken->next;
    }

    return lastToken;
}

void freeToken(struct Token *token)
{
    if (!token)
    {
        return;
    }

    if (token->value)
    {
        free(token->value);
    }

    free(token);
}

void freeTokenArray(struct Token *tokens)
{
    if (tokens)
    {
        struct Token *token = tokens;

        for (;token;)
        {
            struct Token *nextToken = token->next;
            freeToken(token);
            token = nextToken;
        }
    }
}

size_t sizeofTokenArray(struct Token *tokens)
{
    size_t size = 0;

    if (!tokens)
    {
        goto result;
    }


    struct Token *token = tokens;
    size++;

    while ((token = token->next))
    {
        size++;
    }

result:
    return size;
}

/***********************
 * svgc Implementation *
 ***********************/


// svgc global variables

regex_t regex = {0};

/* svgcinit
 *
 * Needs to be called before any operation
 * initializes all variables needes for svgc to work
 */

void svgcinit()
{
    //Compile regular expression
    //Reference: https://www.gnu.org/software/libc/manual/html_node/Regular-Expressions.html

    int reti;

    reti = regcomp(&regex, LEFT_ID "[^<>]*" RIGHT_ID , REG_EXTENDED);

#ifdef DEBUG

    if (reti)
    {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
#endif
}

/* svgcfree
 *
 * needs to be called when all svgc operations are done
 * frees all variables needes for svgc to work
 */

void svgcfree()
{
    //Free compiled regular expression

    regfree(&regex);

    freeVariablesArray();
}

int isNumber(char car)
{
    return (car >= '0' && car <= '9') || car == '-';
}

double valueInToken(struct Token *token)
{
    if (!isNumber(token->value[0]))
    {
        //this must be a variable

        struct VariableNode *variable = findVariableWithName(token->value);

        if (!variable)
        {
            fprintf(stderr, "trying to use a variable named '%s' that hasn't been declared\n", token->value);

            exit(1);
        }

        return variable->value;
    }
    else
    {
        //not a variable, must be a constant

        return atof(token->value);
    }
}

struct Token* exeTokenOperationsWithOperators(struct Token **firstTokens, struct Token **lastTokens, char *operators);

struct Token* exeTokenOperations(struct Token **firstTokens, struct Token **lastTokens)
{
    exeTokenOperationsWithOperators(firstTokens, lastTokens, "*/");
    return exeTokenOperationsWithOperators(firstTokens, lastTokens, "+-");
}

struct Token* exeTokenOperationsWithOperators(struct Token **firstTokens, struct Token **lastTokens, char *operators)
{
    struct Token *token;
    struct Token *lastTokens_ = NULL;
    struct Token *tokenHelper;
    int openParenthesisCount;
    double newTotal = 0;
    char *doubleToStringValue;
    struct Token *lastToken = *firstTokens;

    if (lastTokens)
    {
        lastTokens_ = *lastTokens;
    }

    //first do all * and / operations

    for (token = *firstTokens; token != lastTokens_; token = token->next)
    {
        switch (token->tokenId)
        {
            case TID_Operation:
            {
                switch (token->operationId)
                {
                    case '(':
                    {
                        //find matching parenthesis

                        tokenHelper = NULL;//will contain matching parenthesis
                        openParenthesisCount = 1;

                        for (tokenHelper = token->next; tokenHelper; tokenHelper = tokenHelper->next)
                        {
                            if (tokenHelper->operationId == '(')
                            {
                                openParenthesisCount++;
                            }
                            else if (tokenHelper->operationId == ')')
                            {
                                if (--openParenthesisCount == 0)
                                {
                                    //we have found a matching parenthesis, stop the loop here

                                    break;
                                }
                            }
                        }

                        if (!tokenHelper)
                        {
                            fprintf(stderr, "Could not find a matching parenthesis\n");

                            exit(1);
                        }

                        if (token->next == tokenHelper)
                        {
                            //nothing inside this parenthesis, remove them both

                            struct Token *nextToken = tokenHelper->next;

                            if (token->previous)
                            {
                                token->previous->next = nextToken;
                            }

                            if (nextToken)
                            {
                                nextToken->previous = token->previous;
                                tokenHelper->next->previous = nextToken;
                            }

                            if (token == *firstTokens)
                            {
                                *firstTokens = nextToken;
                            }

                            if (lastTokens && tokenHelper == *lastTokens)
                            {
                                *lastTokens = nextToken;
                            }

                            freeToken(token);
                            freeToken(tokenHelper);

                            //continue the loop

                            token = nextToken;
                            lastToken = token;

                            continue;
                        }

                        struct Token *result = exeTokenOperations(&token->next, &tokenHelper);

                        //result should contain the result inside the parenthesis
                        //now remove the two parenthesis

                        if (token->previous)
                        {
                            token->previous->next = result;
                        }

                        //always assign to next even if its NULL

                        result->previous = token->previous;

                        if (tokenHelper->next)
                        {
                            tokenHelper->next->previous = result;
                        }

                        //always assign to next even if its NULL

                        result->next = tokenHelper->next;

                        if (token == *firstTokens)
                        {
                            *firstTokens = result;
                        }

                        if (lastTokens && tokenHelper == *lastTokens)
                        {
                            *lastTokens = result;
                        }

                        freeToken(token);
                        freeToken(tokenHelper);

                        token = result;

                        break;
                    }
                    default:
                    {
                        if (!strchr(operators, token->operationId) && token->operationId != '=')
                        {
                            lastToken = token;
                            continue;
                        }

                        //previous token should be a value

                        if (!token->previous || token->previous->tokenId != TID_Value)
                        {
                            fprintf(stderr, "trying to do an operation without left operand\n");

                            exit(1);
                        }

                        if (!token->next)
                        {
                            fprintf(stderr, "trying to do an operation without right operand\n");

                            exit(1);
                        }

                        struct Token *nextResultToken = exeTokenOperations(&token->next, lastTokens);

                        if (nextResultToken->tokenId != TID_Value)
                        {
                            fprintf(stderr, "trying to do an operation without right operand\n");

                            exit(1);
                        }

                        struct Token *prevToken = token->previous;

                        int isVariable = 0;

                        if (token->operationId == '/')
                        {
                            newTotal = valueInToken(prevToken) / valueInToken(nextResultToken);
                        }
                        else if (token->operationId == '*')
                        {
                            newTotal = valueInToken(prevToken) * valueInToken(nextResultToken);
                        }
                        else if (token->operationId == '+')
                        {
                            newTotal = valueInToken(prevToken) + valueInToken(nextResultToken);
                        }
                        else if (token->operationId == '-')
                        {
                            newTotal = valueInToken(prevToken) - valueInToken(nextResultToken);
                        }
                        else if (token->operationId == '=')
                        {
                            addVariableWithName(prevToken->value, valueInToken(nextResultToken));
                            isVariable = 1;
                        }

                        if (token->operationId != '=')
                        {
                            dtos(&doubleToStringValue, newTotal);
                            prevToken->value = doubleToStringValue;
                        }

                        //remove the operation token and the nextResultToken, because prevToken have consumed them

                        prevToken->next = nextResultToken->next;

                        if (nextResultToken->next)
                        {
                            nextResultToken->next->previous = prevToken;
                        }

                        if (token == *firstTokens)
                        {
                            *firstTokens = prevToken;
                        }

                        if (lastTokens && nextResultToken == *lastTokens)
                        {
                            *lastTokens = prevToken;
                        }

                        freeToken(token);
                        freeToken(nextResultToken);

                        token = prevToken;

                        if (isVariable)
                        {
                            freeToken(token);

                            if (firstTokens)
                            {
                                *firstTokens = NULL;
                            }

                            if (lastTokens)
                            {
                                *lastTokens = NULL;
                            }

                            return NULL;
                        }
                    }
                }

                lastToken = token;

                break;
            }
            default: //TID_Value
            {
                lastToken = token;
            }
        }
    }

    return lastToken;
}

/* exeoperation
 *
 * executes an operation and returns the result in output
 */

void exeoperation(char **output, const char *source)
{
    char *operationPtr = NULL;
    char *operation = NULL;
    char *token;
    int tokenlen = 0;
    char currentchar;

    struct Token *tokenStruct;
    struct Token *tokens = NULL;
    
    if (!source)
    {
        return;
    }

    strByRemovingChar(&operation, source, ' ');
    operationPtr = operation;

    if (!operation)
    {
        goto freeOperation;

        return;
    }

    if (!strlen(operation))
    {
        return;
    }

    token = calloc(1, strlen(operation) + 1);

#ifdef DEBUG
    printf("exeoperation: %s\n", operation);
#endif

    //store the tokens in token array to execute operations in priority

    while ((currentchar = *operation++))
    {
        if (isTokenDelim(currentchar))
        {
            if (tokenlen > 0)
            {
                token[tokenlen] = '\0';

                tokenStruct = calloc(1, sizeof(struct Token));
                tokenStruct->value = strdup(token);
                tokenStruct->tokenId = TID_Value;
                addToken(&tokens, tokenStruct);
            }

            tokenStruct = calloc(1, sizeof(struct Token));
            tokenStruct->operationId = currentchar;
            tokenStruct->tokenId = TID_Operation;
            addToken(&tokens, tokenStruct);

            //prepare for next token

            memset(token, 0, tokenlen);
            tokenlen = 0;
        }
        else
        {
            if (currentchar == ' ' || currentchar == '\t')
            {
                continue;
            }

            token[tokenlen++] = currentchar;
        }
    }

    if (tokenlen > 0)
    {
        //only values can fall under this condition, token are handled in the above loop

        token[tokenlen] = '\0';

        tokenStruct = calloc(1, sizeof(struct Token));
        tokenStruct->value = strdup(token);
        tokenStruct->tokenId = TID_Value;
        addToken(&tokens, tokenStruct);
    }

    free(token);

#ifdef DEBUG
    printf("\nBEFORE Tokens for '%s':\n", operationPtr);

    for (tokenStruct = tokens; tokenStruct; tokenStruct = tokenStruct->next)
    {
        switch (tokenStruct->tokenId)
        {
            case TID_Operation:
            {
                printf("token: %c (operation)\n", tokenStruct->operationId);
                break;
            }
            case TID_Value:
            {
                printf("token: %s (value)\n", tokenStruct->value);
                break;
            }
        }
    }

    puts("");
#endif

    struct Token *tokenWithResult = exeTokenOperations(&tokens, NULL);

    if (tokenWithResult)
    {
        double value = valueInToken(tokenWithResult);
        dtos(output, value);

        if (tokenWithResult)
        {
            freeTokenArray(tokenWithResult);
        }
    }

freeOperation:

    free(operationPtr);
}

/* rsvgcstring
 *
 * compiles any opration inside <!--- --->
 * returns the result in the output argument
 * the caller is responsible of freeing output
 */

void svgcstring(char **output, const char *source)
{
    int reti;
    const size_t nmatch = 5;
    int imatch;
    regmatch_t matchPtr[nmatch]; //array of results

    //operation variables

    size_t oplen;
    char *operations;
    char *operations_pending; // needed to loop through operations
    char *operation;
    char *carptr;
    char *operationresult;
    regmatch_t last_matchptr = {-1, -1};

#ifdef DEBUG
    char regexMsg[100];
#endif

    if (!source)
    {
        return;
    }

    *output = calloc(1, strlen(source));

    //first check for no matches

    if ((reti = regexec(&regex, source, nmatch, matchPtr, 0)) != 0)
    {
        //no match, copy entire row to the new file.

#ifdef DEBUG
        regerror(reti, &regex, regexMsg, sizeof(regexMsg));
        printf("%s\n", regexMsg);
#endif
        *output = strdup(source);

        return;
    }

    //Execute regular expression

    //regexec stops on first match, doing a while to simulate Pearl's /g option
    //Reference: http://stackoverflow.com/questions/16417454/why-regexec-in-posix-c-always-return-the-first-match-how-can-it-return-all-mat

    while (!(reti = regexec(&regex, source, nmatch, matchPtr, 0)))
    {
        for (imatch = 0; imatch < nmatch; imatch++)
        {
            regmatch_t matchptr = matchPtr[imatch];

            if (matchptr.rm_so < 0)
            {
                // we are finished, no more matches.

                break;
            }

            last_matchptr = matchptr;

            //add everything before the operation

            strncat(*output, source, matchptr.rm_so);

            //get the operation string without the LEFT_ID and RIGHT_ID strings.

            oplen = matchptr.rm_eo - matchptr.rm_so - 5 /*LEFT_ID*/ - 4 /*RIGHT_ID*/ + 1 /*null*/;
            operations = malloc(oplen);
            strncpy(operations, source + matchptr.rm_so + 5 /*LEFT_ID*/, oplen - 1);
            operations[oplen - 1] = '\0';
            operations_pending = operations;

            while ((carptr = strchr(operations_pending, ';')))
            {
                int operationsize = (int)(carptr - operations_pending + 1);
                operation = malloc(operationsize);
                strncpy(operation, operations_pending, operationsize - 1);

#ifdef DEBUG
                printf("operation with ';' ");
#endif

                operationresult = NULL;
                exeoperation(&operationresult, operations_pending);

                if (operationresult)
                {
#ifdef DEBUG
                    printf("operationresult: %s\n", operationresult);
#endif
                    free(operationresult);
                }

                free(operation);
            }

            //check for pending operations without semicolon

            if (strlen(operations_pending))
            {
#ifdef DEBUG
                printf("operation without ';' ");
#endif
                operationresult = NULL;
                exeoperation(&operationresult, operations_pending);

                if (operationresult)
                {
#ifdef DEBUG
                    printf("operationresult: %s\n", operationresult);
#endif

                    strcat(*output, operationresult);
                }
            }

            free(operations);

            source += last_matchptr.rm_eo;
        }
    }

    //add everything after the operation

    strcat(*output, source);


}

