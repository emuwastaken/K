#ifndef LEXER_H
#define LEXER_H

#include "utf_decoder.h"
#include "tokenkeytab.h"

typedef enum LexemeKind {
    LEX_ID,
    LEX_INT,
    LEX_FLOAT,
    LEX_STRING,
    LEX_DELIM
} LexemeKind;



void lexer(
    CharacterUnit *decode_buffer,
    int char_count,
    TokenType **out_tokens,
    int *out_token_count
);

#endif
