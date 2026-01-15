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


/* ---------------------------------------------------
   FOLLOW sets (panic-mode sync sets)
   --------------------------------------------------- */

/* primary_expression is used inside expressions, so its follow
   includes expression operators and expression terminators. */
static const TokenType FOLLOW_primary_expression[] = {
    /* multiplicative */
    TOK_MUL, TOK_DIV, TOK_EXP,

    /* additive */
    TOK_PLUS, TOK_MINUS,

    /* relational / equality */
    TOK_LT, TOK_LTE, TOK_GT, TOK_GTE, TOK_EQ, TOK_NEQ,

    /* logical */
    TOK_OCH, TOK_ELLER,
    TOK_ASSIGN,   // ':' assignment after identifier/array_access/field_access
    TOK_OKAR,     // postfix increment form: 'i ÖKAR;'
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    /* expression terminators / separators */
    TOK_COMMA, TOK_SEMI, TOK_RPAREN, TOK_RBLOCK,

    /* end / error */
    TOK_EOF, TOK_ERROR
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



/* ---------------------------------------------------
   Parses a global statement.
   Supports:
     - type declarations (typedef/struct)
     - global variable declarations: <type> ":" ...
     - function declarations:        <type> ":" <id> "(" ...
   Pre: next begins a global statement
   Post: consumes one global statement or recovers
------------------------------------------------------ */
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
        case TOK_IDENTIFIER: //user-defined types at global scope
        {
            int start_index = state->index;
            int after_type_i = scan_after_type_specifier(state, start_index);
            TokenType after_type = state->tokens[after_type_i].token;

            //both declarations and functions must now have ':' after the type
            if (after_type == TOK_ASSIGN)
            {
                TokenType t1 = state->tokens[after_type_i + 1].token;
                TokenType t2 = state->tokens[after_type_i + 2].token;

                //function: <type> ":" <id> "(" ...
                if (t1 == TOK_IDENTIFIER && t2 == TOK_LPAREN)
                {
                    function_declaration(state);
                    break;
                }

                //declaration: <type> ":" ...
                declaration_statement(state);
                break;
            }

            //error + force progress
            {
                int sync_start = state->index;

                syntax_error_at(state, "expected ':' after type specifier");
                sync_to_follow(state);

                if (state->index == sync_start && state->next != TOK_EOF)
                    next_token(state);
            }

            break;
        }

        default:
        {
            int start_index = state->index;

            syntax_error_at(state, "unexpected token in global scope");
            sync_to_follow(state);

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

    //return type
    type_specifier(state);

    //enforces ':' between type and identifier
    if (state->next == TOK_ASSIGN)
    {
        match(state, TOK_ASSIGN);
    }
    else
    {
        syntax_error_at(state, "expected ':' after function return type");
        //recovery: treat ':' as inserted and continue
    }

    //function name
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

    //'('
    match(state, TOK_LPAREN);

    //parameter list (may be empty)
    parameter_list(state);

    //')'
    match(state, TOK_RPAREN);

    //function body
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


/* ---------------------------------------------------
   Scans forward from a type-start token to the first token
   after the full type specifier (base type + PEK* + <...>*)
   Pre: tokens[start_index] begins a type specifier
   Post: returns index of first token AFTER the type specifier
------------------------------------------------------ */
int scan_after_type_specifier(const ParState *state, int start_index)
{
    int i = start_index;

    //consume the base type token (HEL/FLYT/... or user type identifier)
    i++;

    //consume pointer modifiers: PEK*
    while (state->tokens[i].token == TOK_PEK && state->tokens[i].token != TOK_EOF)
        i++;

    //consume array dimensions: (< expr >)*
    while (state->tokens[i].token == TOK_LBLOCK && state->tokens[i].token != TOK_EOF)
    {
        int depth = 0;

        //walk until matching TOK_RBLOCK
        while (state->tokens[i].token != TOK_EOF)
        {
            if (state->tokens[i].token == TOK_LBLOCK)
                depth++;

            if (state->tokens[i].token == TOK_RBLOCK)
            {
                depth--;
                i++;
                break;
            }

            i++;
        }

        //unbalanced "< ...", just stop (let normal parsing error later)
        if (depth > 0)
            break;
    }

    return i;
}


// ---------------------------------------------------
// Parses a type specifier (built-in, user-defined, and struct-typed)
// Pre: state->next is at the first token of a type
// Post: consumes the full type (including PEK* and optional array dims)
// ---------------------------------------------------
void type_specifier(ParState *state)
{
    printf("[ENTER] type_specifier\n");

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
        case TOK_TOM:
            //consumes built-in types
            match(state, state->next);
            break;

        case TOK_IDENTIFIER:
            //consumes user-defined type aliases
            match(state, TOK_IDENTIFIER);
            break;

        case TOK_STRUKTUR:
            //consumes struct-typed declarations: STRUKTUR PERSON
            match(state, TOK_STRUKTUR);

            //expects the struct type name
            if (state->next == TOK_IDENTIFIER)
                match(state, TOK_IDENTIFIER);
            else
            {
                syntax_error_at(state, "expected struct type name after STRUKTUR");
                sync_to_follow(state);
                printf("[EXIT ] type_specifier\n");
                return;
            }
            break;

        default:
            syntax_error_at(state, "expected type specifier");
            sync_to_follow(state);
            printf("[EXIT ] type_specifier\n");
            return;
    }

    //consume pointers: PEK*
    while (state->next == TOK_PEK)
        match(state, TOK_PEK);

    //consume array dimensions: < expr > (repeatable)
    while (state->next == TOK_LBLOCK)
    {
        match(state, TOK_LBLOCK);
        expression(state);
        match(state, TOK_RBLOCK);
    }

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
// Parses a single parameter
// Pre: state->next begins a type specifier
// Post: consumes one parameter of form: <type> ":" <id>
// ---------------------------------------------------
void parameter(ParState *state)
{
    printf("[ENTER] parameter\n");

    type_specifier(state);

    //enforces ':' between type and identifier
    if (state->next == TOK_ASSIGN)
    {
        match(state, TOK_ASSIGN);
    }
    else
    {
        syntax_error_at(state, "missing ':' between parameter type and name");
        //recovery: treat ':' as inserted and continue
    }

    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);
    }
    else
    {
        syntax_error_at(state, "expected parameter name");
        //recovery: let caller sync
    }

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
// Scans forward in lookahead for any token in a set, but stops at statement boundaries
// Pre: state initialized, targets != NULL, target_count > 0, max_ahead > 0  Post: returns 1 if any target is found before boundary else 0
// ---------------------------------------------------
static int lookahead_contains_any_until_boundary(ParState *state, const TokenType *targets, int target_count, int max_ahead)
{
    int i;
    int k;
    int angle_depth;

    angle_depth = 0;

    //scans tokens ahead until it hits a likely statement boundary
    for (i = 1; i <= max_ahead; i++)
    {
        TokenType t = peek_token(state, i);

        //tracks < ... > nesting so array access does not look like a statement boundary
        if (t == TOK_LBLOCK)
            angle_depth++;

        //only treat '>' as a statement boundary if we are not inside < ... >
        if (t == TOK_RBLOCK)
        {
            if (angle_depth > 0)
            {
                angle_depth--;
            }
            else
            {
                break;
            }
        }

        //stops scanning at common statement boundaries
        if (t == TOK_SEMI || t == TOK_EOF)
            break;

        //checks whether this token matches any target
        for (k = 0; k < target_count; k++)
        {
            if (t == targets[k])
                return 1;
        }
    }

    return 0;
}





// ---------------------------------------------------
// Parses update statements
// Pre: next token begins a valid lvalue (identifier/array_access/field_access/deref) Post: consumes the whole update statement
// ---------------------------------------------------
static void update_statement(ParState *state)
{
    int lvalue_start_index;

    //tracks progress so we can bail out cleanly on lvalue failure
    lvalue_start_index = state->index;

    //parses the lvalue portion (now supports VÄRDE VID)
    if (state->next == TOK_DEREF || state->next == TOK_FALT || state->next == TOK_IDENTIFIER)
    {
        //reuses the shared lvalue parser (handles deref/field/array/id)
        lvalue(state);
    }
    else
    {
        syntax_error_at(state, "expected lvalue before update operator");
        sync_to_follow(state);
        return;
    }

    //if lvalue() failed and did not advance, abort to avoid cascading errors
    if (state->index == lvalue_start_index)
    {
        syntax_error_at(state, "expected lvalue before update operator");
        sync_to_follow(state);
        return;
    }

    //consumes the operator and parses expression when required
    if (state->next == TOK_OKAR)
    {
        match(state, TOK_OKAR);

        //ends the statement
        match(state, TOK_SEMI);
        return;
    }

    if (state->next == TOK_MINSKAR)
    {
        match(state, TOK_MINSKAR);

        //ends the statement
        match(state, TOK_SEMI);
        return;
    }

    if (state->next == TOK_PLUS_ASSIGN)
    {
        match(state, TOK_PLUS_ASSIGN);

        //parses the required rhs expression for "ÖKAR MED"
        expression(state);

        //ends the statement
        match(state, TOK_SEMI);
        return;
    }

    if (state->next == TOK_MINUS_ASSIGN)
    {
        match(state, TOK_MINUS_ASSIGN);

        //parses the required rhs expression for "MINSKAR MED"
        expression(state);

        //ends the statement
        match(state, TOK_SEMI);
        return;
    }

    syntax_error_at(state, "expected ÖKAR, MINSKAR, ÖKAR MED, or MINSKAR MED after lvalue");
    sync_to_follow(state);
}




// ---------------------------------------------------
// Dispatches and parses a single statement
// Pre: state->next is positioned at the start of a statement
// Post: consumes one statement or performs recovery to FOLLOW_statement
// ---------------------------------------------------
void statement(ParState *state)
{
    const TokenType *saved_sync;
    int start_index;

    printf("[ENTER] statement\n");

    start_index = state->index;

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    switch (state->next)
    {
        case TOK_SEMI:
            // empty statement
            match(state, TOK_SEMI);
            break;

        case TOK_RBLOCK:
            // end of block marker, statement_list should stop here
            break;

        case TOK_HEL:
        case TOK_FLYT:
        case TOK_BOK:
        case TOK_BIT:
        case TOK_HALV:
        case TOK_BYTE:
        case TOK_ORD:
        case TOK_VAL:
        case TOK_STRUKTUR: /* allows local struct-typed variable declarations */
            declaration_statement(state);
            break;

        case TOK_OM:
            if_statement(state);
            break;

        case TOK_MEDAN:
        case TOK_FOR:
        case TOK_GOR:
            loop_statement(state);
            break;

        case TOK_ATERVAND:
            return_statement(state);
            break;

        case TOK_IDENTIFIER:
        case TOK_FALT:
        case TOK_DEREF:
        {
            const TokenType update_ops[] = { TOK_OKAR, TOK_MINSKAR, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN };
            const TokenType assign_ops[] = { TOK_ASSIGN };

            //routes update statements (handles both immediate and after array_access/field_access)
            if (lookahead_contains_any_until_boundary(state, update_ops, 4, 32))
            {
                update_statement(state);
                break;
            }

            //routes plain assignment statements (arr<j>: x;  a: b;)
            if (lookahead_contains_any_until_boundary(state, assign_ops, 1, 32))
            {
                assignment_statement(state);
                break;
            }

            //fallback to expression statement (function calls, etc.)
            expression_statement(state);
            break;
        }

        case TOK_OKAR:
        case TOK_MINSKAR:
            //stray postfix operators should not stall the parser
            syntax_error_at(state, "unexpected ÖKAR/MINSKAR without lvalue");
            next_token(state);
            break;

        case TOK_PLUS_ASSIGN:
        case TOK_MINUS_ASSIGN:
            //stray compound update operators should not stall the parser
            syntax_error_at(state, "unexpected ÖKAR MED/MINSKAR MED without lvalue");
            next_token(state);
            break;

        default:
            syntax_error_at(state, "unexpected token in statement");
            sync_to_follow(state);

            //ensures forward progress if sync didn't move the index
            if (state->index == start_index)
                next_token(state);
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

// ---------------------------------------------------
// Parses an lvalue
// Pre: state->next begins an lvalue
// Post: consumes an lvalue or performs recovery
// ---------------------------------------------------
void lvalue(ParState *state)
{
    printf("[ENTER] lvalue\n");

    //deref lvalue: VÄRDE VID <operand>
    //supports:
    //  VÄRDE VID p
    //  VÄRDE VID arr<i>
    //  VÄRDE VID FÄLT p SCORE
    //  VÄRDE VID (VÄRDE VID pp)
    //  VÄRDE VID (FÄLT p SCORE)
    if (state->next == TOK_DEREF)
    {
        match(state, TOK_DEREF);

        //parenthesized deref operand
        if (state->next == TOK_LPAREN)
        {
            match(state, TOK_LPAREN);

            //parses the operand expression inside parentheses
            expression(state);

            match(state, TOK_RPAREN);

            printf("[EXIT ] lvalue\n");
            return;
        }

        //non-parenthesized deref operand forms
        if (state->next == TOK_FALT)
        {
            //deref of field access
            field_access(state);

            printf("[EXIT ] lvalue\n");
            return;
        }

        if (state->next == TOK_IDENTIFIER)
        {
            //deref of identifier or array element
            if (state->next_next == TOK_LBLOCK)
                array_access(state);
            else
                match(state, TOK_IDENTIFIER);

            printf("[EXIT ] lvalue\n");
            return;
        }

        if (state->next == TOK_DEREF)
        {
            //allows chained deref without parentheses if your syntax ever uses it
            lvalue(state);

            printf("[EXIT ] lvalue\n");
            return;
        }

        syntax_error_at(state, "expected deref operand after VÄRDE VID");
        sync_to_follow(state);

        printf("[EXIT ] lvalue\n");
        return;
    }

    //field lvalue: FÄLT <base> <field> ...
    if (state->next == TOK_FALT)
    {
        field_access(state);

        printf("[EXIT ] lvalue\n");
        return;
    }

    //identifier lvalue: id or array element
    if (state->next == TOK_IDENTIFIER)
    {
        if (state->next_next == TOK_LBLOCK)
            array_access(state);
        else
            match(state, TOK_IDENTIFIER);

        printf("[EXIT ] lvalue\n");
        return;
    }

    syntax_error_at(state, "expected lvalue");
    sync_to_follow(state);

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

// ---------------------------------------------------
// Parses field access: FÄLT <base> <field> { <field> } with optional array suffixes
// Pre: state->next == TOK_FALT
// Post: consumes a complete field_access expression/lvalue or recovers to sync set
// ---------------------------------------------------
void field_access(ParState *state)
{
    printf("[ENTER] field_access\n");

    match(state, TOK_FALT);

    //expects the base identifier after FÄLT
    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected identifier after FÄLT");
        sync_to_follow(state);
        printf("[EXIT ] field_access\n");
        return;
    }

    //consumes the base identifier
    match(state, TOK_IDENTIFIER);

    //allows array suffix on the base: ps<0>
    if (state->next == TOK_LBLOCK)
    {
        match(state, TOK_LBLOCK);

        //parses the index expression inside <>
        expression(state);

        match(state, TOK_RBLOCK);
    }

    //requires at least one field identifier after the base
    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected field name after FÄLT base");
        sync_to_follow(state);
        printf("[EXIT ] field_access\n");
        return;
    }

    //consumes one or more field identifiers, each optionally with an array suffix
    while (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);

        //allows array suffix on a field node: FÄLT p ARR<2>
        if (state->next == TOK_LBLOCK)
        {
            match(state, TOK_LBLOCK);

            //parses the index expression inside <>
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

// ---------------------------------------------------
// Parses unary expressions (prefix operators, casts) and delegates to primary expressions
// Pre: state->next begins a unary or primary expression  Post: consumes one unary/primary expression (or recovers)

/* unary_expression ::= (INTE | & | *) unary_expression | primary_expression
   FOLLOW(unary_expression) = tokens that may legally appear immediately after a unary */
static const TokenType FOLLOW_unary_expression[] = {
    /* multiplicative */
    TOK_MUL,
    TOK_DIV,

    /* additive */
    TOK_PLUS,
    TOK_MINUS,

    /* relational */
    TOK_LT,
    TOK_GT,
    TOK_LTE,
    TOK_GTE,

    /* equality / logical */
    TOK_EQ,
    TOK_NEQ,
    TOK_OCH,
    TOK_ELLER,

    /* expression terminators / separators */
    TOK_SEMI,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_RBLOCK,

    /* end / sentinel */
    TOK_EOF,
    TOK_ERROR
};

// ---------------------------------------------------
// Checks whether a token is a builtin type keyword (for cast disambiguation)
// Pre: t is a valid TokenType  Post: returns 1 if t is a builtin type token else 0
// ---------------------------------------------------
static int is_type_token(TokenType t)
{
    //builtin type keywords only (prevents ambiguity with grouped identifiers)
    if (t == TOK_HEL  ||
        t == TOK_FLYT ||
        t == TOK_BOK  ||
        t == TOK_BIT  ||
        t == TOK_HALV ||
        t == TOK_BYTE ||
        t == TOK_ORD  ||
        t == TOK_VAL  ||
        t == TOK_TOM)
        return 1;

    return 0;
}


// ---------------------------------------------------
// Parses unary expressions including casts, NOT, +/- , address-of and deref
// Pre: cursor at start of a unary_expression
// Post: cursor advanced past unary_expression or recovered to FOLLOW_unary_expression
// ---------------------------------------------------
void unary_expression(ParState *state)
{
    const TokenType *saved_sync;
    int start_index;

    printf("[ENTER] unary_expression\n");

    start_index = state->index;

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_unary_expression;

    //handles casts like: (HEL) x
    if (state->next == TOK_LPAREN && is_type_token(peek_token(state, 1)))
    {
        match(state, TOK_LPAREN);
        type_specifier(state);
        match(state, TOK_RPAREN);

        //parses the operand after the cast
        unary_expression(state);

        state->sync_set = saved_sync;
        printf("[EXIT ] unary_expression\n");
        return;
    }

    switch (state->next)
    {
        case TOK_INTE:
            //parses logical NOT
            match(state, TOK_INTE);
            unary_expression(state);
            break;

        case TOK_MINUS:
            //parses unary negation, e.g. -1, -(a + b)
            match(state, TOK_MINUS);
            unary_expression(state);
            break;

        case TOK_PLUS:
            //parses unary plus, e.g. +1, +(a)
            match(state, TOK_PLUS);
            unary_expression(state);
            break;

        case TOK_ADDRESS:
            //parses address-of on an lvalue, optionally parenthesized: ADRESS AV (FÄLT p SCORE)
            match(state, TOK_ADDRESS);

            //supports optional parentheses around lvalue
            if (state->next == TOK_LPAREN)
            {
                match(state, TOK_LPAREN);

                //parses the lvalue inside parentheses
                lvalue(state);

                match(state, TOK_RPAREN);
            }
            else
            {
                //parses direct lvalue after ADRESS AV
                lvalue(state);
            }
            break;

        case TOK_DEREF:
            //parses dereference in expressions
            match(state, TOK_DEREF);

            //parses the operand of dereference
            expression(state);
            break;

        default:
            //falls back to primary expressions (literals, identifiers, calls, grouping, etc.)
            primary_expression(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] unary_expression\n");
}

/* ---------------------------------------------------
   Parses array literal used in expressions, e.g. <1,2,3> or <>
   Pre: next == TOK_LBLOCK
   Post: cursor after TOK_RBLOCK
------------------------------------------------------ */
void array_literal(ParState *state)
{
    printf("[ENTER] array_literal\n");

    match(state, TOK_LBLOCK);

    //empty literal: <>
    if (state->next != TOK_RBLOCK)
    {
        expression(state);

        while (state->next == TOK_COMMA)
        {
            match(state, TOK_COMMA);
            expression(state);
        }
    }

    match(state, TOK_RBLOCK);

    printf("[EXIT ] array_literal\n");
}

// ---------------------------------------------------
// Parses the smallest atomic expression unit (literal, identifier, call, grouping, array literal/access, field access)
//Pre: next token begins a primary expression Post: tokens for one primary expression are consumed (or recovery occurs)
//------------------------------------------------------
void primary_expression(ParState *state)
{
    const TokenType *saved_sync;
    int start_index;

    printf("[ENTER] primary_expression\n");

    start_index = state->index;

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_primary_expression;

    switch (state->next)
    {
        case TOK_INT_LIT:
            match(state, TOK_INT_LIT);
            break;

        case TOK_FLOAT_LIT:
            match(state, TOK_FLOAT_LIT);
            break;

        case TOK_STRING_LIT:
            match(state, TOK_STRING_LIT);
            break;

        case TOK_LPAREN:
            match(state, TOK_LPAREN);
            expression(state);
            match(state, TOK_RPAREN);
            break;

        case TOK_LBLOCK:
            /* array literal: < ... > */
            array_literal(state);
            break;

        case TOK_IDENTIFIER:
            if (state->next_next == TOK_LPAREN)
                function_call(state);
            else if (state->next_next == TOK_LBLOCK)
                array_access(state);
            else
                match(state, TOK_IDENTIFIER);
            break;

        case TOK_FALT:
            field_access(state);
            break;

        default:
            syntax_error_at(state, "expected primary expression");
            sync_to_follow(state);

            /* If sync stops immediately (no progress), force one token forward */
            if (state->index == start_index)
                next_token(state);
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



