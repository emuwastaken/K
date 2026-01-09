#include "lexer.h"
#include "tokenkeytab.h"

#define MAXLEXSIZE 128

char ** lexer(char * char_buffer){
    char ** lexeme_buffer;
    printf("\nLexer entered\n");

    int lex_index       = 0;
    int lex_lookahead   = 0;
    int lex_next_byte;

    int lexeme_count    = 0;
    int lexeme_tot_len  = 0;

    /*
        Keywords and identifiers can be more than 1 character    
        Numbers can be more than 1 characters
        Delimiters cannot be more than 1 character

        To detect characters and place interval bounds use ASCII code

    */

    //Prepare a valid lookahead index
    if(char_buffer[0] !='\0' && char_buffer[1] != '\0'){
            lex_lookahead = 1;
    }

    while(char_buffer[lex_index] != '\0')
    {
        //if()



    }

    return lexeme_buffer;
}

