#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "utf_decoder.h"
#include "tokenkeytab.h"

char *reader(FILE *file);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    // ------------------------------
    // Validate input
    // ------------------------------
    if (argc < 2) {
        fprintf(stderr, "Error: No file specified\n");
        return 1;
    }

    const char *filename = argv[1];
    size_t len = strlen(filename);
    if (len < 2 || filename[len-2] != '.' || filename[len-1] != 'k') {
        fprintf(stderr, "Error: File must end with .k extension\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file\n");
        return 1;
    }

    // ------------------------------
    // Read file
    // ------------------------------
    char *char_buffer = reader(file);
    fclose(file);

    if (!char_buffer) {
        fprintf(stderr, "Error: Failed to read file\n");
        return 1;
    }

    // Echo source
    printf("%s\n", char_buffer);

    // ------------------------------
    // UTF-8 decode
    // ------------------------------
    int char_count = 0;
    CharacterUnit *decode_buffer = decode_utf8(char_buffer, &char_count);

    // ------------------------------
    // Lexing
    // ------------------------------
    TokenType *token_buffer = NULL;
    int token_amount = 0;

    lexer(decode_buffer, char_count, &token_buffer, &token_amount);

    // ------------------------------
    // Output
    // ------------------------------
    printf("\nOutput Lexer:\n");

    for (int i = 0; i < token_amount; i++)
    {
        printf(
            "%-18s, %-15s\n",
            tok2name(token_buffer[i]),
            tok2lexeme(token_buffer[i])
        );
    }

    // ------------------------------
    // Debug unknown tokens
    // ------------------------------
    printf("\nRemaining to fix:\n");

    for (int i = 0; i < token_amount; i++)
    {
        if (token_buffer[i] == TOK_ERROR)
        {
            printf(
                "%-18s, %-15s\n",
                tok2name(token_buffer[i]),
                tok2lexeme(token_buffer[i])
            );
        }
    }

    // ------------------------------
    // Cleanup
    // ------------------------------
    free(char_buffer);
    free(decode_buffer);
    free(token_buffer);

    return 0;
}

// --------------------------------
// File reader
// --------------------------------
char *reader(FILE *file)
{
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *char_buffer = malloc(length + 1);
    if (!char_buffer)
        return NULL;

    size_t i = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
    {
        char_buffer[i++] = (char)c;
    }

    char_buffer[i] = '\0';
    return char_buffer;
}
