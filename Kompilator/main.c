#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <wchar.h>

#include "lexer.h"

#define INPUT_BUFFER_SIZE (1024 * 1024)

// ---------------------------------------------------
// Entry point
// Pre: input provided via PowerShell pipeline
// Post: tokens printed to stdout
// ---------------------------------------------------
int main(void)
{
    wchar_t *buffer;
    size_t len;
    Token tok;

    // Tell Windows CRT that stdin is UTF-16
    _setmode(_fileno(stdin), _O_U16TEXT);

    // allocate wide-character buffer
    buffer = (wchar_t*)malloc(sizeof(wchar_t) * INPUT_BUFFER_SIZE);
    if (!buffer) {
        fwprintf(stderr, L"Failed to allocate input buffer\n");
        return 1;
    }

    // read entire stdin as UTF-16
    len = fgetws(buffer, INPUT_BUFFER_SIZE, stdin) ? wcslen(buffer) : 0;
    buffer[len] = L'\0';

    // initialize lexer with UTF-16 input
    lexer_init(buffer);

    // tokenize and print
    do {
        tok = next_token();

        printf(
            "TOKEN %-10d | %-15s | line %d, col %d\n",
            tok.type,
            tok.lexeme,
            tok.line,
            tok.column
        );

    } while (tok.type != TOK_EOF);

    free(buffer);
    return 0;
}
