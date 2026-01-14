#ifndef TOKENKEYTAB_H
#define TOKENKEYTAB_H

typedef enum TokenType {

    // Structure
    TOK_PROGRAM = 200,
    TOK_FORFATTARE,
    //TOK_ENTRE,

    // Types
    TOK_HEL,
    TOK_FLYT,
    TOK_BOK,
    TOK_BIT,
    TOK_HALV,
    TOK_BYTE,
    TOK_ORD,
    TOK_VAL,
    TOK_STRUKTUR,
    TOK_FALT,
    TOK_TOM,
    TOK_TYPDEF,

    // Qualifiers
    TOK_PEK,
    TOK_KONSTANT,
    TOK_STATISK,

    // Selection / repetition
    TOK_OM,
    TOK_ANNARS,
    TOK_MEDAN,
    TOK_GOR,
    TOK_FOR,
    TOK_VAXEL,
    TOK_FALL,
    TOK_BRYT,
    TOK_FORTSATT,

    // Comparison
    TOK_EQ,
    TOK_NEQ,
    TOK_LT,
    TOK_GT,
    TOK_LTE,
    TOK_GTE,

    // Boolean
    TOK_OCH,
    TOK_ELLER,
    TOK_INTE,

    // Flow
    TOK_ATERVAND,

    // Operators
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_EXP,
    TOK_DOT,

    TOK_OKAR,
    TOK_MINSKAR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,

    TOK_DEREF,
    TOK_ADDRESS,
    TOK_STORLEKAV,

    // Delimiters
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBLOCK,
    TOK_RBLOCK,
    TOK_SEMI,
    TOK_COMMA,

    // Literals
    TOK_IDENTIFIER,
    TOK_INT_LIT,
    TOK_FLOAT_LIT,
    TOK_STRING_LIT,

    // Misc
    TOK_EOF,
    TOK_ERROR

} TokenType;

typedef struct Keyword {
    const char *lexeme;
    TokenType token;
} Keyword;

extern const Keyword keywords[];

TokenType lookup_pair(char *lexeme, char *lextwo);
TokenType lookup(char *lexeme);
const char *tok2name(TokenType tok);
const char *tok2lexeme(TokenType tok);
static int is_string_literal(const char *lexeme);
static int is_number_literal(const char *lexeme);
#endif
