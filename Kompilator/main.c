#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "lexer.h"
#include "parser.h"
#include "utf_decoder.h"
#include "tokenkeytab.h"

char *reader(FILE *file);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    if (argc < 2)
    {
        fprintf(stderr, "Error: No file specified\n");
        return 1;
    }

    const char *filename = argv[1];
    size_t len = strlen(filename);
    if (len < 2 || filename[len - 2] != '.' || filename[len - 1] != 'k')
    {
        fprintf(stderr, "Error: File must end with .k extension\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open file\n");
        return 1;
    }

    char *char_buffer = reader(file);
    fclose(file);

    if (!char_buffer)
    {
        fprintf(stderr, "Error: Failed to read file\n");
        return 1;
    }

    printf("%s\n", char_buffer);

    int char_count = 0;
    CharacterUnit *decode_buffer = decode_utf8(char_buffer, &char_count);

    /* -----------------------------
       Outputs from lexer
       ----------------------------- */

    TokenBuffer *token_buffer = NULL;
    int token_count = 0;

    char **lexemes = NULL;
    int lexeme_count = 0;
    int *lexeme_row = NULL;
    int *lexeme_col = NULL;

    /* -----------------------------
       Run lexer
       ----------------------------- */

    lexer(
        decode_buffer,
        char_count,

        &token_buffer,
        &token_count,

        &lexemes,
        &lexeme_count,
        &lexeme_row,
        &lexeme_col
    );
    
   


    for (int i = 0; i < lexeme_count; i++)
    {
        //printf("This breaks dont it\n");
        printf("Lexeme %d: %s \n", i, lexemes[i]);
    }

 /*
    for (int i = 0; i < token_count; i++)
    {
        printf("Token %d: %s\n", i, tok2name(token_buffer[i].token));
    }

    */

    int parse_error_count = 0;

    parser(
        token_buffer,
        token_count,
        lexemes,
        &parse_error_count
    );

    printf("\nParser finished with %d error(s)\n", parse_error_count);


    /* -----------------------------
       Cleanup
       ----------------------------- */

    for (int i = 0; i < lexeme_count; i++)
    {
        free(lexemes[i]);
    }

    free(lexemes);
    free(lexeme_row);
    free(lexeme_col);

    free(token_buffer);
    free(char_buffer);
    free(decode_buffer);

    return 0;
}
