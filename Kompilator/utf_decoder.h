#ifndef UTF_DECODER_H
#define UTF_DECODER_H

typedef struct {
    int codepoint;
    int byte_length;
    unsigned char bytes[4];
} CharacterUnit;

CharacterUnit * decode_utf8(const char *byte_buffer, int *out_char_count);
void print_codepoint_utf8(int codepoint);

#endif
