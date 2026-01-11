#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "lexer.h"
#include "tokenkeytab.h"

#define MAXLEXSIZE      128
#define SPACE           0x0020
#define HORIZONTAL_TAB  0x0009
#define NEWLINE         0x000A
#define CARRIAGE_RETURN 0x000D

LexemeKind lexeme_kind(TokenType tok);

typedef struct LexState {

    char **lexemes;
    int  *lexeme_row;
    int  *lexeme_col;

    int lexeme_count;
    int lexeme_capacity;

    TokenBuffer *tokens;
    int token_count;
    int token_capacity;

    CharacterUnit *char_stream;
    int char_count;
    int lex_index;

    int row;
    int col;

} LexState;

static CharacterUnit getCurrent(LexState state);
static CharacterUnit getLookahead(LexState state);

static void identifyID(LexState *state);
static void identifyNum(LexState *state);
static void identifyDelim(LexState *state);
static void identifyString(LexState *state);

int whitespace[4] = { SPACE, HORIZONTAL_TAB, NEWLINE, CARRIAGE_RETURN };

/* -------------------------------------------------- */
/* LEXER ENTRY POINT                                  */
/* -------------------------------------------------- */
void lexer(
    CharacterUnit *decode_buffer,
    int char_count,
    TokenBuffer **out_tokens,
    int *out_token_count
)
{
    LexState state = {0};

    state.char_stream = decode_buffer;
    state.char_count  = char_count;
    state.row = 1;
    state.col = 1;

    state.lexeme_capacity = 16;
    state.lexemes    = malloc(sizeof(char*) * state.lexeme_capacity);
    state.lexeme_row = malloc(sizeof(int)    * state.lexeme_capacity);
    state.lexeme_col = malloc(sizeof(int)    * state.lexeme_capacity);

    printf("LEXER START: char_count=%d\n", char_count);

    while (state.lex_index < state.char_count)
    {
        CharacterUnit current = getCurrent(state);

        printf(
            "[LOOP] idx=%d row=%d col=%d cp=U+%04X ('%c')\n",
            state.lex_index,
            state.row,
            state.col,
            current.codepoint,
            (current.codepoint >= 32 && current.codepoint < 127)
                ? (char)current.codepoint
                : '?'
        );

        int handled = 0;

        /* ---------------- Whitespace ---------------- */
        for (int i = 0; i < 4; i++)
        {
            if (current.codepoint == whitespace[i])
            {
                printf("  WHITESPACE\n");

                if (current.codepoint == NEWLINE)
                {
                    state.row++;
                    state.col = 1;
                }
                else
                {
                    state.col++;
                }

                state.lex_index++;
                handled = 1;
                break;
            }
        }
        if (handled) continue;

        /* ---------------- Comments ---------------- */
        if (current.codepoint == '/' && state.lex_index + 1 < state.char_count)
        {
            CharacterUnit la = getLookahead(state);

            if (la.codepoint == '/')
            {
                printf("  COMMENT //\n");

                state.lex_index += 2;
                state.col += 2;

                while (state.lex_index < state.char_count &&
                       state.char_stream[state.lex_index].codepoint != NEWLINE)
                {
                    state.lex_index++;
                    state.col++;
                }
                continue;
            }

            if (la.codepoint == '%')
            {
                printf("  COMMENT /%% %%/\n");

                state.lex_index += 2;
                state.col += 2;

                while (state.lex_index + 1 < state.char_count)
                {
                    if (state.char_stream[state.lex_index].codepoint == '%' &&
                        state.char_stream[state.lex_index + 1].codepoint == '/')
                    {
                        state.lex_index += 2;
                        state.col += 2;
                        break;
                    }

                    if (state.char_stream[state.lex_index].codepoint == NEWLINE)
                    {
                        state.row++;
                        state.col = 1;
                    }
                    else
                    {
                        state.col++;
                    }

                    state.lex_index++;
                }
                continue;
            }
        }

        /* ---------------- String ---------------- */
        if (current.codepoint == '"')
        {
            printf("  STRING START\n");
            identifyString(&state);
            continue;
        }

        /* ---------------- Identifier ---------------- */
        if ((current.codepoint >= 'a' && current.codepoint <= 'z') ||
            (current.codepoint >= 'A' && current.codepoint <= 'Z') ||
            current.codepoint == 0x00E5 || current.codepoint == 0x00E4 ||
            current.codepoint == 0x00F6 || current.codepoint == 0x00C5 ||
            current.codepoint == 0x00C4 || current.codepoint == 0x00D6)
        {
            printf("  IDENTIFIER START\n");
            identifyID(&state);
            continue;
        }

        /* ---------------- Number ---------------- */
        if (current.codepoint >= '0' && current.codepoint <= '9')
        {
            printf("  NUMBER START\n");
            identifyNum(&state);
            continue;
        }

        /* ---------------- Delimiter ---------------- */
        if (current.codepoint == '(' || current.codepoint == ')' ||
            current.codepoint == '<' || current.codepoint == '>' ||
            current.codepoint == ';' || current.codepoint == ',' ||
            current.codepoint == ':')
        {
            printf("  DELIMITER '%c'\n", (char)current.codepoint);
            identifyDelim(&state);
            continue;
        }

        /* ---------------- Fallback ---------------- */
        printf("  FALLBACK ADVANCE\n");
        state.lex_index++;
        state.col++;
    }

    printf("LEXING DONE: lexemes=%d\n", state.lexeme_count);

    /* -------------------------------------------------- */
    /* TOKEN RESOLUTION                                   */
    /* -------------------------------------------------- */
    state.token_capacity = 16;
    state.tokens = malloc(sizeof(TokenBuffer) * state.token_capacity);

    for (int i = 0; i < state.lexeme_count; i++)
    {
        char *lex  = state.lexemes[i];
        char *next = (i + 1 < state.lexeme_count) ? state.lexemes[i + 1] : NULL;

        printf("RESOLVE[%d]: '%s'\n", i, lex);

        TokenType tok = TOK_ERROR;

        if      (!strcmp(lex, "(")) tok = TOK_LPAREN;
        else if (!strcmp(lex, ")")) tok = TOK_RPAREN;
        else if (!strcmp(lex, "<")) tok = TOK_LBLOCK;
        else if (!strcmp(lex, ">")) tok = TOK_RBLOCK;
        else if (!strcmp(lex, ";")) tok = TOK_SEMI;
        else if (!strcmp(lex, ",")) tok = TOK_COMMA;
        else if (!strcmp(lex, ":")) tok = TOK_ASSIGN;
        else if (next)
        {
            tok = lookup_pair(lex, next);
            if (tok != TOK_ERROR)
            {
                printf("  COMPOSITE WITH '%s'\n", next);
                i++;
            }
        }

        if (tok == TOK_ERROR)
            tok = lookup(lex);

        printf("  -> TOKEN %d\n", tok);

        state.tokens[state.token_count++] = (TokenBuffer){
            .token  = tok,
            .kind   = lexeme_kind(tok),
            .row    = state.lexeme_row[i],
            .col    = state.lexeme_col[i],
            .lexeme = strdup(lex)
        };
    }

    *out_tokens = state.tokens;
    *out_token_count = state.token_count;

    printf("TOKENS EMITTED: %d\n", state.token_count);

    for (int i = 0; i < state.lexeme_count; i++)
        free(state.lexemes[i]);

    free(state.lexemes);
    free(state.lexeme_row);
    free(state.lexeme_col);
}

/* -------------------------------------------------- */
/* HELPERS                                            */
/* -------------------------------------------------- */
static CharacterUnit getCurrent(LexState state) {
    return state.char_stream[state.lex_index];
}

static CharacterUnit getLookahead(LexState state) {
    return state.char_stream[state.lex_index + 1];
}

/* -------------------------------------------------- */
/* LEXEME SCANNERS                                    */
/* -------------------------------------------------- */
static void store_lexeme(LexState *s, const char *text, int len, int row, int col)
{
    printf("  STORE lexeme='%.*s' at %d:%d\n", len, text, row, col);

    if (s->lexeme_count >= s->lexeme_capacity)
    {
        s->lexeme_capacity *= 2;
        s->lexemes    = realloc(s->lexemes,    sizeof(char*) * s->lexeme_capacity);
        s->lexeme_row = realloc(s->lexeme_row, sizeof(int)    * s->lexeme_capacity);
        s->lexeme_col = realloc(s->lexeme_col, sizeof(int)    * s->lexeme_capacity);
    }

    char *copy = malloc(len + 1);
    memcpy(copy, text, len);
    copy[len] = '\0';

    s->lexemes[s->lexeme_count] = copy;
    s->lexeme_row[s->lexeme_count] = row;
    s->lexeme_col[s->lexeme_count] = col;
    s->lexeme_count++;
}

static void identifyID(LexState *s)
{
    int start_row = s->row;
    int start_col = s->col;
    char buf[MAXLEXSIZE];
    int len = 0;

    while (s->lex_index < s->char_count)
    {
        CharacterUnit cu = s->char_stream[s->lex_index];
        int cp = cu.codepoint;

        int is_letter =
            (cp >= 'a' && cp <= 'z') ||
            (cp >= 'A' && cp <= 'Z') ||
            cp == 0x00E5 || cp == 0x00E4 || cp == 0x00F6 ||   /* å ä ö */
            cp == 0x00C5 || cp == 0x00C4 || cp == 0x00D6;    /* Å Ä Ö */

        int is_digit = (cp >= '0' && cp <= '9');
        int is_extra = (cp == '_' || cp == '.');

        if (!(is_letter || is_digit || is_extra))
            break;

        for (int i = 0; i < cu.byte_length; i++)
        {
            if (len >= MAXLEXSIZE - 1)
                break;
            buf[len++] = cu.bytes[i];
        }

        s->lex_index++;
        s->col++;
    }

    /* SAFETY: never allow empty identifiers */
    if (len == 0)
    {
        /* consume one character to guarantee progress */
        s->lex_index++;
        s->col++;
        return;
    }

    store_lexeme(s, buf, len, start_row, start_col);
}



static void identifyNum(LexState *s)
{
    printf("    identifyNum()\n");

    int start_row = s->row;
    int start_col = s->col;
    char buf[MAXLEXSIZE];
    int len = 0, dot = 0;

    while (s->lex_index < s->char_count)
    {
        int cp = s->char_stream[s->lex_index].codepoint;
        if (cp == '.' && !dot) dot = 1;
        else if (!isdigit(cp)) break;

        buf[len++] = s->char_stream[s->lex_index].bytes[0];
        s->lex_index++;
        s->col++;
    }

    store_lexeme(s, buf, len, start_row, start_col);
}

static void identifyString(LexState *s)
{
    printf("    identifyString()\n");

    int start_row = s->row;
    int start_col = s->col;
    char buf[MAXLEXSIZE];
    int len = 0;

    s->lex_index++;
    s->col++;

    while (s->lex_index < s->char_count)
    {
        CharacterUnit cu = s->char_stream[s->lex_index];
        if (cu.codepoint == '"')
        {
            s->lex_index++;
            s->col++;
            break;
        }

        buf[len++] = cu.bytes[0];
        s->lex_index++;
        s->col++;
    }

    store_lexeme(s, buf, len, start_row, start_col);
}

static void identifyDelim(LexState *s)
{
    printf("    identifyDelim()\n");

    int start_row = s->row;
    int start_col = s->col;
    char c = s->char_stream[s->lex_index].bytes[0];

    store_lexeme(s, &c, 1, start_row, start_col);

    s->lex_index++;
    s->col++;
}

LexemeKind lexeme_kind(TokenType tok)
{
    switch (tok)
    {
        case TOK_IDENTIFIER:   return LEX_ID;
        case TOK_INT_LIT:      return LEX_INT;
        case TOK_FLOAT_LIT:    return LEX_FLOAT;
        case TOK_STRING_LIT:   return LEX_STRING;

        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_LBLOCK:
        case TOK_RBLOCK:
        case TOK_SEMI:
        case TOK_COMMA:
        case TOK_ASSIGN:
            return LEX_DELIM;

        default:
            return LEX_KEYWORD;
    }
}
