#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "tokenkeytab.h"


/* ---------------------------------------------
   Parser state (LL(2))
--------------------------------------------- */
typedef struct ParState
{
    TokenBuffer *tokens;
    int          token_count;

    int          index;

    TokenType    current;     /* last consumed */
    TokenType    next;        /* lookahead 1 */
    TokenType    next_next;   /* lookahead 2 */

    int          line;
    int          column;

    int          error_count;
    int          panic_mode;

    const TokenType *sync_set;
} ParState;


/* ---------------------------------------------
   Entry point
--------------------------------------------- */
void parser(TokenBuffer *token_stream,
            int token_count,
            char **lexeme_stream,
            int *out_error_count);

void init_parser(ParState *state,
                 TokenBuffer *token_stream,
                 int token_count);


/* ---------------------------------------------
   Program structure
--------------------------------------------- */
void program(ParState *state);
void global_statement_list(ParState *state);
void global_statement(ParState *state);


/* ---------------------------------------------
   Declarations
--------------------------------------------- */
void function_declaration(ParState *state);
void declaration_statement(ParState *state);
void type_declaration(ParState *state);

void typedef_declaration(ParState *state);
void struct_declaration(ParState *state);

void type_specifier(ParState *state);

void parameter_list(ParState *state);
void parameter(ParState *state);


/* ---------------------------------------------
   Blocks and statements
--------------------------------------------- */
void block(ParState *state);
void statement_list(ParState *state);
void statement(ParState *state);


/* ---------------------------------------------
   Statement variants
--------------------------------------------- */
void assignment_statement(ParState *state);
void assignment_core(ParState *state);

void return_statement(ParState *state);
void break_statement(ParState *state);

void expression_statement(ParState *state);
void field_statement(ParState *state);

void if_statement(ParState *state);
void switch_statement(ParState *state);
void loop_statement(ParState *state);


/* ---------------------------------------------
   Loop variants
--------------------------------------------- */
void while_statement(ParState *state);
void do_while_statement(ParState *state);
void for_statement(ParState *state);


/* ---------------------------------------------
   Expressions
--------------------------------------------- */
void expression(ParState *state);
void logical_expression(ParState *state);
void relational_expression(ParState *state);
void additive_expression(ParState *state);
void multiplicative_expression(ParState *state);
void unary_expression(ParState *state);
void primary_expression(ParState *state);


/* ---------------------------------------------
   Lvalues and access
--------------------------------------------- */
void lvalue(ParState *state);
void array_access(ParState *state);
void field_access(ParState *state);


/* ---------------------------------------------
   Function calls
--------------------------------------------- */
void function_call(ParState *state);
void argument_list(ParState *state);
void function_call_statement(ParState * state);

/* ---------------------------------------------
   Parser utilities
--------------------------------------------- */
void match(ParState *state, TokenType expected);
void next_token(ParState *state);
TokenType peek_token(ParState *state, int offset);
void sync_to_follow(ParState *state);
void initializer(ParState *state);


#endif /* PARSER_H */
