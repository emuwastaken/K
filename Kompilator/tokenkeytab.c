#include <stddef.h>
#include "tokenkeytab.h"
#include <string.h>

#include <string.h>



const Keyword keywords[] = {

    /* Structure */
    { "PROGRAM",     TOK_PROGRAM },
    { "FÖRFATTARE",  TOK_FORFATTARE },
    //{ "ENTRE",       TOK_ENTRE },

    /* Types */
    { "HEL",         TOK_HEL },
    { "FLYT",        TOK_FLYT },
    { "BOK",         TOK_BOK },
    { "BIT",         TOK_BIT },
    { "HALV",        TOK_HALV },
    { "BYTE",        TOK_BYTE },
    { "ORD",         TOK_ORD },
    { "VAL",         TOK_VAL },
    { "STRUKTUR",    TOK_STRUKTUR },
    { "FÄLT",        TOK_FALT },
    { "TOM",         TOK_TOM },
    { "TYPDEF",      TOK_TYPDEF},

    /* Arithmetic operators */
    { "+",           TOK_PLUS },
    { "-",           TOK_MINUS },
    { "*",           TOK_MUL },
    { "/",           TOK_DIV },


    /* Delimiters */
    { "(",           TOK_LPAREN },
    { ")",           TOK_RPAREN },
    { "<",           TOK_LBLOCK },
    { ">",           TOK_RBLOCK },
    { ",",           TOK_COMMA },
    { ";",           TOK_SEMI },
    { ":",           TOK_ASSIGN },
    { ".",           TOK_DOT },

    /* Qualifiers */
    { "PEK",         TOK_PEK },
    { "KONSTANT",    TOK_KONSTANT },
    { "STATISK",     TOK_STATISK },

    /* Comparison */
    { "LIKA",        TOK_EQ },
    { "INTE LIKA",   TOK_NEQ },
    { "MINDRE",      TOK_LT },
    { "STÖRRE",      TOK_GT },
    { "MINLIK",      TOK_LTE },
    { "STÖLIK",      TOK_GTE },

    /* Selection / repetition */
    { "OM",          TOK_OM },
    { "ANNARS",      TOK_ANNARS },
    { "MEDAN",       TOK_MEDAN },
    { "GÖR",         TOK_GOR },
    { "FÖR",         TOK_FOR },
    { "VÄXEL",       TOK_VAXEL },
    { "FALL",        TOK_FALL },
    { "BRYT",        TOK_BRYT },
    { "FORTSATT",    TOK_FORTSATT },

    /* Boolean */
    { "OCH",         TOK_OCH },
    { "ELLER",       TOK_ELLER },
    { "INTE",        TOK_INTE },

    /* Flow */
    { "ÅTERVÄND",    TOK_ATERVAND },

    /* Operators (single-word first) */
    { "ÖKAR",        TOK_OKAR },
    { "MINSKAR",     TOK_MINSKAR },

    /* Composite operators */
    { "ÖKAR MED",    TOK_PLUS_ASSIGN },
    { "MINSKAR MED", TOK_MINUS_ASSIGN },
    { "MULT MED",    TOK_MUL_ASSIGN },
    { "DELAS MED",   TOK_DIV_ASSIGN },

    /* Addressing */
    { "VÄRDE VID",   TOK_DEREF },
    { "ADRESS AV",   TOK_ADDRESS },
    { "STORLEK AV",  TOK_STORLEKAV },

    /* Sentinel */
    { NULL,          TOK_ERROR }
};





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
    if (is_string_literal(lexeme))
        return TOK_STRING_LIT;

    if (is_number_literal(lexeme))
        return TOK_INT_LIT;

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

const char * tok2name(TokenType tok)
{
    switch (tok)
    {
        case TOK_PROGRAM:        return "TOK_PROGRAM";
        case TOK_FORFATTARE:     return "TOK_FORFATTARE";
        //case TOK_ENTRE:          return "TOK_ENTRE";
        case TOK_HEL:            return "TOK_HEL";
        case TOK_FLYT:           return "TOK_FLYT";
        case TOK_BOK:            return "TOK_BOK";
        case TOK_BIT:            return "TOK_BIT";
        case TOK_HALV:           return "TOK_HALV";
        case TOK_BYTE:           return "TOK_BYTE";
        case TOK_ORD:            return "TOK_ORD";
        case TOK_VAL:            return "TOK_VAL";
        case TOK_STRUKTUR:       return "TOK_STRUKTUR";
        case TOK_FALT:           return "TOK_FALT";
        case TOK_TOM:            return "TOK_TOM";
        case TOK_PEK:            return "TOK_PEK";
        case TOK_KONSTANT:       return "TOK_KONSTANT";
        case TOK_STATISK:        return "TOK_STATISK";
        case TOK_OM:             return "TOK_OM";
        case TOK_ANNARS:         return "TOK_ANNARS";
        case TOK_MEDAN:          return "TOK_MEDAN";
        case TOK_GOR:            return "TOK_GOR";
        case TOK_FOR:            return "TOK_FOR";
        case TOK_VAXEL:          return "TOK_VAXEL";
        case TOK_FALL:           return "TOK_FALL";
        case TOK_BRYT:           return "TOK_BRYT";
        case TOK_FORTSATT:       return "TOK_FORTSATT";
        case TOK_ATERVAND:       return "TOK_ATERVAND";
        case TOK_PLUS_ASSIGN:    return "TOK_PLUS_ASSIGN";
        case TOK_LPAREN:   return "TOK_LPAREN";
        case TOK_RPAREN:   return "TOK_RPAREN";
        case TOK_LBLOCK:   return "TOK_LBLOCK";
        case TOK_RBLOCK:   return "TOK_RBLOCK";
        case TOK_SEMI:     return "TOK_SEMI";
        case TOK_COMMA:    return "TOK_COMMA";
        case TOK_ASSIGN:   return "TOK_ASSIGN";
        case TOK_DOT:      return "TOK_DOT";
        case TOK_STRING_LIT:     return "TOK_STRING_LIT";
        case TOK_TYPDEF:         return "TOK_TYPDEF";

        case TOK_PLUS:      return "TOK_PLUS";
        case TOK_MINUS:    return "TOK_MINUS";
        case TOK_MUL:      return "TOK_MUL";
        case TOK_DIV:      return "TOK_DIV";
        case TOK_EXP:      return "TOK_EXP";

        case TOK_OCH:      return "TOK_OCH";
        case TOK_ELLER:    return "TOK_ELLER";
        case TOK_INTE:     return "TOK_INTE";

        case TOK_ADDRESS:  return "TOK_ADDRESS";
        case TOK_DEREF:    return "TOK_DEREF";
        case TOK_STORLEKAV:return "TOK_STORLEKAV";



        case TOK_MINSKAR:        return "TOK_MINSKAR";
        case TOK_OKAR:           return "TOK_OKAR";
        case TOK_MINUS_ASSIGN:   return "TOK_MINUS_ASSIGN";
        case TOK_MUL_ASSIGN:     return "TOK_MUL_ASSIGN";
        case TOK_DIV_ASSIGN:     return "TOK_DIV_ASSIGN";
        case TOK_EQ:             return "TOK_EQ";
        case TOK_NEQ:            return "TOK_NEQ";
        case TOK_LT:             return "TOK_LT";
        case TOK_GT:             return "TOK_GT";
        case TOK_LTE:            return "TOK_LTE";
        case TOK_GTE:            return "TOK_GTE";



        case TOK_IDENTIFIER:     return "TOK_IDENTIFIER";
        case TOK_INT_LIT:        return "TOK_INT_LIT";
        case TOK_FLOAT_LIT:      return "TOK_FLOAT_LIT";
        case TOK_EOF:            return "TOK_EOF";
        case TOK_ERROR:          return "TOK_ERROR";
        default:                 return "TOK_UNKNOWN";
    }
}

const char * tok2lexeme(TokenType tok)
{
    switch (tok)
    {
        /* Structure */
        case TOK_PROGRAM:        return "PROGRAM";
        case TOK_FORFATTARE:     return "FÖRFATTARE";
        //case TOK_ENTRE:          return "ENTRE";

        /* Types */
        case TOK_HEL:            return "HEL";
        case TOK_FLYT:           return "FLYT";
        case TOK_BOK:            return "BOK";
        case TOK_BIT:            return "BIT";
        case TOK_HALV:           return "HALV";
        case TOK_BYTE:           return "BYTE";
        case TOK_ORD:            return "ORD";
        case TOK_VAL:            return "VAL";
        case TOK_STRUKTUR:       return "STRUKTUR";
        case TOK_FALT:           return "FÄLT";
        case TOK_TOM:            return "TOM";
        case TOK_TYPDEF:         return "TYPDEF";

        /* Qualifiers */
        case TOK_PEK:            return "PEK";
        case TOK_KONSTANT:       return "KONSTANT";
        case TOK_STATISK:        return "STATISK";

        /* Control flow */
        case TOK_OM:             return "OM";
        case TOK_ANNARS:         return "ANNARS";
        case TOK_MEDAN:          return "MEDAN";
        case TOK_GOR:            return "GÖR";
        case TOK_FOR:            return "FÖR";
        case TOK_ATERVAND:       return "ÅTERVÄND";

        /* Operators */
        case TOK_OKAR:     return "ÖKAR";
        case TOK_MINSKAR:  return "MINSKAR";

        case TOK_PLUS_ASSIGN:    return "ÖKAR MED";
        case TOK_MINUS_ASSIGN:   return "MINSKAR MED";
        case TOK_MUL_ASSIGN:     return "MULT MED";
        case TOK_DIV_ASSIGN:     return "DELAS MED";

        case TOK_ASSIGN:         return ":";
        case TOK_PLUS:           return "+";
        case TOK_MINUS:          return "-";
        case TOK_MUL:            return "*";
        case TOK_DIV:            return "/";
        case TOK_EXP:            return "^";

        /* Delimiters */
        case TOK_LPAREN:         return "(";
        case TOK_RPAREN:         return ")";
        case TOK_LBLOCK:         return "<";
        case TOK_RBLOCK:         return ">";
        case TOK_SEMI:           return ";";
        case TOK_COMMA:          return ",";

        case TOK_EQ:   return "LIKA";
        case TOK_NEQ:  return "INTE LIKA";
        case TOK_LT:   return "MINDRE";
        case TOK_GT:   return "STÖRRE";
        case TOK_LTE:  return "MINLIK";
        case TOK_GTE:  return "STÖLIK";


        /* Generic categories */
        case TOK_IDENTIFIER:     return "identifier";
        case TOK_INT_LIT:        return "integer literal";
        case TOK_FLOAT_LIT:      return "float literal";
        case TOK_STRING_LIT:     return "string literal";

        case TOK_EOF:            return "<EOF>";
        case TOK_ERROR:          return "<ERROR>";

        default:                 return "<UNKNOWN>";
    }
}

static int is_string_literal(const char *lexeme)
{
    size_t len = strlen(lexeme);

    // must be at least ""
    if (len < 2)
        return 0;

    // must start and end with "
    if (lexeme[0] != '"' || lexeme[len - 1] != '"')
        return 0;

    // inner chars may not contain ", \n, or \r
    for (size_t i = 1; i < len - 1; i++)
    {
        if (lexeme[i] == '"' ||
            lexeme[i] == '\n' ||
            lexeme[i] == '\r')
            return 0;
    }

    return 1;
}

static int is_number_literal(const char *lexeme)
{
    int i = 0;
    int seen_digit = 0;
    int seen_dot = 0;

    while (lexeme[i] != '\0')
    {
        if (lexeme[i] >= '0' && lexeme[i] <= '9')
        {
            seen_digit = 1;
        }
        else if (lexeme[i] == '.')
        {
            if (seen_dot)
                return 0;          // second dot not allowed
            seen_dot = 1;
        }
        else
        {
            return 0;              // illegal character
        }
        i++;
    }

    return seen_digit;             // must contain at least one digit
}

