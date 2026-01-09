#include <stddef.h>
#include "tokenkeytab.h"


//Returns the token type of composite keywords, if they are compatible
TokenType lookup_pair(char *lexeme, char *lextwo)
{
    char composite[64];

    // build "LEXEME LEXTWO"
    strcpy(composite, lexeme);
    strcat(composite, " ");
    strcat(composite, lextwo);

    for (int i = 0; keywords[i].lexeme != NULL; i++)
    {
        // match only multi-word keywords
        if (strchr(keywords[i].lexeme, ' ') == NULL)
            continue;

        if (strcmp(composite, keywords[i].lexeme) == 0)
            return keywords[i].token;
    }

    return TOK_ERROR;
}

//Returns the token type of singular keywords, skipping composites
TokenType lookup(char *lexeme)
{
    for (int i = 0; keywords[i].lexeme != NULL; i++)
    {
        // skip multi-word keywords
        if (strchr(keywords[i].lexeme, ' ') != NULL)
            continue;

        if (strcmp(lexeme, keywords[i].lexeme) == 0)
            return keywords[i].token;
    }

    return TOK_IDENTIFIER;
}

