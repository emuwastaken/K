
#include <stdlib.h>
#include "utf_decoder.h"

CharacterUnit * decode_utf8(const char * byte_buffer, int * out_char_count)
{
    int byte_index = 0;
    int char_index = 0;
    int capacity   = 64;

    //Start with 64 units
    CharacterUnit * char_units = malloc(sizeof(CharacterUnit) * capacity);
    if (!char_units) return NULL;

    while (byte_buffer[byte_index] != '\0')
    {
        unsigned char b0 = (unsigned char) byte_buffer[byte_index];
        int codepoint = 0;
        int byte_len  = 0;

        // 1-byte ASCII
        if ((b0 & 0x80) == 0x00)
        {
            codepoint = b0;
            byte_len = 1;
        }
        // 2-byte UTF-8
        else if ((b0 & 0xE0) == 0xC0)
        {
            unsigned char b1 = (unsigned char) byte_buffer[byte_index + 1];
            if ((b1 & 0xC0) != 0x80)
            {
                byte_index++;
                continue;
            }
            codepoint = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
            byte_len = 2;
        }
        // 3-byte UTF-8
        else if ((b0 & 0xF0) == 0xE0)
        {
            unsigned char b1 = (unsigned char) byte_buffer[byte_index + 1];
            unsigned char b2 = (unsigned char) byte_buffer[byte_index + 2];
            if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80)
            {
                byte_index++;
                continue;
            }
            codepoint = ((b0 & 0x0F) << 12) |
                        ((b1 & 0x3F) << 6)  |
                        (b2 & 0x3F);
            byte_len = 3;
        }
        // 4-byte UTF-8
        else if ((b0 & 0xF8) == 0xF0)
        {
            unsigned char b1 = (unsigned char) byte_buffer[byte_index + 1];
            unsigned char b2 = (unsigned char) byte_buffer[byte_index + 2];
            unsigned char b3 = (unsigned char) byte_buffer[byte_index + 3];
            if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80)
            {
                byte_index++;
                continue;
            }
            codepoint = ((b0 & 0x07) << 18) |
                        ((b1 & 0x3F) << 12) |
                        ((b2 & 0x3F) << 6)  |
                        (b3 & 0x3F);
            byte_len = 4;
        }
        else
        {
            byte_index++;
            continue;
        }


        //If capacity is exceeded dynamically reallocate 
        if (char_index >= capacity)
        {
            capacity *= 2;
            CharacterUnit * tmp = realloc(char_units, sizeof(CharacterUnit) * capacity);
            if (!tmp)
            {
                free(char_units);
                return NULL;
            }
            char_units = tmp;
        }

        char_units[char_index].codepoint   = codepoint;
        char_units[char_index].byte_length = byte_len;

        char_index++;
        byte_index += byte_len;
    }

    //Return the correct values!
    *out_char_count = char_index;
    return char_units;
}

