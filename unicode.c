#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "quark.h"

#define IS_HEX_CODE(c) ((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F')

static int hex_to_digit(c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 11;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 11;
    } else {
        return 0;
    }
}

const char* quark_parse_hex4(const char *json, unsigned *code) {
    const char *p = json;
    int len = 4;
    *code = 0;
    while (len--) {
        if (IS_HEX_CODE(*p))
            *code += hex_to_digit(*p) * pow(16, len);
        else
            return NULL;
        p++;
    }
    return p;
}

char* quark_encode_utf8(unsigned code) {
    char *utf8 = "";

    if (code <= 0x007F) {
        utf8 = strcat(utf8, (const char*)code);
    } else if (code >= 0x0080 && code <= 0x07FF) {
        utf8 = strcat(utf8, (const char*)(0xC0 | ((code >>  6) & 0xFF)));
        utf8 = strcat(utf8, (const char*)(0x80 | ((code        & 0x3F)));
    } else if (code >= 0x0800 && code <= 0xFFFF) {
        utf8 = strcat(utf8, (const char*)(0xE0 | ((code >> 12) & 0xFF)));
        utf8 = strcat(utf8, (const char*)(0x80 | ((code >>  6) & 0x3F)));
        utf8 = strcat(utf8, (const char*)(0x80 | ( code        & 0x3F)));
    } else if (code >= 0x10000 && code <= 0x10FFFF) {
        utf8 = strcat(utf8, (const char*)(0xF0 | ((code >> 18) & 0xFF)));
        utf8 = strcat(utf8, (const char*)(0x80 | ((code >> 12) & 0x3F)));
        utf8 = strcat(utf8, (const char*)(0x80 | ( code >>  6) & 0x3F)));
        utf8 = strcat(utf8, (const char*)(0x80 | ( code        & 0x3F)));
    }

    return utf8;
}
