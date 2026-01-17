#include "parser.h"
#include "lexer.h"
#include "tokenkeytab.h"
#include <stdio.h>


/*
  _____            _                 _   _                 
 |  __ \          | |               | | (_)                
 | |  | | ___  ___| | __ _ _ __ __ _| |_ _  ___  _ __  ___ 
 | |  | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
 | |__| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
 |_____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/
                                                           
*/
void enum_declaration(ParState *state);
static int is_function_declaration_start(ParState *state);
static int is_declaration_statement_start(ParState *state);
static void goto_statement(ParState *state);
static void label_statement(ParState *state);
void bitwise_or_expression(ParState *state);
void bitwise_xor_expression(ParState *state);
void bitwise_and_expression(ParState *state);
void equality_expression(ParState *state);
void shift_expression(ParState *state);
static void shift_operator(ParState *state);

///////////////////////////////////////////








/*
  ______    _ _                  _____      _       
 |  ____|  | | |                / ____|    | |      
 | |__ ___ | | | _____      __ | (___   ___| |_ ___ 
 |  __/ _ \| | |/ _ \ \ /\ / /  \___ \ / _ \ __/ __|
 | | | (_) | | | (_) \ V  V /   ____) |  __/ |_\__ \
 |_|  \___/|_|_|\___/ \_/\_/   |_____/ \___|\__|___/                                                   
*/


static const TokenType FOLLOW_program[] = {
    TOK_EOF,
    TOK_ERROR
};

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

static const TokenType FOLLOW_statement[] = {
    TOK_SEMI,
    TOK_RBLOCK,
    TOK_ANNARS,
    TOK_EOF,
    TOK_ERROR
};

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

static const TokenType FOLLOW_primary_expression[] = {
    /* multiplicative */
    TOK_MUL, TOK_DIV, TOK_MOD, TOK_EXP,

    /* additive */
    TOK_PLUS, TOK_MINUS,

    /* shift */
    TOK_SHIFT,

    /* relational / equality */
    TOK_LT, TOK_LTE, TOK_GT, TOK_GTE, TOK_EQ, TOK_NEQ,

    /* bitwise */
    TOK_BITAND, TOK_BITXOR, TOK_BITOR,

    /* logical */
    TOK_OCH, TOK_ELLER,

    /* statement-context operators after lvalues */
    TOK_ASSIGN,
    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    /* expression terminators / separators */
    TOK_COMMA, TOK_SEMI, TOK_RPAREN, TOK_RBLOCK,

    /* end / error */
    TOK_EOF, TOK_ERROR
};

static const TokenType FOLLOW_additive_expression[] = {
    /* shift operator starter */
    TOK_SHIFT,

    /* relational */
    TOK_LT, TOK_GT, TOK_LTE, TOK_GTE,

    /* equality */
    TOK_EQ, TOK_NEQ,

    /* bitwise */
    TOK_BITAND, TOK_BITXOR, TOK_BITOR,

    /* logical */
    TOK_OCH, TOK_ELLER,

    /* assignment/update lookaheads in statement context */
    TOK_ASSIGN,
    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    /* expression terminators / separators */
    TOK_SEMI, TOK_RPAREN, TOK_COMMA, TOK_RBLOCK,

    /* end / error */
    TOK_EOF, TOK_ERROR
};

static const TokenType FOLLOW_relational_expression[] = {
    /* equality */
    TOK_EQ, TOK_NEQ,

    /* bitwise */
    TOK_BITAND, TOK_BITXOR, TOK_BITOR,

    /* logical */
    TOK_OCH, TOK_ELLER,

    /* assignment/update lookaheads in statement context */
    TOK_ASSIGN,
    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    /* expression terminators / separators */
    TOK_SEMI, TOK_RPAREN, TOK_COMMA, TOK_RBLOCK,

    /* end / error */
    TOK_EOF, TOK_ERROR
};

static const TokenType FOLLOW_equality_expression[] = {
    /* bitwise */
    TOK_BITAND, TOK_BITXOR, TOK_BITOR,

    /* logical */
    TOK_OCH, TOK_ELLER,

    /* assignment/update lookaheads in statement context */
    TOK_ASSIGN,
    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    /* expression terminators / separators */
    TOK_SEMI, TOK_RPAREN, TOK_COMMA, TOK_RBLOCK,

    /* end / error */
    TOK_EOF, TOK_ERROR
};

static const TokenType FOLLOW_bitwise_and_expression[] = {
    TOK_BITXOR,
    TOK_BITOR,
    TOK_OCH,
    TOK_ELLER,

    TOK_ASSIGN,
    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    TOK_SEMI, TOK_RPAREN, TOK_COMMA, TOK_RBLOCK,
    TOK_EOF, TOK_ERROR
};

static const TokenType FOLLOW_bitwise_xor_expression[] = {
    TOK_BITOR,
    TOK_OCH,
    TOK_ELLER,

    TOK_ASSIGN,
    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,

    TOK_SEMI, TOK_RPAREN, TOK_COMMA, TOK_RBLOCK,
    TOK_EOF, TOK_ERROR
};

static const TokenType FOLLOW_unary_expression[] = {
    /* multiplicative */
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,

    /* additive */
    TOK_PLUS,
    TOK_MINUS,

    /* shift */
    TOK_SHIFT,

    /* relational / equality */
    TOK_LT,
    TOK_GT,
    TOK_LTE,
    TOK_GTE,
    TOK_EQ,
    TOK_NEQ,

    /* bitwise */
    TOK_BITAND,
    TOK_BITXOR,
    TOK_BITOR,

    /* logical */
    TOK_OCH,
    TOK_ELLER,

    /* expression terminators */
    TOK_COMMA,
    TOK_SEMI,
    TOK_RPAREN,
    TOK_RBLOCK,
    TOK_EOF,
    TOK_ERROR
};

////////////////////////////////////////////////////////////////







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



/*
  _    _      _                     
 | |  | |    | |                    
 | |__| | ___| |_ __   ___ _ __ ___ 
 |  __  |/ _ \ | '_ \ / _ \ '__/ __|
 | |  | |  __/ | |_) |  __/ |  \__ \
 |_|  |_|\___|_| .__/ \___|_|  |___/
               | |                  
               |_|                  
*/

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

    /* advance token to prevent infinite loops on missing tokens */
    next_token(state);

    return;
}

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

static int is_function_declaration_start(ParState *state)
{
    int i;
    int depth;

    //handles optional EXTERN
    i = state->index;
    if (i < state->token_count && state->tokens[i].token == TOK_EXTERN)
        i++;

    //rejects if there is no token left
    if (i >= state->token_count)
        return 0;

    //consumes base type of the type_specifier
    switch (state->tokens[i].token)
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
        case TOK_IDENTIFIER:
            i++;
            break;

        case TOK_STRUKTUR:
            //requires: STRUKTUR <identifier>
            i++;
            if (i >= state->token_count || state->tokens[i].token != TOK_IDENTIFIER)
                return 0;
            i++;
            break;

        default:
            return 0;
    }

    //consumes pointer suffixes: PEK*
    while (i < state->token_count && state->tokens[i].token == TOK_PEK)
        i++;

    //consumes array suffixes on the type: < ... > (balanced)
    while (i < state->token_count && state->tokens[i].token == TOK_LBLOCK)
    {
        //tracks nested < > in the dimension expression
        depth = 0;
        do
        {
            if (state->tokens[i].token == TOK_LBLOCK)
                depth++;
            else if (state->tokens[i].token == TOK_RBLOCK)
                depth--;

            i++;

            //unterminated dimension
            if (i >= state->token_count && depth > 0)
                return 0;
        }
        while (depth > 0);
    }

    //expects ':' after the type
    if (i >= state->token_count || state->tokens[i].token != TOK_ASSIGN)
        return 0;
    i++;

    //expects function name
    if (i >= state->token_count || state->tokens[i].token != TOK_IDENTIFIER)
        return 0;
    i++;

    //expects '(' to confirm a function declaration
    if (i >= state->token_count)
        return 0;

    return (state->tokens[i].token == TOK_LPAREN);
}

static int is_declaration_statement_start(ParState *state)
{
    int i;
    int depth;

    //handles optional EXTERN
    i = state->index;
    if (i < state->token_count && state->tokens[i].token == TOK_EXTERN)
        i++;

    //rejects if there is no token left
    if (i >= state->token_count)
        return 0;

    //consumes base type of the type_specifier
    switch (state->tokens[i].token)
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
        case TOK_IDENTIFIER:
            i++;
            break;

        case TOK_STRUKTUR:
            //requires: STRUKTUR <identifier>
            i++;
            if (i >= state->token_count || state->tokens[i].token != TOK_IDENTIFIER)
                return 0;
            i++;
            break;

        default:
            return 0;
    }

    //consumes pointer suffixes: PEK*
    while (i < state->token_count && state->tokens[i].token == TOK_PEK)
        i++;

    //consumes array suffixes on the type: < ... > (balanced)
    while (i < state->token_count && state->tokens[i].token == TOK_LBLOCK)
    {
        //tracks nested < > in the dimension expression
        depth = 0;
        do
        {
            if (state->tokens[i].token == TOK_LBLOCK)
                depth++;
            else if (state->tokens[i].token == TOK_RBLOCK)
                depth--;

            i++;

            //unterminated dimension
            if (i >= state->token_count && depth > 0)
                return 0;
        }
        while (depth > 0);
    }

    //expects ':' after the type
    if (i >= state->token_count || state->tokens[i].token != TOK_ASSIGN)
        return 0;
    i++;

    //expects variable name
    if (i >= state->token_count || state->tokens[i].token != TOK_IDENTIFIER)
        return 0;
    i++;

    //rejects function declarations
    if (i < state->token_count && state->tokens[i].token == TOK_LPAREN)
        return 0;

    //accepts common declaration continuations
    if (i >= state->token_count)
        return 0;

    return (state->tokens[i].token == TOK_SEMI ||
            state->tokens[i].token == TOK_COMMA ||
            state->tokens[i].token == TOK_LBLOCK);
}



/*
  _   _               _______                  _             _     
 | \ | |             |__   __|                (_)           | |    
 |  \| | ___  _ __      | | ___ _ __ _ __ ___  _ _ __   __ _| |___ 
 | . ` |/ _ \| '_ \     | |/ _ \ '__| '_ ` _ \| | '_ \ / _` | / __|
 | |\  | (_) | | | |    | |  __/ |  | | | | | | | | | | (_| | \__ \
 |_| \_|\___/|_| |_|    |_|\___|_|  |_| |_| |_|_|_| |_|\__,_|_|___/
                                                                   
*/

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

    //routes by first token
    switch (state->next)
    {
        case TOK_TYPDEF:
        case TOK_STRUKTUR:
        case TOK_ENUM:
            //type declarations at global scope
            type_declaration(state);
            break;

        default:
            if (is_function_declaration_start(state))
            {
                function_declaration(state);
            }
            else if (is_declaration_statement_start(state))
            {
                declaration_statement(state);
            }
            else
            {
                syntax_error_at(state, "unexpected token in global scope");
                //recovery
                sync_to_follow(state);
            }
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] global_statement\n");
}

void function_declaration(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] function_declaration\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_global_statement;

    //consumes optional EXTERN storage specifier
    if (state->next == TOK_EXTERN)
        match(state, TOK_EXTERN);

    //parses return type
    type_specifier(state);

    //enforces ':' between type and identifier
    if (state->next == TOK_ASSIGN)
    {
        match(state, TOK_ASSIGN);
    }
    else
    {
        //treat ':' as inserted and continue
        syntax_error_at(state, "expected ':' after function return type");
    }

    //parses function name
    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);
    }
    else
    {
        syntax_error_at(state, "expected function name");

        //recovers to global statement boundary
        sync_to_follow(state);
        state->sync_set = saved_sync;

        printf("[EXIT ] function_declaration\n");
        return;
    }

    //parses '('
    match(state, TOK_LPAREN);

    //parses optional parameter list
    parameter_list(state);

    //parses ')'
    match(state, TOK_RPAREN);

    //parses function body
    block(state);

    state->sync_set = saved_sync;

    printf("[EXIT ] function_declaration\n");
}

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

        case TOK_ENUM:
            enum_declaration(state);
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

void enum_declaration(ParState *state)
{
    printf("[ENTER] enum_declaration\n");

    match(state, TOK_ENUM);

    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected enum name after ENUM");
        sync_to_follow(state);
        printf("[EXIT ] enum_declaration\n");
        return;
    }

    match(state, TOK_IDENTIFIER);

    match(state, TOK_LBLOCK);

    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected enumerator inside ENUM < ... >");
        sync_to_follow(state);
        printf("[EXIT ] enum_declaration\n");
        return;
    }

    while (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);

        // optional explicit value: NAME : <expr>
        if (state->next == TOK_ASSIGN)
        {
            match(state, TOK_ASSIGN);
            expression(state);
        }

        if (state->next == TOK_COMMA)
        {
            match(state, TOK_COMMA);
            continue;
        }

        break;
    }

    match(state, TOK_RBLOCK);

    // optional trailing ';' if you want it
    if (state->next == TOK_SEMI)
        match(state, TOK_SEMI);

    printf("[EXIT ] enum_declaration\n");
}

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

static void update_statement(ParState *state)
{
    int lvalue_start_index;

    //tracks progress so we can bail out cleanly on lvalue failure
    lvalue_start_index = state->index;

    //parses the lvalue portion (supports VÄRDE VID, FÄLT, arrays, identifiers)
    if (state->next == TOK_DEREF || state->next == TOK_FALT || state->next == TOK_IDENTIFIER)
    {
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

    //handles compound shift-updates
    //x SKIFT VÄNSTER MED 3;
    //x SKIFT HÖGER MED 1;
    if (state->next == TOK_SHIFT)
    {
        match(state, TOK_SHIFT);

        //expects the combined tokens VÄNSTER MED / HÖGER MED
        if (state->next != TOK_SHL_ASSIGN && state->next != TOK_SHR_ASSIGN)
        {
            syntax_error_at(state, "expected VÄNSTER MED or HÖGER MED after SKIFT");
            sync_to_follow(state);
            return;
        }

        //consumes TOK_SHL_ASSIGN or TOK_SHR_ASSIGN
        next_token(state);

        //parses shift amount
        expression(state);

        //ends the statement
        match(state, TOK_SEMI);
        return;
    }

    //handles postfix increment (ÖKAR)
    if (state->next == TOK_OKAR)
    {
        match(state, TOK_OKAR);
        match(state, TOK_SEMI);
        return;
    }

    //handles postfix decrement (MINSKAR)
    if (state->next == TOK_MINSKAR)
    {
        match(state, TOK_MINSKAR);
        match(state, TOK_SEMI);
        return;
    }

    //handles plus-assign (ÖKAR MED)
    if (state->next == TOK_PLUS_ASSIGN)
    {
        match(state, TOK_PLUS_ASSIGN);
        expression(state);
        match(state, TOK_SEMI);
        return;
    }

    //handles minus-assign (MINSKAR MED)
    if (state->next == TOK_MINUS_ASSIGN)
    {
        match(state, TOK_MINUS_ASSIGN);
        expression(state);
        match(state, TOK_SEMI);
        return;
    }

    //handles mul-assign (MULT MED)
    if (state->next == TOK_MUL_ASSIGN)
    {
        match(state, TOK_MUL_ASSIGN);
        expression(state);
        match(state, TOK_SEMI);
        return;
    }

    //handles div-assign (DELAS MED)
    if (state->next == TOK_DIV_ASSIGN)
    {
        match(state, TOK_DIV_ASSIGN);
        expression(state);
        match(state, TOK_SEMI);
        return;
    }

    syntax_error_at(state, "expected update operator after lvalue");
    sync_to_follow(state);
}

void statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    //dispatch based on the first token
    switch (state->next)
    {
        case TOK_HEL:
        case TOK_FLYT:
        case TOK_BOK:
        case TOK_STRUKTUR:
        case TOK_TOM:
        case TOK_OSIGNERAD:
        case TOK_SIGNERAD:
        case TOK_KORT:
        case TOK_LANG:
        case TOK_DUBBEL:
        case TOK_LANG_DUBBEL:
        case TOK_VOLATIL:
        case TOK_BEGRANSA:
        case TOK_EXTERN:
        case TOK_AUTO:
        case TOK_REGISTER:
            //declaration statements (including EXTERN)
            declaration_statement(state);
            break;

        case TOK_ATERVAND:
            return_statement(state);
            break;

        case TOK_OM:
            if_statement(state);
            break;

        case TOK_FOR:
            for_statement(state);
            break;

        case TOK_MEDAN:
        case TOK_GOR:
            do_while_statement(state);
            break;

        case TOK_VAXEL:
            switch_statement(state);
            break;

        case TOK_BRYT:
            break_statement(state);
            break;

        case TOK_FORTSATT:
            continue_statement(state);
            break;

        case TOK_GOTO:
            goto_statement(state);
            break;

        case TOK_ETIKETT:
            label_statement(state);
            break;

        case TOK_LBLOCK:
            block(state);
            break;

        case TOK_IDENTIFIER:
        case TOK_FALT:
        case TOK_DEREF:
        {
            const TokenType update_ops[] =
            {
                TOK_OKAR,
                TOK_MINSKAR,
                TOK_PLUS_ASSIGN,
                TOK_MINUS_ASSIGN,
                TOK_MUL_ASSIGN,
                TOK_DIV_ASSIGN,
                TOK_SHIFT
            };

            const TokenType assign_ops[] = { TOK_ASSIGN };

            //routes user-defined type declarations (heuristic)
            //TYPE: name, init;
            //Without a symbol table, distinguish from assignment by requiring a comma after the declared name.
            if (state->next == TOK_IDENTIFIER &&
                peek_token(state, 1) == TOK_ASSIGN &&
                peek_token(state, 2) == TOK_IDENTIFIER &&
                peek_token(state, 3) == TOK_COMMA)
            {
                declaration_statement(state);
                break;
            }

            //routes update-style statements (ÖKAR/MINSKAR/… and SKIFT … MED …)
            if (lookahead_contains_any_until_boundary(state, update_ops, 7, 32))
            {
                update_statement(state);
                break;
            }

            //routes assignment-style statements (lvalue ':' expr ';')
            if (lookahead_contains_any_until_boundary(state, assign_ops, 1, 16))
            {
                assignment_statement(state);
                break;
            }

            //fallback: expression statement
            expression_statement(state);
            break;
        }

        default:
            syntax_error_at(state, "unexpected token in statement");
            //recovery
            sync_to_follow(state);
            break;
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] statement\n");
}

static void goto_statement(ParState *state)
{
    printf("[ENTER] goto_statement\n");

    match(state, TOK_GOTO);

    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected label identifier after GÅ TILL");
        sync_to_follow(state);
        printf("[EXIT ] goto_statement\n");
        return;
    }

    match(state, TOK_IDENTIFIER);
    match(state, TOK_SEMI);

    printf("[EXIT ] goto_statement\n");
}

static void label_statement(ParState *state)
{
    printf("[ENTER] label_statement\n");

    match(state, TOK_ETIKETT);

    if (state->next != TOK_IDENTIFIER)
    {
        syntax_error_at(state, "expected label identifier after ETIKETT");
        sync_to_follow(state);
        printf("[EXIT ] label_statement\n");
        return;
    }

    match(state, TOK_IDENTIFIER);
    match(state, TOK_SEMI);

    printf("[EXIT ] label_statement\n");
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

    // deref lvalue: VÄRDE VID <lvalue> | VÄRDE VID (<expression>)
    if (state->next == TOK_DEREF)
    {
        match(state, TOK_DEREF);

        if (state->next == TOK_LPAREN)
        {
            match(state, TOK_LPAREN);
            expression(state);
            match(state, TOK_RPAREN);
            printf("[EXIT ] lvalue\n");
            return;
        }

        // allow chained deref/field/identifier targets
        if (state->next == TOK_DEREF)
        {
            lvalue(state);
            printf("[EXIT ] lvalue\n");
            return;
        }

        if (state->next == TOK_FALT)
        {
            field_access(state);
            printf("[EXIT ] lvalue\n");
            return;
        }

        if (state->next == TOK_IDENTIFIER)
        {
            match(state, TOK_IDENTIFIER);

            // optional array suffix: p<1>
            if (state->next == TOK_LBLOCK)
            {
                match(state, TOK_LBLOCK);
                expression(state);
                match(state, TOK_RBLOCK);
            }

            printf("[EXIT ] lvalue\n");
            return;
        }

        syntax_error_at(state, "expected lvalue after VÄRDE VID");
        sync_to_follow(state);
        printf("[EXIT ] lvalue\n");
        return;
    }

    // field lvalue
    if (state->next == TOK_FALT)
    {
        field_access(state);
        printf("[EXIT ] lvalue\n");
        return;
    }

    // identifier (optionally array)
    if (state->next == TOK_IDENTIFIER)
    {
        match(state, TOK_IDENTIFIER);

        if (state->next == TOK_LBLOCK)
        {
            match(state, TOK_LBLOCK);
            expression(state);
            match(state, TOK_RBLOCK);
        }

        printf("[EXIT ] lvalue\n");
        return;
    }

    syntax_error_at(state, "expected lvalue");
    sync_to_follow(state);
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

    //parses the lower-precedence base (this chain should include == != < > etc inside it)
    bitwise_or_expression(state);

    //parses logical chaining
    while (state->next == TOK_OCH || state->next == TOK_ELLER)
    {
        //consumes OCH/ELLER
        next_token(state);

        //parses next operand
        bitwise_or_expression(state);
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

    /* IMPORTANT: base operand must be shift_expression, not additive_expression
       so expressions like: a SKIFT VÄNSTER 2 parse correctly */
    shift_expression(state);

    while (state->next == TOK_LT  ||
           state->next == TOK_GT  ||
           state->next == TOK_LTE ||
           state->next == TOK_GTE)
    {
        next_token(state);
        shift_expression(state);
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
    state->sync_set = FOLLOW_unary_expression;

    unary_expression(state);

    while (state->next == TOK_MUL ||
           state->next == TOK_DIV ||
           state->next == TOK_MOD)
    {
        next_token(state);
        unary_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] multiplicative_expression\n");
}

static void shift_operator(ParState *state)
{
    match(state, TOK_SHIFT);

    if (state->next != TOK_VANSTER && state->next != TOK_HOGER)
    {
        syntax_error_at(state, "expected VÄNSTER or HÖGER after SKIFT");
        sync_to_follow(state);
        return;
    }

    next_token(state);
}

void shift_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] shift_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_additive_expression;

    additive_expression(state);

    while (state->next == TOK_SHIFT)
    {
        shift_operator(state);
        additive_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] shift_expression\n");
}

void equality_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] equality_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_relational_expression;

    relational_expression(state);

    while (state->next == TOK_EQ || state->next == TOK_NEQ)
    {
        next_token(state);
        relational_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] equality_expression\n");
}

void bitwise_and_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] bitwise_and_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_equality_expression;

    equality_expression(state);

    while (state->next == TOK_BITAND)
    {
        match(state, TOK_BITAND);
        equality_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] bitwise_and_expression\n");
}

void bitwise_xor_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] bitwise_xor_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_bitwise_and_expression;

    bitwise_and_expression(state);

    while (state->next == TOK_BITXOR)
    {
        match(state, TOK_BITXOR);
        bitwise_and_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] bitwise_xor_expression\n");
}

void bitwise_or_expression(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] bitwise_or_expression\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_bitwise_xor_expression;

    bitwise_xor_expression(state);

    while (state->next == TOK_BITOR)
    {
        match(state, TOK_BITOR);
        bitwise_xor_expression(state);
    }

    state->sync_set = saved_sync;

    printf("[EXIT ] bitwise_or_expression\n");
}

void continue_statement(ParState *state)
{
    const TokenType *saved_sync;

    printf("[ENTER] continue_statement\n");

    saved_sync = state->sync_set;
    state->sync_set = FOLLOW_statement;

    match(state, TOK_FORTSATT);
    match(state, TOK_SEMI);

    state->sync_set = saved_sync;

    printf("[EXIT ] continue_statement\n");
}

int is_type_starter(TokenType t)
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

int is_cast_start(ParState *state)
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

int is_type_token(TokenType t)
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

        case TOK_BITNOT:
            //parses bitwise NOT
            match(state, TOK_BITNOT);
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



