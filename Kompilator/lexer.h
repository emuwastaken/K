// ---------------------------------------------------
// lexer.h
// Public interface and token definitions for K-language lexer
// Author: Emu Nilsson
// ---------------------------------------------------

#ifndef LEXER_H
#define LEXER_H

// ---------------------------------------------------
// Token types produced by the lexer
// ---------------------------------------------------
typedef enum {
    TOK_EOF = 0,
    TOK_KEYWORD,
    TOK_ID,
    TOK_NUMBER,
    TOK_SYMBOL,
    TOK_ERROR
} TokenType;

// ---------------------------------------------------
// Token structure
// ---------------------------------------------------
typedef struct {
    TokenType type;      // kind of token
    char lexeme[128];    // raw lexeme text
    int line;            // line number
    int column;          // column number
} Token;

// ---------------------------------------------------
// Lexer public API
// ---------------------------------------------------

// Initializes lexer with source buffer
void lexer_init(const wchar_t* src);

// Returns next token from input stream
Token next_token(void);

#endif // LEXER_H
