#include "parser.h"
#include "lexer.h"
#include "tokenkeytab.h"
#include <stdio.h>

/* ---------------------------------------------------
   FOLLOW sets for panic-mode recovery
--------------------------------------------------- */

/* ---------------------------------------------------
   FOLLOW sets (panic-mode sync sets)
--------------------------------------------------- */

/* program ::= global_statement_list EOF */
static const TokenType FOLLOW_program[] = {
    TOK_EOF,
    TOK_ERROR
};

/* global_statement ::= function | declaration | typedef */
static const TokenType FOLLOW_global_statement[] = {
    TOK_HEL,
    TOK_FLYT,
    TOK_BOK,
    TOK_BIT,
    TOK_HALV,
    TOK_BYTE,
    TOK_ORD,
    TOK_VAL,
    TOK_STRUKTUR,
    TOK_TYPDEF,
    TOK_IDENTIFIER,
    TOK_EOF,
    TOK_ERROR
};

/* statement ::= ... */
static const TokenType FOLLOW_statement[] = {
    TOK_SEMI,
    TOK_RBLOCK,
    TOK_ANNARS,
    TOK_EOF,
    TOK_ERROR
};

/* expression ::= ... */
static const TokenType FOLLOW_expression[] = {
    TOK_SEMI,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_RBLOCK,
    TOK_EOF,
    TOK_ERROR
};

static const TokenType FOLLOW_statement_list[] = {
    TOK_RBLOCK,
    TOK_ANNARS,
    TOK_EOF,
    TOK_ERROR
};


static const TokenType FOLLOW_parameter_list[] = {
    TOK_RPAREN,
    TOK_ERROR
};


static const TokenType FOLLOW_parameter[] = {
    TOK_COMMA,
    TOK_RPAREN,
    TOK_ERROR
};


static const TokenType FOLLOW_block[] = {
    TOK_RBLOCK,
    TOK_EOF,
    TOK_ERROR
};

static const TokenType FOLLOW_type_specifier[] = {
    TOK_ASSIGN,     /* ':' */
    TOK_IDENTIFIER,
    TOK_PEK,
    TOK_COMMA,
    TOK_SEMI,
    TOK_RPAREN,
    TOK_ERROR
};


static const TokenType FOLLOW_assignment_core[] = {
    TOK_SEMI,
    TOK_RPAREN,
    TOK_ERROR
};


//////////////////////////////////////////////////////

void parser(TokenBuffer *token_stream, 
            int token_count, 
            char **lexeme_stream,
            int * out_error_count
)
{

    ParState state = {0};

    init_parser(&state, token_stream, token_count);
    program(&state);
    *out_error_count = state.error_count;
    return;

}

// ---------------------------------------------------
// Initializes parser state and primes token stream
// Pre: token_stream valid, token_count > 0
// Post: parser ready to enter program()
// ---------------------------------------------------
// ---------------------------------------------------
// Initializes parser state and primes lookahead
// Pre: token_stream valid
// Post: next and next_next populated
// ---------------------------------------------------
void init_parser(ParState *state,
                 TokenBuffer *token_stream,
                 int token_count)
{
    state->tokens      = token_stream;
    state->token_count = token_count;
    state->index = 0;
    state->current   = TOK_ERROR;
    state->next      = (token_count > 0) ? token_stream[0].token : TOK_EOF;
    state->next_next = (token_count > 1) ? token_stream[1].token : TOK_EOF;
    state->error_count = 0;
    state->panic_mode  = 0;
    state->sync_set = FOLLOW_program;
}

// Advances token stream by one
void next_token(ParState *state)
{
    state->current = state->next;

    state->index++;

    if (state -> index < state -> token_count)
        state -> next = state -> tokens[state -> index].token;
    else
        state->next = TOK_EOF;

    if (state -> index + 1 < state -> token_count)
        state -> next_next = state -> tokens[state -> index + 1].token;
    else
        state->next_next = TOK_EOF;
}

// peek_tokens n tokens ahead without consuming


TokenType peek_token(ParState *state, int offset)
{
    int idx = state->index + offset;

    if (idx >= state->token_count)
        return TOK_EOF;

    return state->tokens[idx].token;
}



void match(ParState *state, TokenType expected)
{
    if (state->next == expected)
    {
        next_token(state);
        return;
    }

    /* missing symbol */
    state -> error_count++;

    printf("Error: missing %s before %s\n",
           tok2name(expected),
           tok2name(state -> next));
}


// Skips tokens until one in the current sync_set
void sync_to_follow(ParState *state)
{
    while (state->next != TOK_EOF)
    {
        for (int i = 0; state->sync_set[i] != TOK_ERROR; i++)
        {
            if (state->next == state->sync_set[i])
                return;
        }

        next_token(state);
    }
}


//Program structure
void program(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] program\n");

    /* enter non-terminal */
    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_program;

    /* FIRST(program) */
    switch (state -> next)
    {
        default:
            global_statement_list(state);
            break;
    }

    /* exit non-terminal */
    state -> sync_set = saved_sync;

    printf("[EXIT] program\n");
}

void global_statement_list(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] global_statement_list\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_program;

    while (state->next != TOK_EOF)
        global_statement(state);

    state->sync_set = saved_sync;

    printf("[EXIT ] global_statement_list\n");
}


void global_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] global_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_global_statement;

    /* FIRST(global_statement) */
    switch (state->next)
    {
        case TOK_TYPDEF:
        case TOK_STRUKTUR:
            type_declaration(state);
            break;

        case TOK_HEL:
        case TOK_FLYT:
        case TOK_BOK:
        case TOK_BIT:
        case TOK_HALV:
        case TOK_BYTE:
        case TOK_ORD:
        case TOK_VAL:
        case TOK_IDENTIFIER:
            /* LL(2) disambiguation */
            if (state->next_next == TOK_LPAREN)
                function_declaration(state);
            else
                declaration_statement(state);
            break;

        default:
            /* extra symbol */
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error: unexpected token %s in global scope\n",
                   tok2name(state->next));

            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] global_statement\n");
}




//Declarations
void function_declaration(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] function_declaration\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_global_statement;

    /* return type */
    type_specifier(state);

    /* function name */
    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected function name, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
        state->sync_set = saved_sync;
        printf("[EXIT ] function_declaration\n");
        return;
    }

    /* '(' */
    match(state, TOK_LPAREN);

    /* parameter list (may be empty) */
    parameter_list(state);

    /* ')' */
    match(state, TOK_RPAREN);

    /* function body */
    block(state);

    state->sync_set = saved_sync;

    printf("[EXIT ] function_declaration\n");
}


void declaration_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] declaration_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    /* type */
    type_specifier(state);

    /* ':' */
    match(state, TOK_ASSIGN);

    /* first decl_item */
    if (state->next == TOK_IDENTIFIER)
    {
        /* ID */
        match(state, TOK_IDENTIFIER);

        /* optional initializer */
        if (state->next == TOK_ASSIGN)
        {
            match(state, TOK_ASSIGN);
            initializer(state);
        }
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected identifier in declaration\n");
        sync_to_follow(state);

        state->sync_set = saved_sync;
        printf("[EXIT ] declaration_statement\n");
        return;
    }

    /* additional decl_items */
    while (state->next == TOK_COMMA)
    {
        match(state, TOK_COMMA);

        if (state->next == TOK_IDENTIFIER)
        {
            match(state, TOK_IDENTIFIER);

            if (state->next == TOK_ASSIGN)
            {
                match(state, TOK_ASSIGN);
                initializer(state);
            }
        }
        else
        {
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error: expected identifier after ',' in declaration\n");
            sync_to_follow(state);
            break;
        }
    }

    /* ';' */
    match(state, TOK_SEMI);

    state->sync_set = saved_sync;

    printf("[EXIT ] declaration_statement\n");
}


// ---------------------------------------------------
// Parses a typedef declaration
// Pre: TOK_TYPDEF
// Post: typedef declaration consumed
// ---------------------------------------------------
void typedef_declaration(ParState *state)
{
    printf("[ENTER] typedef_declaration\n");

    match(state, TOK_TYPDEF);

    if (state->next == TOK_STRUKTUR)
    {
        match(state, TOK_STRUKTUR);

        if (state->next == TOK_IDENTIFIER)
            match(state, TOK_IDENTIFIER);
        else
        {
            state->error_count++;
            printf("Syntax error: expected struct name in typedef\n");
            sync_to_follow(state);
            printf("[EXIT ] typedef_declaration\n");
            return;
        }

        match(state, TOK_LBLOCK);

        /* struct_field_list */
        while (state->next != TOK_RBLOCK && state->next != TOK_EOF)
        {
            type_specifier(state);
            match(state, TOK_ASSIGN);

            if (state->next == TOK_IDENTIFIER)
                match(state, TOK_IDENTIFIER);
            else
            {
                state->error_count++;
                printf("Syntax error: expected field name in struct\n");
                sync_to_follow(state);
                break;
            }

            /* optional array suffix */
            if (state->next == TOK_LBLOCK)
            {
                match(state, TOK_LBLOCK);
                expression(state);
                match(state, TOK_RBLOCK);
            }

            match(state, TOK_SEMI);
        }

        match(state, TOK_RBLOCK);
    }
    else
    {
        /* TYPDEF <type> ID ; */
        type_specifier(state);

        if (state->next == TOK_IDENTIFIER)
            match(state, TOK_IDENTIFIER);
        else
        {
            state->error_count++;
            printf("Syntax error: expected typedef name\n");
            sync_to_follow(state);
        }

        match(state, TOK_SEMI);
    }

    printf("[EXIT ] typedef_declaration\n");
}

void struct_declaration(ParState *state)
{
    printf("[ENTER] struct_declaration\n");

    match(state, TOK_STRUKTUR);

    if (state->next == TOK_IDENTIFIER)
        match(state, TOK_IDENTIFIER);
    else
    {
        state->error_count++;
        printf("Syntax error: expected struct name\n");
        sync_to_follow(state);
        printf("[EXIT ] struct_declaration\n");
        return;
    }

    match(state, TOK_LBLOCK);

    while (state->next != TOK_RBLOCK && state->next != TOK_EOF)
    {
        type_specifier(state);
        match(state, TOK_ASSIGN);

        if (state->next == TOK_IDENTIFIER)
            match(state, TOK_IDENTIFIER);
        else
        {
            state->error_count++;
            printf("Syntax error: expected field name\n");
            sync_to_follow(state);
            break;
        }

        if (state->next == TOK_LBLOCK)
        {
            match(state, TOK_LBLOCK);
            expression(state);
            match(state, TOK_RBLOCK);
        }

        match(state, TOK_SEMI);
    }

    match(state, TOK_RBLOCK);

    printf("[EXIT ] struct_declaration\n");
}

// Parses an initializer
void initializer(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] initializer\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    /* aggregate literal starts with '<' */
    if (state->next == TOK_LBLOCK)
    {
        /* '<' */
        match(state, TOK_LBLOCK);

        /* optional aggregate_value_list */
        if (state->next != TOK_RBLOCK)
        {
            expression(state);

            while (state->next == TOK_COMMA)
            {
                match(state, TOK_COMMA);
                expression(state);
            }
        }

        /* '>' */
        match(state, TOK_RBLOCK);
    }
    else
    {
        /* otherwise, normal expression */
        expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] initializer\n");
}


void type_declaration(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] type_declaration\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_global_statement;

    switch (state->next)
    {
        case TOK_TYPDEF:
            typedef_declaration(state);
            break;

        case TOK_STRUKTUR:
            struct_declaration(state);
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error: unexpected token %s in type declaration\n",
                   tok2name(state->next));

            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] type_declaration\n");
}


void type_specifier(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] type_specifier\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_type_specifier;

    switch (state->next)
    {
        case TOK_HEL:
        case TOK_FLYT:
        case TOK_BOK:
        case TOK_BIT:
        case TOK_HALV:
        case TOK_BYTE:
        case TOK_ORD:
        case TOK_IDENTIFIER:
            next_token(state);
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error: expected type, got %s\n",
                   tok2name(state->next));

            sync_to_follow(state);
            state->sync_set = saved_sync;
            printf("[EXIT ] type_specifier\n");
            return;
    }

    while (state->next == TOK_PEK)
    {
        next_token(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] type_specifier\n");
}



void parameter_list(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] parameter_list\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_parameter_list;

    if (state->next != TOK_RPAREN)
    {
        parameter(state);

        while (state->next == TOK_COMMA)
        {
            match(state, TOK_COMMA);
            parameter(state);
        }
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] parameter_list\n");
}



void parameter(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] parameter\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_parameter;

    type_specifier(state);

    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected parameter name, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] parameter\n");
}




//Blocks and statements

void block(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] block\n");

    // save outer sync set
    saved_sync = state->sync_set;

    // block-specific recovery
    state->sync_set = FOLLOW_block;

    // opening delimiter
    match(state, TOK_LBLOCK);

    // zero or more statements
    statement_list(state);

    // closing delimiter
    match(state, TOK_RBLOCK);

    // restore outer sync set
    state->sync_set = saved_sync;

    printf("[EXIT ] block\n");
}


void statement_list(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] statement_list\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement_list;

    while (state->next != TOK_RBLOCK &&
           state->next != TOK_ANNARS &&
           state->next != TOK_EOF)
    {
        statement(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] statement_list\n");
}



void statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    switch (state->next)
    {
        /* --------------------
           Declarations
        -------------------- */
        case TOK_HEL:
        case TOK_FLYT:
        case TOK_BOK:
        case TOK_BIT:
        case TOK_HALV:
        case TOK_BYTE:
        case TOK_ORD:
        case TOK_KONSTANT:
        case TOK_STATISK:
            declaration_statement(state);
            break;

        /* --------------------
           Control / flow
        -------------------- */
        case TOK_ATERVAND:
            return_statement(state);
            break;

        case TOK_BRYT:
            break_statement(state);
            break;

        case TOK_OM:
            if_statement(state);
            break;

        case TOK_VAXEL:
            switch_statement(state);
            break;

        case TOK_FOR:
        case TOK_MEDAN:
        case TOK_GOR:
            loop_statement(state);
            break;

        /* --------------------
           Field-based statements
        -------------------- */
        case TOK_FALT:
            field_statement(state);
            break;

        /* --------------------
           ID-based (LL(2))
        -------------------- */
        case TOK_IDENTIFIER:
            if (state->next_next == TOK_ASSIGN ||
                state->next_next == TOK_OKAR ||
                state->next_next == TOK_MINSKAR)
            {
                assignment_statement(state);
            }
            else
            {
                expression_statement(state);
            }
            break;

        //Expression starters
        case TOK_STRING_LIT:
        case TOK_INT_LIT:
        case TOK_FLOAT_LIT:
        case TOK_LPAREN:
        case TOK_INTE:
        case TOK_ADDRESS:
        case TOK_DEREF:
            expression_statement(state);
            break;

        //Error
        default:
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error: unexpected token %s in statement\n",
                   tok2name(state->next));

            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] statement\n");
}


void break_statement(ParState *state)
{
    printf("[ENTER] break_statement\n");

    match(state, TOK_BRYT);
    match(state, TOK_SEMI);

    printf("[EXIT ] break_statement\n");
}

void lvalue(ParState *state)
{
    printf("[ENTER] lvalue\n");

    if (state->next == TOK_FALT)
    {
        field_access(state);
    }
    else if (state->next == TOK_IDENTIFIER)
    {
        if (state->next_next == TOK_LBLOCK)
            array_access(state);
        else
            match(state, TOK_IDENTIFIER);
    }
    else
    {
        state->error_count++;
        printf("Syntax error: expected lvalue, got %s\n",
               tok2name(state->next));
        sync_to_follow(state);
    }

    printf("[EXIT ] lvalue\n");
}


void array_access(ParState *state)
{
    printf("[ENTER] array_access\n");

    match(state, TOK_IDENTIFIER);
    match(state, TOK_LBLOCK);
    expression(state);
    match(state, TOK_RBLOCK);

    printf("[EXIT ] array_access\n");
}

void field_access(ParState *state)
{
    printf("[ENTER] field_access\n");

    match(state, TOK_FALT);

    if (state->next != TOK_IDENTIFIER)
    {
        state->error_count++;
        printf("Syntax error: expected identifier after FÄLT\n");
        sync_to_follow(state);
        printf("[EXIT ] field_access\n");
        return;
    }

    match(state, TOK_IDENTIFIER);

    /* one or more field segments */
    while (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);

        /* optional array suffix */
        if (state->next == TOK_LBLOCK)
        {
            match(state, TOK_LBLOCK);
            expression(state);
            match(state, TOK_RBLOCK);
        }
    }

    printf("[EXIT ] field_access\n");
}


//Statement Variants

void assignment_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] assignment_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    assignment_core(state);
    match(state, TOK_SEMI);

    state->sync_set = saved_sync;

    printf("[EXIT ] assignment_statement\n");
}


void return_statement(ParState *state)
{
    printf("[ENTER] return_statement\n");
    /* stub */
    printf("[EXIT ] return_statement\n");
}


void expression_statement(ParState *state)
{
    printf("[ENTER] expression_statement\n");
    /* stub */
    printf("[EXIT ] expression_statement\n");
}


void field_statement(ParState *state)
{
    printf("[ENTER] field_statement\n");
    /* stub */
    printf("[EXIT ] field_statement\n");
}


void if_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] if_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    /* FIRST(if_statement) */
    if (state->next == TOK_OM)
    {
        match(state, TOK_OM);
        match(state, TOK_LPAREN);
        expression(state);
        match(state, TOK_RPAREN);
        block(state);

        /* optional else_clause */
        if (state->next == TOK_ANNARS)
        {
            match(state, TOK_ANNARS);
            block(state);
        }
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected OM, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] if_statement\n");
}



void switch_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] switch_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    if (state->next == TOK_VAXEL)
    {
        match(state, TOK_VAXEL);
        match(state, TOK_LPAREN);
        expression(state);
        match(state, TOK_RPAREN);
        match(state, TOK_LBLOCK);

        /* zero or more case clauses */
        while (state->next == TOK_FALL)
        {
            match(state, TOK_FALL);
            expression(state);
            match(state, TOK_ASSIGN);
            statement_list(state);
        }

        /* optional default clause */
        if (state->next == TOK_ANNARS)
        {
            match(state, TOK_ANNARS);
            match(state, TOK_ASSIGN);
            statement_list(state);
        }

        match(state, TOK_RBLOCK);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected VÄXEL, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] switch_statement\n");
}



void loop_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] loop_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    /* FIRST(loop_statement) */
    switch (state->next)
    {
        case TOK_FOR:
            for_statement(state);
            break;

        case TOK_MEDAN:
            while_statement(state);
            break;

        case TOK_GOR:
            do_while_statement(state);
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error: unexpected token %s in loop statement\n",
                   tok2name(state->next));

            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] loop_statement\n");
}



void while_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] while_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    if (state->next == TOK_MEDAN)
    {
        match(state, TOK_MEDAN);
        match(state, TOK_LPAREN);
        expression(state);
        match(state, TOK_RPAREN);
        block(state);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected MEDAN, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] while_statement\n");
}



void do_while_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] do_while_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    if (state->next == TOK_GOR)
    {
        match(state, TOK_GOR);
        block(state);
        match(state, TOK_MEDAN);
        match(state, TOK_LPAREN);
        expression(state);
        match(state, TOK_RPAREN);
        match(state, TOK_SEMI);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected GÖR, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] do_while_statement\n");
}



void for_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] for_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    if (state->next == TOK_FOR)
    {
        match(state, TOK_FOR);
        match(state, TOK_LPAREN);

        /* for_init */
        if (state->next != TOK_SEMI)
        {
            if (state->next == TOK_HEL ||
                state->next == TOK_FLYT ||
                state->next == TOK_BOK ||
                state->next == TOK_BIT ||
                state->next == TOK_HALV ||
                state->next == TOK_BYTE ||
                state->next == TOK_ORD ||
                state->next == TOK_STATISK)
            {
                /* declaration-style init */
                declaration_statement(state);
            }
            else
            {
                /* assignment-style init */
                assignment_core(state);
            }
        }

        match(state, TOK_SEMI);

        /* condition (mandatory) */
        expression(state);
        match(state, TOK_SEMI);

        /* for_update (optional) */
        if (state->next != TOK_RPAREN)
        {
            assignment_core(state);
        }

        match(state, TOK_RPAREN);
        block(state);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error: expected FÖR, got %s\n",
               tok2name(state->next));

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] for_statement\n");
}

void assignment_core(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] assignment_core\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_assignment_core;

    lvalue(state);

    switch (state->next)
    {
        case TOK_ASSIGN:
            match(state, TOK_ASSIGN);
            expression(state);
            break;

        case TOK_OKAR:
            match(state, TOK_OKAR);
            break;

        case TOK_MINSKAR:
            match(state, TOK_MINSKAR);
            break;

        case TOK_PLUS_ASSIGN:
            match(state, TOK_PLUS_ASSIGN);
            expression(state);
            break;

        case TOK_MINUS_ASSIGN:
            match(state, TOK_MINUS_ASSIGN);
            expression(state);
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf(
                "Syntax error: expected assignment operator after lvalue, got %s\n",
                tok2name(state->next)
            );

            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] assignment_core\n");
}



//Expressions
void expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    logical_expression(state);

    state->sync_set = saved_sync;

    printf("[EXIT ] expression\n");
}




void logical_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] logical_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    relational_expression(state);

    while (state->next == TOK_EQ   ||
           state->next == TOK_NEQ  ||
           state->next == TOK_OCH  ||
           state->next == TOK_ELLER)
    {
        next_token(state);
        relational_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] logical_expression\n");
}



void relational_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] relational_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    additive_expression(state);

    while (state->next == TOK_LT  ||
           state->next == TOK_GT  ||
           state->next == TOK_LTE ||
           state->next == TOK_GTE)
    {
        next_token(state);
        additive_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] relational_expression\n");
}


void additive_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] additive_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    multiplicative_expression(state);

    while (state->next == TOK_PLUS ||
           state->next == TOK_MINUS)
    {
        next_token(state);
        multiplicative_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] additive_expression\n");
}



void multiplicative_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] multiplicative_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    unary_expression(state);

    while (state->next == TOK_MUL ||
           state->next == TOK_DIV)
    {
        next_token(state);
        unary_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] multiplicative_expression\n");
}



void unary_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] unary_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    switch (state->next)
    {
        case TOK_INTE:
            match(state, TOK_INTE);
            unary_expression(state);
            break;

        case TOK_ADDRESS:
            match(state, TOK_ADDRESS);
            lvalue(state);
            break;

        case TOK_DEREF:
            match(state, TOK_DEREF);
            expression(state);
            break;

        default:
            primary_expression(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] unary_expression\n");
}


void primary_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] primary_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    switch (state->next)
    {
        /* parenthesized expression */
        case TOK_LPAREN:
            match(state, TOK_LPAREN);
            expression(state);
            match(state, TOK_RPAREN);
            break;

        /* literals */
        case TOK_INT_LIT:
        case TOK_FLOAT_LIT:
            next_token(state);
            break;

        case TOK_STRING_LIT:
            next_token(state);
            break;

        /* field access starts explicitly */
        case TOK_FALT:
            field_access(state);
            break;

        /* identifier-based */
        case TOK_IDENTIFIER:
            if (state->next_next == TOK_LPAREN)
            {
                function_call(state);
            }
            else if (state->next_next == TOK_LBLOCK)
            {
                array_access(state);
            }
            else
            {
                match(state, TOK_IDENTIFIER);
            }
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf(
                "Syntax error: expected primary expression, got %s\n",
                tok2name(state->next)
            );

            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] primary_expression\n");
}



//Function calls
void function_call(ParState *state)
{
    printf("[ENTER] function_call\n");
    /* stub */
    printf("[EXIT ] function_call\n");
}


void argument_list(ParState *state)
{
    printf("[ENTER] argument_list\n");
    /* stub */
    printf("[EXIT ] argument_list\n");
}


