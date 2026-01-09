#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "tokenkeytab.h"


#define MAXLEXSIZE      128
#define SPACE           0x0020
#define HORIZONTAL_TAB  0x0009
#define NEWLINE         0x000A
#define CARRIAGE_RETURN 0x000D



typedef struct LexState {

    LexemeKind * lexeme_kinds;

    char ** lexemes;        // array of C strings
    int lexeme_count;       // how many are stored
    int lexeme_capacity;    // allocated slots

    TokenType * tokens;
    int token_count;
    int token_capacity;

    CharacterUnit * char_stream;
    int char_count;
    int lex_index;
} LexState;


int whitespace[4] = {SPACE, HORIZONTAL_TAB, NEWLINE, CARRIAGE_RETURN}; 

static CharacterUnit getCurrent(LexState state);
static CharacterUnit getLookahead(LexState state);

static void identifyID(LexState *state);
static void identifyNum(LexState *state);
static void identifyDelim(LexState *state);
static void identifyString(LexState *state);


void lexer(
    CharacterUnit *decode_buffer,
    int char_count,
    TokenType **out_tokens,
    int *out_token_count
)   
{

    LexState state;
    state.char_stream = decode_buffer;
    state.char_count = char_count;
    state.lex_index = 0;

    state.lexeme_capacity = 16;
    state.lexeme_count = 0;
    state.lexemes = malloc(sizeof(char*) * state.lexeme_capacity);
    if (!state.lexemes) {
        // handle allocation failure
    }

    //printf("CP: U+%04X\n", state.char_stream[0].codepoint);


    while(state.lex_index < state.char_count)
    {
        CharacterUnit current = getCurrent(state);
        CharacterUnit lookahead;
        if(state.lex_index + 1 < state.char_count){
            lookahead = getLookahead(state);
        } else {
            //lookahead = EOF 
        }



        //Check for whitespaces
        int is_ws = 0;
        for (int i = 0; i < sizeof(whitespace) / sizeof(whitespace[0]); i++) {
            if (current.codepoint == whitespace[i]) {
                is_ws = 1;
                break;
            }
        }

        if (is_ws) {
            state.lex_index++;     // consume
            continue;              
        }

        // Check for comments
        if (current.codepoint == '/' && state.lex_index + 1 < state.char_count)
        {
            CharacterUnit la = getLookahead(state);

            // single-line //
            if (la.codepoint == '/')
            {
                state.lex_index += 2;
                while (state.lex_index < state.char_count &&
                    state.char_stream[state.lex_index].codepoint != NEWLINE)
                {
                    state.lex_index++;
                }
                continue;
            }

            // multi-line /% %/
            if (la.codepoint == '%')
            {
                state.lex_index += 2;
                while (state.lex_index + 1 < state.char_count)
                {
                    if (state.char_stream[state.lex_index].codepoint == '%' &&
                        state.char_stream[state.lex_index + 1].codepoint == '/')
                    {
                        state.lex_index += 2;
                        break;
                    }
                    state.lex_index++;
                }
                continue;
            }
        }

        if (current.codepoint == 0x0022)   // "
        {
            //printf("TRIGGAR DETTA NÅGONSIN EVER FÖR \" ");
            identifyString(&state);
            continue;
        }


        //Check for ID/Keywords
        if( (current.codepoint >= 0x0061 && current.codepoint <= 0x007A) ||
            (current.codepoint >= 0x0041 && current.codepoint <= 0x005A) ||
            (   
                current.codepoint == 0x00E5 ||
                current.codepoint == 0x00E4 ||
                current.codepoint == 0x00F6 ||
                current.codepoint == 0x00C5 ||
                current.codepoint == 0x00C4 ||
                current.codepoint == 0x00D6
            )
        ){
            identifyID(&state);    
            continue;    
        }

        // number
        if (current.codepoint >= 0x0030 && current.codepoint <= 0x0039)
        {
            identifyNum(&state);
            continue;
        }

        // delimiter
        if (
            current.codepoint == '(' || current.codepoint == ')' ||
            current.codepoint == '<' || current.codepoint == '>' ||
            current.codepoint == ';' || current.codepoint == ',' ||
            current.codepoint == ':'
        )
        {
            identifyDelim(&state);
            continue;
        }

        state.lex_index++;   // ALWAYS advance to avoid infinite loop
    }

    //Resolve tokens from lexemes
    state.token_capacity = 16;
    state.token_count = 0;
    state.tokens = malloc(sizeof(TokenType) * state.token_capacity);

    // Resolve tokens from lexemes
    // Resolve tokens from lexemes
    for (int i = 0; i < state.lexeme_count; i++)
    {
        char *lexeme = state.lexemes[i];
        char *lextwo = (i + 1 < state.lexeme_count)
                    ? state.lexemes[i + 1]
                    : NULL;

        TokenType tok = TOK_ERROR;


        // Delimiters (single lexeme)

        if (strcmp(lexeme, "(") == 0)          tok = TOK_LPAREN;
        else if (strcmp(lexeme, ")") == 0)     tok = TOK_RPAREN;
        else if (strcmp(lexeme, "<") == 0)     tok = TOK_LBLOCK;
        else if (strcmp(lexeme, ">") == 0)     tok = TOK_RBLOCK;
        else if (strcmp(lexeme, ";") == 0)     tok = TOK_SEMI;
        else if (strcmp(lexeme, ",") == 0)     tok = TOK_COMMA;
        else if (strcmp(lexeme, ":") == 0)     tok = TOK_ASSIGN;


        // Composite keywords (two lexemes)

        else if (lextwo != NULL)
        {
            tok = lookup_pair(lexeme, lextwo);
            if (tok != TOK_ERROR)
            {
                if (state.token_count >= state.token_capacity)
                {
                    state.token_capacity *= 2;
                    TokenType *tmp = realloc(
                        state.tokens,
                        sizeof(TokenType) * state.token_capacity
                    );
                    if (!tmp) break;
                    state.tokens = tmp;
                }

                state.tokens[state.token_count++] = tok;
                i++;            // consume BOTH lexemes
                continue;
            }
        }

        // Single keyword or identifier

        if (tok == TOK_ERROR)
        {
            tok = lookup(lexeme);
        }


        // Emit token

        if (state.token_count >= state.token_capacity)
        {
            state.token_capacity *= 2;
            TokenType *tmp = realloc(
                state.tokens,
                sizeof(TokenType) * state.token_capacity
            );
            if (!tmp) break;
            state.tokens = tmp;
        }

        state.tokens[state.token_count++] = tok;
    }

    //Bind output
    *out_tokens = state.tokens;
    *out_token_count = state.token_count;

    // Cleanup lexer-owned memory
    for (int i = 0; i < state.lexeme_count; i++)
    {
        free(state.lexemes[i]);
    }
    free(state.lexemes);
    free(state.tokens);

    return;
}







CharacterUnit getCurrent(LexState state){
    return state.char_stream[state.lex_index];
}

CharacterUnit getLookahead(LexState state){
    return state.char_stream[state.lex_index + 1];
}


void identifyID(LexState *state)
{
    char lexeme[MAXLEXSIZE];
    int lex_len = 0;

    while (state->lex_index < state->char_count)
    {
        CharacterUnit cu = state->char_stream[state->lex_index];
        int cp = cu.codepoint;

        int is_letter =
            (cp >= 0x0061 && cp <= 0x007A) ||
            (cp >= 0x0041 && cp <= 0x005A) ||
            cp == 0x00E5 || cp == 0x00E4 || cp == 0x00F6 ||
            cp == 0x00C5 || cp == 0x00C4 || cp == 0x00D6;

        int is_digit = (cp >= 0x0030 && cp <= 0x0039);
        int is_underscore = (cp == 0x005F);
        int is_dot = (cp == 0x002E);

        if (!is_letter && !is_digit && !is_underscore && !is_dot)
            break;

        for (int i = 0; i < cu.byte_length; i++)
        {
            if (lex_len >= MAXLEXSIZE - 1)
                break;
            lexeme[lex_len++] = cu.bytes[i];
        }

        state->lex_index++;
    }

    lexeme[lex_len] = '\0';

    // Store lexeme dynamically
    if (state->lexeme_count >= state->lexeme_capacity)
    {
        state->lexeme_capacity *= 2;
        char **tmp = realloc(
            state->lexemes,
            sizeof(char*) * state->lexeme_capacity
        );
        if (!tmp) {
            // handle allocation failure
            return;
        }
        state->lexemes = tmp;
    }

    char *stored = malloc(lex_len + 1);
    if (!stored) {
        // handle allocation failure
        return;
    }

    memcpy(stored, lexeme, lex_len + 1);
    state->lexemes[state->lexeme_count++] = stored;

    //printf("LEXEME STORED: %s\n", stored);
}

void identifyNum(LexState *state)
{
    char lexeme[MAXLEXSIZE];
    int lex_len = 0;
    int seen_dot = 0;

    while (state->lex_index < state->char_count)
    {
        CharacterUnit cu = state->char_stream[state->lex_index];
        int cp = cu.codepoint;

        int is_digit = (cp >= 0x0030 && cp <= 0x0039); // 0–9
        int is_dot   = (cp == 0x002E);                 // '.'

        if (is_dot)
        {
            if (seen_dot)
                break;          // second dot ends number
            seen_dot = 1;
        }
        else if (!is_digit)
        {
            break;
        }

        for (int i = 0; i < cu.byte_length; i++)
        {
            if (lex_len >= MAXLEXSIZE - 1)
                break;
            lexeme[lex_len++] = cu.bytes[i];
        }

        state->lex_index++;
    }

    lexeme[lex_len] = '\0';

    // store lexeme dynamically
    if (state->lexeme_count >= state->lexeme_capacity)
    {
        state->lexeme_capacity *= 2;
        char **tmp = realloc(
            state->lexemes,
            sizeof(char*) * state->lexeme_capacity
        );
        if (!tmp) return;
        state->lexemes = tmp;
    }

    char *stored = malloc(lex_len + 1);
    if (!stored) return;

    memcpy(stored, lexeme, lex_len + 1);
    state->lexemes[state->lexeme_count++] = stored;

    //printf("NUMBER STORED: %s\n", stored);
}

void identifyString(LexState *state)
{
    //printf("\nENTERING IDENTIFY STRING");
    char lexeme[MAXLEXSIZE];
    int lex_len = 0;

    // consume opening quote "
    state->lex_index++;

    while (state->lex_index < state->char_count)
    {
        CharacterUnit cu = state->char_stream[state->lex_index];
        int cp = cu.codepoint;

        // closing quote → end of string
        if (cp == 0x0022)   // "
        {
            state->lex_index++;   // consume closing quote
            break;
        }

        // forbid newline inside string (for now)
        if (cp == 0x000A || cp == 0x000D)
        {
            break; // could emit error later
        }

        // append UTF-8 bytes
        for (int i = 0; i < cu.byte_length; i++)
        {
            if (lex_len >= MAXLEXSIZE - 1)
                break;
            lexeme[lex_len++] = cu.bytes[i];
        }

        state->lex_index++;
    }

    lexeme[lex_len] = '\0';

    // store lexeme dynamically
    if (state->lexeme_count >= state->lexeme_capacity)
    {
        state->lexeme_capacity *= 2;
        char **tmp = realloc(
            state->lexemes,
            sizeof(char*) * state->lexeme_capacity
        );
        if (!tmp)
            return;
        state->lexemes = tmp;
    }


    char *stored = malloc(lex_len + 3);
    if(!stored){
        return;
    }
    stored[0] = '"';
    memcpy(stored + 1, lexeme, lex_len);
    stored[lex_len + 1] = '"';
    stored[lex_len + 2] = '\0';

    state->lexemes[state->lexeme_count++] = stored;

    //printf("STRING STORED: %s\n", stored);
}



void identifyDelim(LexState *state)
{
    CharacterUnit cu = state->char_stream[state->lex_index];

    char lexeme[5];   // enough for 1 UTF-8 char + '\0'
    int lex_len = 0;

    for (int i = 0; i < cu.byte_length; i++)
    {
        lexeme[lex_len++] = cu.bytes[i];
    }

    lexeme[lex_len] = '\0';

    // store lexeme dynamically
    if (state->lexeme_count >= state->lexeme_capacity)
    {
        state->lexeme_capacity *= 2;
        char **tmp = realloc(
            state->lexemes,
            sizeof(char*) * state->lexeme_capacity
        );
        if (!tmp) return;
        state->lexemes = tmp;
    }

    char *stored = malloc(lex_len + 1);
    if (!stored) return;

    memcpy(stored, lexeme, lex_len + 1);
    state->lexemes[state->lexeme_count++] = stored;

    //printf("DELIM STORED: %s\n", stored);

    state->lex_index++;   // delimiters consume exactly one CharacterUnit
}


