/*
lexer()
│
├─ init LexState
│
├─ lex_scan(state)
│   │
│   ├─ while not EOF
│   │   │
│   │   ├─ cu = peek()
│   │   │
│   │   ├─ if whitespace
│   │   │     advance()
│   │   │     continue
│   │   │
│   │   ├─ if comment start
│   │   │     identify_comment()
│   │   │     continue
│   │   │
│   │   ├─ identify_identifier()
│   │   │       advance()  
│   │   │   
│   │   ├─ identify_number()
│   │   │       advance()
│   │   │  
│   │   ├─ identify_string()
│   │   │       advance()
│   │   │  
│   │   ├─ identify_delimiter()
│   │   │       advance()
│   │   │  
│   │   │
│   │   └─ ensure_progress()
│   │
│   └─ end while
│
├─ lex_resolve(state)
│
└─ return tokens
*/

#include "tokenkeytab.h"
#include "lexer.h"
#include "helper.h"


#define MAXLEXSIZE          128
#define SPACE               0x0020
#define HORIZONTAL_TAB      0x0009
#define NEWLINE             0x000A
#define CARRIAGE_RETURN     0x000D
#define EOF_CODEPOINT       -1

void init_lex_state(LexState * state, int char_count, CharacterUnit * decode_buffer);

void lex_scan(LexState *state);

CharacterUnit peek(LexState *state);

CharacterUnit lookahead(LexState *state);

void lexer(
    CharacterUnit *decode_buffer,
    int char_count,
    TokenBuffer **out_tokens,   
    int *out_token_count
){
    LexState state = {0};
    printd("Init state");
    init_lex_state(&state, char_count, decode_buffer);      //Init struct for lexing
    printd("Entering lex_scan()");
    lex_scan(&state);                                       //While loop scan
    //init_lex_resolve(&state);                               //Re-init struct for token resolving
    //lex_resolve(&state);                                    //While loop resolver
}

/* Subroutines */

void init_lex_state(LexState * state, int char_count, CharacterUnit * decode_buffer){
    state -> lex_index = 0;
    state -> lexeme_count = 0;
    state -> char_count = char_count;
    state -> char_stream = decode_buffer;
    state -> lexeme_capacity = 16;
}


void lex_scan(LexState * state){

    printf("Total characters: %d\n", state -> char_count);

    state -> current = peek(state);
    state -> next = lookahead(state);

    while(state -> lex_index < state -> char_count)
    {
        printf("Current codepoint: %d\n", state -> current.codepoint);

        if (state -> current.codepoint == HORIZONTAL_TAB ||
            state -> current.codepoint == NEWLINE        ||
            state -> current.codepoint == SPACE          ||
            state -> current.codepoint == CARRIAGE_RETURN) 
        {
            printd("\nEntering whitespace\n");

            switch(state -> current.codepoint){
                case HORIZONTAL_TAB:
                    state -> col += 4;
                    printd("Found a tab");
                    advance(state);
                    break;

                case NEWLINE:
                    state -> col = 1;
                    state -> row ++;
                    printd("Found a newline");
                    advance(state);
                    break;

                case SPACE:
                    state -> col ++;
                    printd("Found a space");
                    advance(state);
                    break;

                case CARRIAGE_RETURN:
                    state -> col = 1;
                    state -> row ++;
                    printd("Found a carriage return");
                    advance(state);
                    break;

                default:
                    printd("Error reading whitespace character");
                    advance(state);
                    break;
            }

            continue;
        }
    
        else if (state -> current.codepoint == '/' &&
                (state -> next.codepoint == '/' ||
                 state -> next.codepoint == '%'))
        {
            printd("\nEntering comments\n");

            if(state -> next.codepoint == '/') 
                identify_single_comment(state);

            if(state -> next.codepoint == '%')
                identify_multi_comment(state);

            continue;
        } 
        else if(
            (state -> current.codepoint >= 'a' && state -> current.codepoint <= 'z') ||
            (state -> current.codepoint >= 'A' && state -> current.codepoint <= 'Z') ||
            (state -> current.codepoint == 0x00C5) ||
            (state -> current.codepoint == 0x00C4) ||
            (state -> current.codepoint == 0x00D6) ||
            (state -> current.codepoint == 0x00E5) ||
            (state -> current.codepoint == 0x00E4) ||
            (state -> current.codepoint == 0x00F6)
        ) {
            identify_ID(state);
            printf("current lex_index: %d\n", state -> lex_index);
        } else if(
            (state -> current.codepoint == '"')
        ) {
            identify_string(state);
        } else if(
            (state -> current.codepoint >= '0' && state -> current.codepoint <= '9')
        ) {
            identify_number(state);
        } else if(
            (state->current.codepoint != ';') ||
            (state->current.codepoint != ',') ||
            (state->current.codepoint != ':') ||
            (state->current.codepoint != '(') ||
            (state->current.codepoint != ')') ||
            (state->current.codepoint != '<') ||
            (state->current.codepoint != '>') ||
            (state->current.codepoint != '.') ||
            (state->current.codepoint != '+') ||
            (state->current.codepoint != '-') ||
            (state->current.codepoint != '*') ||
            (state->current.codepoint != '/') 
        ) {
            identify_delimiter(state);
        }
        else {
            printd("Resolving scan error!");
            advance(state);
        }
        printd("Loop iteration complete, starting next iteration!\n");
    }
    printd("Lex Scanning Complete!");
}


CharacterUnit peek(LexState * state) {
    return state -> char_stream[state -> lex_index];
}

CharacterUnit lookahead(LexState * state) {
    return state -> char_stream[state -> lex_index + 1];
}

void advance(LexState * state)
{
    printd("Entering advance");
    state->lex_index++;

    if (state->lex_index < state->char_count)
    {
        state->current = peek(state);
        if (state->lex_index + 1 < state->char_count)
        {
            state->next = lookahead(state);
        }
        else
        {
            state->next.codepoint = EOF_CODEPOINT;   
        }
    }
    else
    {
        state->current.codepoint = EOF_CODEPOINT;   
        state->next.codepoint    = EOF_CODEPOINT;
    }
}

void identify_single_comment(LexState * state){
    //consume character units from lexState index until newline is eaten, increment lex_state each time
    printd("Entering single comments");
    while(state -> lex_index < state -> char_count)
    {
        if( state -> current.codepoint != NEWLINE && 
            state -> next.codepoint != CARRIAGE_RETURN)
        {
            state -> col++;
            advance(state);  

        } else {
            state -> col = 1;
            state -> row++;
            printf("Finished single line comment, current lex_index: %d\n", state -> lex_index);
            advance(state);
            break;
        }
    }
}

void identify_multi_comment(LexState * state){
    while(state -> lex_index < state -> char_count){
        if( !(state -> current.codepoint == '%' && 
              state -> next.codepoint == '/'))
        {
            if( state -> current.codepoint != NEWLINE && 
                state -> current.codepoint != CARRIAGE_RETURN)
            {
                state -> col++;
                advance(state);  
            } else {
                state -> col = 1;
                state -> row++;
                advance(state);
                continue;
            }
        } else {
            state -> col += 2;
            printf("Finished multi line comment, current lex_index: %d\n", state -> lex_index);
            advance(state);
            advance(state);
            break;
        }
    }
}

void identify_ID(LexState *state){
    printd("\nEntering identify ID\n");

    char buf[MAXLEXSIZE];
    int len = 0;

    int first_handled = 0;



    while (
        state->lex_index < state->char_count &&
        state->current.codepoint != ';' &&
        state->current.codepoint != ',' &&
        state->current.codepoint != ':' &&
        state->current.codepoint != '(' &&
        state->current.codepoint != ')' &&
        state->current.codepoint != '<' &&
        state->current.codepoint != '>' &&
        state->current.codepoint != '.' &&
        state->current.codepoint != '+' &&
        state->current.codepoint != '-' &&
        state->current.codepoint != '*' &&
        state->current.codepoint != '/' &&
        state->current.codepoint != '"' &&
        state->current.codepoint != NEWLINE &&
        state->current.codepoint != SPACE &&
        state->current.codepoint != HORIZONTAL_TAB &&
        state->current.codepoint != CARRIAGE_RETURN
    )
    {
        if(first_handled == 0){
            //printd("ID first character handling");

            if(
                (state->current.codepoint >= 'a' && state->current.codepoint <= 'z') ||
                (state->current.codepoint >= 'A' && state->current.codepoint <= 'Z') ||
                (state->current.codepoint == 0x00C5) ||
                (state->current.codepoint == 0x00C4) ||
                (state->current.codepoint == 0x00D6) ||
                (state->current.codepoint == 0x00E5) ||
                (state->current.codepoint == 0x00E4) ||
                (state->current.codepoint == 0x00F6)
            )
            {
                //printd("ID appending first character");
                append_character(buf, &len, state->current);
                //printf("ID buffer len after append: %d\n", len);
                first_handled = 1;
            }
            else
            {
                //printd("ID first character invalid, breaking");
                break;
            }
        }
        else
        {
            //printd("ID subsequent character handling");

            if(
                (state->current.codepoint >= 'a' && state->current.codepoint <= 'z') ||
                (state->current.codepoint >= 'A' && state->current.codepoint <= 'Z') ||
                (state->current.codepoint >= '0' && state->current.codepoint <= '9') ||
                (state->current.codepoint == '_') ||
                (state->current.codepoint == 0x00C5) ||
                (state->current.codepoint == 0x00C4) ||
                (state->current.codepoint == 0x00D6) ||
                (state->current.codepoint == 0x00E5) ||
                (state->current.codepoint == 0x00E4) ||
                (state->current.codepoint == 0x00F6)
            )
            {
                //printd("ID appending subsequent character");
                append_character(buf, &len, state->current);
                //printf("ID buffer len after append: %d\n", len);
            }
            else
            {
                //printd("ID character invalid for identifier, breaking");
                break;
            }
        }

        advance(state);


    }

    //printd("ID loop exited");

    buf[len] = '\0';

    //printf("ID FINAL BUFFER -> \"%s\" (len=%d)\n", buf, len);


    append_lexeme(state, buf);

    printd("ID append_lexeme complete");
}

void identify_string(LexState * state){
    printd("Entering identify string");

    char buf[MAXLEXSIZE];
    int len = 0;

    // current is assumed to be opening quote
    append_character(buf, &len, state->current);
    advance(state);

    while (1)
    {
        // EOF before closing quote
        if (state->lex_index >= state->char_count)
        {
            printd("Error: unterminated string at EOF");
            break;
        }

        // Newline before closing quote (illegal by spec)
        if (state->current.codepoint == NEWLINE ||
            state->current.codepoint == CARRIAGE_RETURN)
        {
            printd("Error: unterminated string before newline");
            break;
        }

        // Closing quote ends the string
        if (state->current.codepoint == '"')
        {
            append_character(buf, &len, state->current);
            advance(state);
            break;
        }

        // Normal string content
        append_character(buf, &len, state->current);
        advance(state);
    }

    append_lexeme(state, buf);
}

void identify_number(LexState *state)
{
    printd("Entering identify number");

    char buf[MAXLEXSIZE];
    int len = 0;

    printf(
        "NUMBER START -> lex_index=%d codepoint=%d '%c'\n",
        state->lex_index,
        state->current.codepoint,
        (char)state->current.codepoint
    );

    // HARD GUARD: number must start with digit
    if (!(state->current.codepoint >= '0' && state->current.codepoint <= '9'))
    {
        printd("NUMBER ERROR -> first character is not a digit, aborting");
        return;
    }

    while (state->lex_index < state->char_count)
    {
        printf(
            "NUMBER LOOP -> lex_index=%d codepoint=%d '%c' len=%d\n",
            state->lex_index,
            state->current.codepoint,
            (char)state->current.codepoint,
            len
        );

        // digits are always accepted
        if (state->current.codepoint >= '0' &&
            state->current.codepoint <= '9')
        {
            append_character(buf, &len, state->current);
            printd("NUMBER appended digit");
            advance(state);
            continue;
        }

        // decimal point — allowed ANY number of times
        if (state->current.codepoint == '.')
        {
            append_character(buf, &len, state->current);
            printd("NUMBER appended decimal point");
            advance(state);
            continue;
        }

        // anything else ends the number
        printd("NUMBER terminating on non-numeric character");
        break;
    }

    buf[len] = '\0';

    printf("NUMBER FINAL BUFFER -> \"%s\"\n", buf);
    printf(
        "NUMBER END -> lex_index=%d current_codepoint=%d\n",
        state->lex_index,
        state->current.codepoint
    );

    append_lexeme(state, buf);
}

void identify_delimiter(LexState * state)
{
    printd("Entering identify delimiter");

    char buf[MAXLEXSIZE];
    int len = 0;

    printf(
        "DELIMITER START -> lex_index=%d codepoint=%d '%c'\n",
        state->lex_index,
        state->current.codepoint,
        (state->current.codepoint >= 32 && state->current.codepoint < 127)
            ? (char)state->current.codepoint
            : '?'
    );

    printd("DELIMITER appending character");
    append_character(buf, &len, state->current);

    printd("DELIMITER advancing");
    advance(state);

    buf[len] = '\0';

    printf("DELIMITER FINAL BUFFER -> \"%s\"\n", buf);
    printf("DELIMITER END -> lex_index=%d current_codepoint=%d\n",
           state->lex_index,
           state->current.codepoint);

    append_lexeme(state, buf);
}

void append_character(char *buf, int *len, CharacterUnit cu)
{
    for (int i = 0; i < cu.byte_length; i++)
    {
        if (*len >= MAXLEXSIZE - 1)
            return;

        buf[(*len)++] = cu.bytes[i];
    }
}

void append_lexeme(LexState * state, char * buf)
{
    printd("Entering append_lexeme");


    /* FIX: allocate if storage does not exist */
    if (state->lexemes == NULL ||
        state->lexeme_row == NULL ||
        state->lexeme_col == NULL)
    {
        //printd("append_lexeme -> storage uninitialized, allocating");

        size_t new_cap = (state->lexeme_capacity == 0) ? 8 : state->lexeme_capacity;

        state->lexemes    = malloc(new_cap * sizeof(char *));
        state->lexeme_row = malloc(new_cap * sizeof(int));
        state->lexeme_col = malloc(new_cap * sizeof(int));

        if (!state->lexemes || !state->lexeme_row || !state->lexeme_col)
        {
            fprintf(stderr, "Fatal error: failed to allocate lexeme storage\n");
            exit(1);
        }

        state->lexeme_capacity = new_cap;
    }
    /* grow if needed */
    else if (state->lexeme_count >= state->lexeme_capacity)
    {
        //printd("append_lexeme -> growing storage");

        size_t new_cap = state->lexeme_capacity * 2;
        //printf("append_lexeme -> new_cap=%zu\n", new_cap);

        char **new_lexemes = realloc(state->lexemes, new_cap * sizeof(char *));
        int  *new_rows     = realloc(state->lexeme_row, new_cap * sizeof(int));
        int  *new_cols     = realloc(state->lexeme_col, new_cap * sizeof(int));

        if (!new_lexemes || !new_rows || !new_cols)
        {
            fprintf(stderr, "Fatal error: failed to reallocate lexeme storage\n");
            exit(1);
        }

        state->lexemes        = new_lexemes;
        state->lexeme_row     = new_rows;
        state->lexeme_col     = new_cols;
        state->lexeme_capacity = new_cap;
    }
    else
    {
        //printd("append_lexeme -> no resize needed");
    }

    //printd("append_lexeme -> duplicating lexeme");
    //printf("append_lexeme -> buf=\"%s\"\n", buf);

    size_t len = strlen(buf);
    //printf("append_lexeme -> strlen(buf)=%zu\n", len);

    char *copy = malloc(len + 1);
    //printf("append_lexeme -> malloc copy=%p\n", (void*)copy);

    if (!copy)
    {
        fprintf(stderr, "Fatal error: failed to allocate lexeme copy\n");
        exit(1);
    }

    memcpy(copy, buf, len + 1);

    //printd("append_lexeme -> storing lexeme pointer");
    //printf("append_lexeme -> storing at index %d\n", state->lexeme_count);

    state->lexemes[state->lexeme_count] = copy;
    state->lexeme_row[state->lexeme_count] = state->row;
    state->lexeme_col[state->lexeme_count] = state->col;

    state->lexeme_count++;

    //printd("append_lexeme -> completed successfully");
}





