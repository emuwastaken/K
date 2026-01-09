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

    char * curr_lexeme[MAXLEXSIZE];

    while(char_buffer[lex_index] != '\0')
    {
    
        /*Algorithm:
        Build a base keyword e.g HEL, STRUKTUR, STORLEK, etc.
        Skip whitespaces, newlines, tabular space
        Automatically accept :, ;, ,, (, ), <, >,
        */
        if(char_buffer[lex_index] == ' '){
            lex_index++;
            continue;
        } else if(  (char_buffer[lex_index] >= 'a'   &&
                    char_buffer[lex_index] <= 'Z')   ||
                    char_buffer[lex_index] == 'å'   ||
                    char_buffer[lex_index] == 'ä'   ||
                    char_buffer[lex_index] == 'ö'   ||
                    char_buffer[lex_index] == 'Å'   ||
                    char_buffer[lex_index] == 'Ä'   ||
                    char_buffer[lex_index] == 'Ö'                                                                                                       
        ){
            identify_key(char_buffer + lex_index, &lex_index, &curr_lexeme);    
        } else if() {

        }
    }

    return lexeme_buffer;
}

void identify_key (char * char_buffer, int * index_address, char * lexeme){

}

