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


// Reports a syntax error at the current token location
static void syntax_error_at(ParState *state, const char *msg)
{
    state->error_count++;
    state->panic_mode = 1;

    printf(
        "Syntax error at %d:%d: %s (got %s)\n",
        state->tokens[state->index].row,
        state->tokens[state->index].col,
        msg,
        tok2name(state->next)
    );
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
        case TOK_TOM:
        {
            //must be: <type> <identifier> ...
            if (state->next_next != TOK_IDENTIFIER)
            {
                int start_index = state->index;

                syntax_error_at(state, "expected identifier after type specifier");
                sync_to_follow(state);

                //forces progress if sync didn't move
                if (state->index == start_index && state->next != TOK_EOF)
                    next_token(state);

                break;
            }

            //looks one past identifier: '(' => function, ':' => declaration
            {
                TokenType after_id = state->tokens[state->index + 2].token;

                if (after_id == TOK_LPAREN)
                    function_declaration(state);
                else if (after_id == TOK_ASSIGN)
                    declaration_statement(state);
                else
                {
                    int start_index = state->index;

                    syntax_error_at(state, "expected '(' or ':' after identifier");
                    sync_to_follow(state);

                    //forces progress if sync didn't move
                    if (state->index == start_index && state->next != TOK_EOF)
                        next_token(state);
                }
            }

            break;
        }

        default:
        {
            int start_index = state->index;

            syntax_error_at(state, "unexpected token in global scope");
            sync_to_follow(state);

            //forces progress if sync didn't move
            if (state->index == start_index && state->next != TOK_EOF)
                next_token(state);

            break;
        }
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
        syntax_error_at(state, "expected function name");

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

// ---------------------------------------------------
// Parses a declaration statement
// Pre: state->next begins a type specifier
// Post: consumes a full declaration statement
// ---------------------------------------------------




void declaration_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] declaration_statement\n");

    saved_sync = state->sync_set;

    //parses the type part
    type_specifier(state);

    //expects ':'
    if (state->next != TOK_ASSIGN)
    {
        syntax_error_at(state, "expected ':' in declaration");
        goto recover;
    }
    next_token(state);

    //expects an identifier
    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected identifier in declaration");
        goto recover;
    }
    next_token(state);

    //allows either ';' or ', <expr> ;'
    if (state->next == TOK_SEMI)
    {
        next_token(state);
        state->sync_set = saved_sync;
        printf("[EXIT ] declaration_statement\n");
        return;
    }

    //initializer branch
    if (state->next != TOK_COMMA)
    {
        syntax_error_at(state, "expected ';' or ',' after identifier in declaration");
        goto recover;
    }
    next_token(state);

    //parses initializer expression
    expression(state);

    //expects ';' at end
    if (state->next != TOK_SEMI)
    {
        syntax_error_at(state, "expected ';' after declaration");
        goto recover;
    }
    next_token(state);

    state->sync_set = saved_sync;
    printf("[EXIT ] declaration_statement\n");
    return;

recover:
    //skips to a safe boundary for declarations
    while (state->next != TOK_SEMI &&
           state->next != TOK_RBLOCK &&
           state->next != TOK_EOF)
    {
        next_token(state);
    }

    //consumes the statement terminator if present
    if (state->next == TOK_SEMI)
        next_token(state);

    state->sync_set = saved_sync;
    printf("[EXIT ] declaration_statement\n");
}

// ---------------------------------------------------
// Parses an optional array type suffix: '<' expression '>'
// Pre: next may be TOK_LBLOCK  Post: consumed suffix if present
// ---------------------------------------------------
static void optional_type_array_suffix(ParState *state)
{
    // consumes '<expr>' when used as a suffix on a type
    if (state->next == TOK_LBLOCK)
    {
        match(state, TOK_LBLOCK);
        expression(state);
        match(state, TOK_RBLOCK);
    }
}



void typedef_declaration(ParState *state)
{
    printf("[ENTER] typedef_declaration\n");

    match(state, TOK_TYPDEF);

    if (state->next == TOK_STRUKTUR)
    {
        match(state, TOK_STRUKTUR);

        // struct name
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

        // -------------------------------------------
        // struct field list:
        //   type_specifier [ '<' expr '>' ] ':' ID [ '<' expr '>' ] ';'
        // -------------------------------------------
        while (state->next != TOK_RBLOCK && state->next != TOK_EOF)
        {
            // field type
            type_specifier(state);

            // optional array suffix on the type: person<2>: föräldrar;
            if (state->next == TOK_LBLOCK)
            {
                match(state, TOK_LBLOCK);
                expression(state);
                match(state, TOK_RBLOCK);
            }

            // ':'
            match(state, TOK_ASSIGN);

            // field name
            if (state->next == TOK_IDENTIFIER)
                match(state, TOK_IDENTIFIER);
            else
            {
                state->error_count++;
                printf("Syntax error: expected field name in struct\n");
                sync_to_follow(state);
                break;
            }

            // optional array suffix on the field name (keep if you want both forms)
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
        // TYPDEF <type> ID ;
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


// Parses a standalone STRUKTUR declaration and its field list
void struct_declaration(ParState *state)
{
    printf("[ENTER] struct_declaration\n");

    // consumes STRUKTUR
    match(state, TOK_STRUKTUR);

    // consumes struct name
    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);
    }
    else
    {
        syntax_error_at(state, "expected struct name");
        sync_to_follow(state);
        printf("[EXIT ] struct_declaration\n");
        return;
    }

    // consumes '<'
    match(state, TOK_LBLOCK);

    // parses zero or more fields until '>' or EOF
    while (state->next != TOK_RBLOCK && state->next != TOK_EOF)
    {
        // parses the field type
        type_specifier(state);

        // optional type-array suffix: person<2>: föräldrar;
        if (state->next == TOK_LBLOCK)
        {
            match(state, TOK_LBLOCK);
            expression(state);
            match(state, TOK_RBLOCK);
        }

        // consumes ':'
        match(state, TOK_ASSIGN);

        // parses field name
        if (state->next == TOK_IDENTIFIER)
        {
            match(state, TOK_IDENTIFIER);
        }
        else
        {
            syntax_error_at(state, "expected field name");
            sync_to_follow(state);
            break;
        }

        // optional field-array suffix (keep it if you want arrays after field names too)
        if (state->next == TOK_LBLOCK)
        {
            match(state, TOK_LBLOCK);
            expression(state);
            match(state, TOK_RBLOCK);
        }

        // consumes ';'
        match(state, TOK_SEMI);
    }

    // consumes '>'
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
            syntax_error_at(
                state,
                "unexpected token in type declaration"
            );

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
        case TOK_VAL:
        case TOK_TOM:          /* <-- ADD THIS */
        case TOK_IDENTIFIER:
            next_token(state);
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error at %d:%d: expected type, got %s\n",
                   state->tokens[state->index].row,
                   state->tokens[state->index].col,
                   tok2name(state->next));

            sync_to_follow(state);
            state->sync_set = saved_sync;
            printf("[EXIT ] type_specifier\n");
            return;
    }

    while (state->next == TOK_PEK)
        next_token(state);

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



// ---------------------------------------------------
// Parses a single parameter: <type_specifier> ':' ID
// Pre: next is at start of parameter  Post: consumed one parameter (or recovered)
// ---------------------------------------------------
void parameter(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] parameter\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_parameter;

    // parse the type (includes optional PEK chain)
    type_specifier(state);

    // consume ':' between type and parameter name
    if (state->next == TOK_ASSIGN)
    {
        match(state, TOK_ASSIGN);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error at %d:%d: expected ':' after parameter type (got %s)\n",
               state->tokens[state->index].row,
               state->tokens[state->index].col,
               tok2name(state->next));

        sync_to_follow(state);
        state->sync_set = saved_sync;
        printf("[EXIT ] parameter\n");
        return;
    }

    // parse the parameter name
    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);
    }
    else
    {
        state->error_count++;
        state->panic_mode = 1;

        printf("Syntax error at %d:%d: expected parameter name (got %s)\n",
               state->tokens[state->index].row,
               state->tokens[state->index].col,
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



// ---------------------------------------------------
// Dispatches a statement based on lookahead
// Pre: next is at start of a statement  Post: consumed one statement (or recovered)
// ---------------------------------------------------
void statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    switch (state->next)
    {
        // declarations starting with built-in type keywords
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

        // control / flow
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

        // field statement
        case TOK_FALT:
            field_statement(state);
            break;

        // ------------------------------------------------
        // TOK_IDENTIFIER can be either:
        // - type-led declaration (typedef type)
        // - assignment/expression
        // ------------------------------------------------
        case TOK_IDENTIFIER:
        {
            // declaration heuristic: TypeName ':' VarName (',' | ';')
            if (state->next_next == TOK_ASSIGN)
            {
                TokenType t2 = state->tokens[state->index + 2].token;
                TokenType t3 = state->tokens[state->index + 3].token;

                // if it looks like: ID ':' ID ','  OR  ID ':' ID ';'
                if (t2 == TOK_IDENTIFIER && (t3 == TOK_COMMA || t3 == TOK_SEMI))
                {
                    declaration_statement(state);
                    break;
                }
            }

            // otherwise treat as assignment / expression as before
            if (state->next_next == TOK_ASSIGN ||
                state->next_next == TOK_OKAR ||
                state->next_next == TOK_MINSKAR ||
                state->next_next == TOK_PLUS_ASSIGN ||
                state->next_next == TOK_MINUS_ASSIGN)
            {
                assignment_statement(state);
            }
            else
            {
                expression_statement(state);
            }
            break;
        }

        // expression starters
        case TOK_STRING_LIT:
        case TOK_INT_LIT:
        case TOK_FLOAT_LIT:
        case TOK_LPAREN:
        case TOK_INTE:
        case TOK_ADDRESS:
        case TOK_DEREF:
            expression_statement(state);
            break;

        default:
            state->error_count++;
            state->panic_mode = 1;

            printf("Syntax error at %d:%d: unexpected token in statement (got %s)\n",
                   state->tokens[state->index].row,
                   state->tokens[state->index].col,
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

    //deref lvalue: VÄRDE VID <identifier>
    if (state->next == TOK_DEREF)
    {
        match(state, TOK_DEREF);

        //expects identifier after deref
        if (state->next == TOK_IDENTIFIER)
            match(state, TOK_IDENTIFIER);
        else
        {
            syntax_error_at(state, "expected identifier after VÄRDE VID");
            sync_to_follow(state);
        }

        printf("[EXIT ] lvalue\n");
        return;
    }

    //field lvalue
    if (state->next == TOK_FALT)
    {
        field_access(state);
    }
    //identifier or array lvalue
    else if (state->next == TOK_IDENTIFIER)
    {
        if (state->next_next == TOK_LBLOCK)
            array_access(state);
        else
            match(state, TOK_IDENTIFIER);
    }
    else
    {
        syntax_error_at(state, "expected lvalue");
        sync_to_follow(state);
    }

    printf("[EXIT ] lvalue\n");
}

//continue here
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
        syntax_error_at(state, "expected identifier after FÄLT");

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
    const TokenType *saved_sync;

    printf("[ENTER] return_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    /* 'ÅTERVÄND' */
    match(state, TOK_ATERVAND);

    /* optional return expression */
    if (state->next != TOK_SEMI)
    {
        expression(state);
    }

    /* ';' */
    match(state, TOK_SEMI);

    state->sync_set = saved_sync;

    printf("[EXIT ] return_statement\n");
}


void expression_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] expression_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    /* expression */
    expression(state);

    /* ';' */
    match(state, TOK_SEMI);

    state->sync_set = saved_sync;

    printf("[EXIT ] expression_statement\n");
}


void field_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] field_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    /* field access (starts with FÄLT) */
    field_access(state);

    /* direct assignment: ':' expression */
    if (state->next == TOK_ASSIGN)   /* ':' */
    {
        match(state, TOK_ASSIGN);
        expression(state);
    }
    /* unary update: ÖKAR / MINSKAR */
    else if (state->next == TOK_OKAR || state->next == TOK_MINSKAR)
    {
        match(state, state->next);
    }
    /* compound update: ÖKAR MED / MINSKAR MED */
    else if (state->next == TOK_PLUS_ASSIGN ||
             state->next == TOK_MINUS_ASSIGN)
    {
        match(state, state->next);
        expression(state);
    }
    else
    {
        syntax_error_at(
            state,
            "expected ':', field update operator, or compound update operator"
        );

        sync_to_follow(state);
        state->sync_set = saved_sync;
        printf("[EXIT ] field_statement\n");
        return;
    }

    /* ';' */
    match(state, TOK_SEMI);

    state->sync_set = saved_sync;

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
        syntax_error_at(state, "expected OM");

        sync_to_follow(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] if_statement\n");
}


// Parses a switch statement with FALL and optional ANNARS clauses
void switch_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] switch_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    if (state->next != TOK_VAXEL)
    {
        syntax_error_at(state, "expected VÄXEL");
        sync_to_follow(state);
        state->sync_set = saved_sync;
        printf("[EXIT ] switch_statement\n");
        return;
    }

    match(state, TOK_VAXEL);
    match(state, TOK_LPAREN);
    expression(state);
    match(state, TOK_RPAREN);

    match(state, TOK_LBLOCK);

    //parses zero or more FALL clauses
    while (state->next == TOK_FALL)
    {
        match(state, TOK_FALL);
        expression(state);
        match(state, TOK_ASSIGN);

        //parses statements until next label or end of switch
        while (state->next != TOK_FALL &&
               state->next != TOK_ANNARS &&
               state->next != TOK_RBLOCK &&
               state->next != TOK_EOF)
        {
            statement(state);
        }
    }

    //parses optional ANNARS clause
    if (state->next == TOK_ANNARS)
    {
        match(state, TOK_ANNARS);
        match(state, TOK_ASSIGN);

        //parses statements until end of switch
        while (state->next != TOK_RBLOCK &&
               state->next != TOK_EOF)
        {
            statement(state);
        }
    }

    match(state, TOK_RBLOCK);

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
            syntax_error_at(state, "unexpected token in loop statement");

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
        syntax_error_at(state, "expected MEDAN");

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
        syntax_error_at(state, "expected GÖR");

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

        //parses for_init
        if (state->next != TOK_SEMI)
        {
            if (state->next == TOK_HEL ||
                state->next == TOK_FLYT ||
                state->next == TOK_BOK  ||
                state->next == TOK_BIT  ||
                state->next == TOK_HALV ||
                state->next == TOK_BYTE ||
                state->next == TOK_ORD  ||
                state->next == TOK_STATISK)
            {
                //declaration-style init consumes its own ';'
                declaration_statement(state);
            }
            else
            {
                //assignment-style init does not consume ';'
                assignment_core(state);
                match(state, TOK_SEMI);
            }
        }
        else
        {
            //empty init
            match(state, TOK_SEMI);
        }

        //parses condition (must end with ';')
        expression(state);
        match(state, TOK_SEMI);

        //parses update (optional)
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
            syntax_error_at(
                state,
                "expected assignment operator after lvalue"
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

// ---------------------------------------------------
// Checks if token can start a type specifier
// Pre: token Post: returns 1 if token starts a type, else 0
// ---------------------------------------------------
static int is_type_starter(TokenType t)
{
    switch (t)
    {
        case TOK_HEL:
        case TOK_FLYT:
        case TOK_BOK:
        case TOK_BIT:
        case TOK_HALV:
        case TOK_BYTE:
        case TOK_ORD:
        case TOK_VAL:
        case TOK_IDENTIFIER:
            return 1;

        default:
            return 0;
    }
}




// ---------------------------------------------------
// Detects if the current position looks like a cast: ( <type_specifier> )
// Pre: state->index at '(' Post: returns 1 if cast pattern, else 0
// ---------------------------------------------------
static int is_cast_start(ParState *state)
{
    int i;

    if (state->next != TOK_LPAREN)
        return 0;

    if (!is_type_starter(state->next_next))
        return 0;

    // scan: '(' <type> { PEK } ')'
    i = state->index + 2;

    // consumes the base type token at index+1, now check pointer suffixes
    while (i < state->token_count && state->tokens[i].token == TOK_PEK)
        i++;

    return (i < state->token_count && state->tokens[i].token == TOK_RPAREN);
}

void unary_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] unary_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    // cast: '(' type_specifier ')' unary_expression
    if (is_cast_start(state))
    {
        match(state, TOK_LPAREN);
        type_specifier(state);
        match(state, TOK_RPAREN);
        unary_expression(state);

        state->sync_set = saved_sync;
        printf("[EXIT ] unary_expression\n");
        return;
    }

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

// ---------------------------------------------------
// Parses primary expressions (literals, identifiers, calls, parenthesis, casts)
// Pre: next begins a primary expr  Post: consumed one primary expr (or recovered)
// ---------------------------------------------------
void primary_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] primary_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    switch (state->next)
    {
        case TOK_LPAREN:
            match(state, TOK_LPAREN);
            expression(state);
            match(state, TOK_RPAREN);
            break;

        /* aggregate literal: < ... > */
        case TOK_LBLOCK:
            match(state, TOK_LBLOCK);

            /* optional value list */
            if (state->next != TOK_RBLOCK)
            {
                expression(state);

                while (state->next == TOK_COMMA)
                {
                    match(state, TOK_COMMA);
                    expression(state);
                }

                /* allow optional trailing ';' inside aggregates, to support "< ... ; >" */
                if (state->next == TOK_SEMI)
                {
                    match(state, TOK_SEMI);
                }
            }

            match(state, TOK_RBLOCK);
            break;

        case TOK_INT_LIT:
        case TOK_FLOAT_LIT:
        case TOK_STRING_LIT:
            next_token(state);
            break;

        case TOK_FALT:
            field_access(state);
            break;

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

void function_call_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] function_call_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    function_call(state);

    /* optional ';' */
    if (state->next == TOK_SEMI)
        match(state, TOK_SEMI);

    state->sync_set = saved_sync;

    printf("[EXIT ] function_call_statement\n");
}


void function_call(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] function_call\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    /* function name */
    match(state, TOK_IDENTIFIER);

    /* '(' */
    match(state, TOK_LPAREN);

    /* optional argument list */
    if (state->next != TOK_RPAREN)
    {
        expression(state);

        while (state->next == TOK_COMMA)
        {
            match(state, TOK_COMMA);
            expression(state);
        }
    }

    /* ')' */
    match(state, TOK_RPAREN);

    state->sync_set = saved_sync;

    printf("[EXIT ] function_call\n");
}



void argument_list(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] argument_list\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_expression;

    /* first argument */
    expression(state);

    /* additional arguments */
    while (state->next == TOK_COMMA)
    {
        match(state, TOK_COMMA);
        expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] argument_list\n");
}



