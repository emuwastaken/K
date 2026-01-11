
#include <string.h>
#include <stdlib.h>
#include "utf_decoder.h"
#include "tokenkeytab.h"

typedef enum LexemeKind {
    LEX_KEYWORD,
    LEX_ID,
    LEX_INT,
    LEX_FLOAT,
    LEX_STRING,
    LEX_DELIM
} LexemeKind;

typedef struct TokenBuffer {
    TokenType   token;     // final resolved token (TOK_HEL, TOK_ID, etc)
    LexemeKind  kind;      // lexical category (ID, NUMBER, STRING, KEYWORD, DELIM)
    int         row;       // 1-based line number
    int         col;       // 1-based column number (start of token)
    char       *lexeme;    // exact source text (owned by token)
} TokenBuffer;

typedef struct LexState {

    char ** lexemes;                 //Stores all lexemes
    char * current_lexeme;
    int current_lexeme_len;

    int  * lexeme_row;               //Stores the row    of all stored lexemes
    int  * lexeme_col;               //Stores the column of all stored lexemes

    int lexeme_count;               //Amount of lexemes
    int lexeme_capacity;            //Amount of lexemes allowed before dynamic reallocation

    TokenBuffer *tokens;            //Stores all resolved lexemes as tokens
    int token_count;                //Amount of tokens (!= lexeme_count, double keywords are 1 token but two lexemes)
    int token_capacity;             //Amount of tokens allowed before dynamic reallocation
    int * token_row;
    int * token_col;

    CharacterUnit *char_stream;     //List of all characters with codepoint, byte length and individual bytes stored
    int char_count;                 //Amount of characters
    int lex_index;                  //Current index of lexer, used by token resolver

    CharacterUnit current;          //Current character
    CharacterUnit next;             //Next character

    int row;                   //Current row                    
    int col;                   //Current column

} LexState;

void lexer(
    CharacterUnit *decode_buffer,
    int char_count,

    TokenBuffer **out_tokens,
    int *out_token_count,

    char ***out_lexemes,
    int *out_lexeme_count,
    int **out_lexeme_cols,
    int **out_lexeme_rows
);

void init_lex_state(LexState * state, int char_count, CharacterUnit * decode_buffer);

void lex_scan(LexState *state);

CharacterUnit peek(LexState *state);
CharacterUnit lookahead(LexState *state);
void advance(LexState * state);

void identify_single_comment(LexState * state);
void identify_multi_comment(LexState * state);

void lex_identify(LexState *state);
void append_lexeme(LexState * state, char * buf);

void identify_ID(LexState *state);
void identify_number(LexState *state);
void identify_string(LexState *state);
void identify_delimiter(LexState *state);

void ensure_progress(LexState *state);

void append_character(char *buf, int *len, CharacterUnit cu);
void init_lex_resolve(LexState *state);
void lex_resolve(LexState *state);





