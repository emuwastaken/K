#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "tokenkeytab.h"

 
//Parser state
typedef struct ParState
{
    TokenBuffer *tokens;
    int         token_count;

    int         index;
    TokenType   current;
    TokenType   lookahead;

    int         line;
    int         column;

    int         error_count;
    int         panic_mode;

    const TokenType *sync_set;
} ParState;
 
//Entry point
 
void parser(TokenBuffer *token_stream, char **lexeme_stream, int * out_error_count);

 
//Program structure

void program(ParState *ps);
void global_statement_list(ParState *ps);
void global_statement(ParState *ps);

 
//Declarations
void function_declaration(ParState *ps);
void declaration_statement(ParState *ps);
void type_declaration(ParState *ps);

void parameter_list(ParState *ps);
void parameter(ParState *ps);

void type_specifier(ParState *ps);
void identifier_list(ParState *ps);

 
//Blocks and statements
 
void block(ParState *ps);
void statement_list(ParState *ps);
void statement(ParState *ps);

 
// variants
void assignment_statement(ParState *ps);
void return_statement(ParState *ps);
void expression_statement(ParState *ps);
void field_statement(ParState *ps);

void if_statement(ParState *ps);
void switch_statement(ParState *ps);
void loop_statement(ParState *ps);

 
//Loop variants
void while_statement(ParState *ps);
void do_while_statement(ParState *ps);
void for_statement(ParState *ps);


//Expressions

void expression(ParState *ps);
void logical_expression(ParState *ps);
void relational_expression(ParState *ps);
void additive_expression(ParState *ps);
void multiplicative_expression(ParState *ps);
void unary_expression(ParState *ps);
void primary_expression(ParState *ps);


//Function calls and arguments

void function_call(ParState *ps);
void argument_list(ParState *ps);


//Parser utilities
void match(ParState *ps, TokenType expected);
void next_token(ParState *ps);

#endif
