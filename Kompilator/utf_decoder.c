
#include <stdlib.h>
#include <stdio.h>

#include "utf_decoder.h"

CharacterUnit * decode_utf8(const char * byte_buffer, int * out_char_count)
{
    int byte_index = 0;
    int char_index = 0;
    int capacity   = 64;

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

        // grow buffer if needed
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

        // store decoded unit
        char_units[char_index].codepoint   = codepoint;
        char_units[char_index].byte_length = byte_len;

        // NEW: copy original UTF-8 bytes
        for (int i = 0; i < byte_len; i++)
        {
            char_units[char_index].bytes[i] =
                (unsigned char) byte_buffer[byte_index + i];
        }

        char_index++;
        byte_index += byte_len;
    }

    *out_char_count = char_index;
    return char_units;
}


void print_codepoint_utf8(int codepoint)
{
    if (codepoint <= 0x7F)
    {
        putchar(codepoint);
    }
    else if (codepoint <= 0x7FF)
    {
        putchar(0xC0 | (codepoint >> 6));
        putchar(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0xFFFF)
    {
        putchar(0xE0 | (codepoint >> 12));
        putchar(0x80 | ((codepoint >> 6) & 0x3F));
        putchar(0x80 | (codepoint & 0x3F));
    }
    else
    {
        putchar(0xF0 | (codepoint >> 18));
        putchar(0x80 | ((codepoint >> 12) & 0x3F));
        putchar(0x80 | ((codepoint >> 6) & 0x3F));
        putchar(0x80 | (codepoint & 0x3F));
    }
}


