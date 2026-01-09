#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>


#include "lexer.h"
#include "utf_decoder.h"

char * reader(FILE * file);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    //Read file into memory from commandline arguments
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
 
    char * char_buffer = reader(file);
    fclose(file);

    //Test it out!
    int c = 0;
    while(char_buffer[c] != '\0'){
        printf("%c", char_buffer[c++]);
    }

    int char_count = 0;
    CharacterUnit * decode_buffer = decode_utf8(char_buffer, &char_count);


    //Invoke the lexer
    //Pre:  Decoded buffer of src code chars
    //Post: Lexeme buffer 
    
    //char ** lexeme_buffer = lexer(decode_buffer);



    return 0;
}

char *reader(FILE *file)
{
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *char_buffer = malloc(length + 1);
    if (!char_buffer) return NULL;

    size_t i = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        char_buffer[i++] = (char)c;
        //printf("Current byte: %c\n", char_buffer[i-1]);
    }

    char_buffer[i] = '\0';
    return char_buffer;
}

