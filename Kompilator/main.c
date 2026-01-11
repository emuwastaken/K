#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "lexer.h"
#include "utf_decoder.h"
#include "tokenkeytab.h"

char *reader(FILE *file);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    if (argc < 2) {
        fprintf(stderr, "Error: No file specified\n");
        return 1;
    }

    const char *filename = argv[1];
    size_t len = strlen(filename);
    if (len < 2 || filename[len - 2] != '.' || filename[len - 1] != 'k') {
        fprintf(stderr, "Error: File must end with .k extension\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file\n");
        return 1;
    }

    char *char_buffer = reader(file);
    fclose(file);

    if (!char_buffer) {
        fprintf(stderr, "Error: Failed to read file\n");
        return 1;
    }

    printf("%s\n", char_buffer);

    int char_count = 0;
    char local_char[4];
    CharacterUnit *decode_buffer = decode_utf8(char_buffer, &char_count);

    /*
    printf("Current UTF-8 Codes for file contents:\n");
    for (int i = 0; i < char_count; i++) {
        printf("Char %d: U+%04X (%c)\n", i, decode_buffer[i].codepoint, 
               (decode_buffer[i].codepoint < 128) ? (char)decode_buffer[i].codepoint : '?');
    } */



    TokenBuffer *token_buffer = NULL;
    int token_amount = 0;

    lexer(decode_buffer, char_count, &token_buffer, &token_amount);

    free(token_buffer);
    free(char_buffer);
    free(decode_buffer);

    return 0;
}



