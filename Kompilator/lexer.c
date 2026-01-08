// ---------------------------------------------------
// Lexer for K-language (UTF-8 safe)
// Author: Emu Nilsson
// ---------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "lexer.h"

#define MEGABYTE 1024
#define MULTIPLIER 25
#define MAX_LEXEME_LEN 128
#define LEXSIZE (MEGABYTE * MULTIPLIER)

// ---------------------------------------------------
// Lexer state (wide characters)
// ---------------------------------------------------
static wchar_t input_stream[LEXSIZE];
static int index_pos;
static int row;
static int col;

// ---------------------------------------------------
// Static prototypes
// ---------------------------------------------------
static void skip_whitespace_and_comments(void);
static Token lex_identifier_or_keyword(void);
static Token lex_number(void);
static Token lex_two_char_symbol(void);
static Token lex_one_char_symbol(void);

static wchar_t peek(void);
static wchar_t peek_next(void);
static void advance(void);
static int is_eof(void);
static int is_letter(wchar_t c);
static void lexer_error(const char* msg);
static Token make_token(TokenType type, const wchar_t* lexeme, int line, int column);

// ---------------------------------------------------
// Initialize lexer (UTF-8 → wchar_t)
// ---------------------------------------------------
void lexer_init(const wchar_t* src)
{
    wcsncpy(input_stream, src, LEXSIZE - 1);
    input_stream[LEXSIZE - 1] = L'\0';

    index_pos = 0;
    row = 1;
    col = 1;
}


// ---------------------------------------------------
// Fetch next token
// ---------------------------------------------------
Token next_token(void)
{
    skip_whitespace_and_comments();

    if (is_eof())
        return make_token(TOK_EOF, L"", row, col);

    wchar_t c = peek();

    if (is_letter(c))
        return lex_identifier_or_keyword();

    if (iswdigit(c))
        return lex_number();

    if ((peek() == L'%' && peek_next() == L'%') ||
        (peek() == L'%' && peek_next() == L'&') ||
        (peek() == L'&' && peek_next() == L'%'))
        return lex_two_char_symbol();

    if (wcschr(L"()<>;:+-*/,#£", c))
        return lex_one_char_symbol();

    lexer_error("Illegal character");
    advance();
    return make_token(TOK_ERROR, L"", row, col);
}

// ---------------------------------------------------
// Helpers
// ---------------------------------------------------
static wchar_t peek(void) { return input_stream[index_pos]; }
static wchar_t peek_next(void) { return input_stream[index_pos + 1]; }

static void advance(void)
{
    if (input_stream[index_pos] == L'\n') {
        row++;
        col = 1;
    } else {
        col++;
    }
    index_pos++;
}

static int is_eof(void)
{
    return input_stream[index_pos] == L'\0';
}

static int is_letter(wchar_t c)
{
    return iswalpha(c) || c == L'_';
}

// ---------------------------------------------------
// Skip whitespace and comments
// ---------------------------------------------------
static void skip_whitespace_and_comments(void)
{
    int done = 0;

    while (!done)
    {
        while (iswspace(peek()))
            advance();

        if (peek() == L'%' && peek_next() == L'%')
        {
            advance(); advance();
            while (!is_eof() && peek() != L'\n')
                advance();
            continue;
        }

        if (peek() == L'%' && peek_next() == L'&')
        {
            advance(); advance();
            while (!is_eof())
            {
                if (peek() == L'&' && peek_next() == L'%')
                {
                    advance(); advance();
                    break;
                }
                advance();
            }
            continue;
        }

        done = 1;
    }
}

// ---------------------------------------------------
// Identifier / keyword
// ---------------------------------------------------
static Token lex_identifier_or_keyword(void)
{
    wchar_t buffer[MAX_LEXEME_LEN];
    int start_col = col;
    int len = 0;

    while (!is_eof() && (is_letter(peek()) || iswdigit(peek())))
    {
        if (len < MAX_LEXEME_LEN - 1)
            buffer[len++] = peek();
        advance();
    }

    buffer[len] = L'\0';

    if (wcscmp(buffer, L"HEL") == 0 ||
        wcscmp(buffer, L"FLYT") == 0 ||
        wcscmp(buffer, L"ÅTERVÄND") == 0 ||
        wcscmp(buffer, L"OM") == 0 ||
        wcscmp(buffer, L"ANNARS") == 0 ||
        wcscmp(buffer, L"FÖRFATTARE") == 0)
        return make_token(TOK_KEYWORD, buffer, row, start_col);

    return make_token(TOK_ID, buffer, row, start_col);
}

// ---------------------------------------------------
// Number
// ---------------------------------------------------
static Token lex_number(void)
{
    wchar_t buffer[MAX_LEXEME_LEN];
    int start_col = col;
    int len = 0;

    while (!is_eof() && iswdigit(peek()))
    {
        if (len < MAX_LEXEME_LEN - 1)
            buffer[len++] = peek();
        advance();
    }

    buffer[len] = L'\0';
    return make_token(TOK_NUMBER, buffer, row, start_col);
}

// ---------------------------------------------------
// Symbols
// ---------------------------------------------------
static Token lex_two_char_symbol(void)
{
    wchar_t buffer[3];
    int start_col = col;

    buffer[0] = peek();
    buffer[1] = peek_next();
    buffer[2] = L'\0';

    advance(); advance();
    return make_token(TOK_SYMBOL, buffer, row, start_col);
}

static Token lex_one_char_symbol(void)
{
    wchar_t buffer[2];
    int start_col = col;

    buffer[0] = peek();
    buffer[1] = L'\0';

    advance();
    return make_token(TOK_SYMBOL, buffer, row, start_col);
}

// ---------------------------------------------------
// Token creation (wide → UTF-8)
// ---------------------------------------------------
static Token make_token(TokenType type, const wchar_t* lexeme, int line, int column)
{
    Token t;
    t.type = type;
    wcstombs(t.lexeme, lexeme, MAX_LEXEME_LEN);
    t.line = line;
    t.column = column;
    return t;
}

// ---------------------------------------------------
static void lexer_error(const char* msg)
{
    fprintf(stderr,
        "Lexer error at line %d, column %d: %s\n",
        row, col, msg);
}
