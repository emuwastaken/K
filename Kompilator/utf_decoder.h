

typedef struct {
    int  codepoint;     // Unicode code point (e.g. U+00E4 for ä)
    int  byte_length;   // How many UTF-8 bytes were consumed (1–4)
} CharacterUnit;

CharacterUnit * decode_utf8(const char * byte_buffer, int * out_char_count);