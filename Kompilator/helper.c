
#include "helper.h"
#include <stdlib.h>

#define DEBUG 1
void printd(char * str){
    if(DEBUG){
        printf("%s\n", str);
    }
}

char *reader(FILE *file)
{
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *char_buffer = malloc(length + 1);
    if (!char_buffer)
        return NULL;

    int i = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
        char_buffer[i++] = (char)c;

    char_buffer[i] = '\0';
    return char_buffer;
}