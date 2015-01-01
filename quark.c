#include <stdio.h>
#include <assert.h>
#include <stdlib.h> /* relloc */
#include <math.h>
#include <errno.h>
#include <string.h> /* memcpy */
#include <stddef.h> /* size_t */
#include "quark.h"

#define EXPECT(c, ch)\
    do {\
        assert(*c->json == (ch)); c->json++;\
    } while(0)
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_E(c) ((c) == 'e' || (c) == 'E')
#define IS_NOT_E(c) ((c) != 'e' && (c) != 'E')
#define H_SURROGATE_LOWER 0xD834
#define H_SURROGATE_UPPER 0xDBFF
#define L_SURROGATE_LOWER 0xDC00
#define L_SURROGATE_UPPER 0xDFFF
#define IS_H_SURROGATE(code) ((code) > H_SURROGATE_LOWER && (code) < H_SURROGATE_UPPER)
#define IS_L_SURROGATE(code) ((code) > L_SURROGATE_LOWER && (code) < L_SURROGATE_UPPER)
#define PUTC(context, c) *(char *)quark_context_push(context, sizeof(char)) = c;
#define STRING_ERROR(rtn) do { context->stack.top = head; return rtn; } while(0)
static int quark_parse_literal(quark_context *context, quark_node *node, const char *literal, quark_type type) {
    int len = 0;
    const char *json = context->json;

    while (*literal != '\0') {
        if (*json == *literal) {
            len++;
            json++;
            literal++;
        } else {
            return QUARK_PARSE_INVALID_VALUE;
        }
    }

    context->json += len;
    node->type = type;
    return QUARK_PARSE_OK;
}

static int quark_parse_number(quark_context *context, quark_node *node) {
    char *end;
    char c = '\0';
    char last_c = '\0';
    int in_farc = 0;
    int in_exp = 0;
    const char *json = context->json;

    while ((c = *json) != '\0' && c != ' ' && c != '\t' && c != '\n' && c != '\r') {
        if (IS_DIGIT(c)) {
            if (c == '0' && (last_c == '\0' || last_c == '-') && IS_DIGIT(*(json + 1)))
                return QUARK_PARSE_INVALID_VALUE;

        } else {
            switch (c) {
                case '-':
                    if (last_c != '\0' && IS_NOT_E(last_c))
                        return QUARK_PARSE_INVALID_VALUE;
                    break;

                case '+':
                    if (IS_NOT_E(last_c))
                        return QUARK_PARSE_INVALID_VALUE;
                    break;

                case 'e':
                case 'E':
                    if (in_exp || last_c == '\0')
                        return QUARK_PARSE_INVALID_VALUE;
                    else
                        in_exp = 1;
                    break;

                case '.':
                    if (in_farc || last_c == '\0' || !IS_DIGIT(*(json + 1)))
                        return QUARK_PARSE_INVALID_VALUE;
                    else
                        in_farc = 1;
                    break;

                default:
                    return QUARK_PARSE_INVALID_VALUE;
            }
        }

        json++;
        last_c = c;
    }

    errno = 0;
    node->value.n = strtod(context->json, &end);

    if (errno == ERANGE && (node->value.n == HUGE_VAL || node->value.n == -HUGE_VAL))
        return QUARK_PARSE_NUMBER_TOO_BIG;

    node->type = QUARK_NUMBER;
    context->json = end;
    return QUARK_PARSE_OK;
}

static void quark_parse_whitespace(quark_context *context) {
    // printf("cmp: %d\n", context->json == NULL);
    const char *p = context->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    context->json = p;
}

static int quark_parse_string(quark_context *context, quark_node *node) {
    EXPECT(context, '\"');

    char ch;
    size_t len, head = context->stack.top;
    const char *json = context->json;
    unsigned code, l_code;

    while (1) {
        ch = *json++;

        switch (ch) {
            case '\"':
                len = context->stack.top - head;
                quark_set_string(node, (const char*)quark_context_pop(context, len), len);
                context->json = json;
                return QUARK_PARSE_OK;
            case '\\':
                switch (ch = *json++) {
                    case '\"': PUTC(context, '\"'); break;
                    case '\\': PUTC(context, '\\'); break;
                    case '/':  PUTC(context, '/');  break;
                    case 'b':  PUTC(context, '\b'); break;
                    case 'f':  PUTC(context, '\f'); break;
                    case 'n':  PUTC(context, '\n'); break;
                    case 'r':  PUTC(context, '\r'); break;
                    case 't':  PUTC(context, '\t'); break;
                    case 'u':
                        if (!(json = quark_parse_hex4(json, &code)))
                            STRING_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX);

                        // surrogate
                        if (IS_H_SURROGATE(code)) {
                            const char* p = json;
                            if (
                                *++p == '\\' && *++p == 'u' &&
                                quark_parse_hex4(p, &l_code) &&
                                IS_L_SURROGATE(l_code)
                            ) {
                                json = p;
                                code = 0x10000 + (code - H_SURROGATE_LOWER) * 0x400 + (l_code - L_SURROGATE_LOWER);
                            } else {
                                STRING_ERROR(QUARK_PARSE_INVALID_UNICODE_SURROGATE);
                            }
                        }
                        printf("code: %s\n", quark_encode_utf8(code));
                        // *(char *)quark_context_push(context, sizeof(char)) = c;
                        break;
                    default:
                        STRING_ERROR(QUARK_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                STRING_ERROR(QUARK_PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char)ch < 0x20) {
                    STRING_ERROR(QUARK_PARSE_INVALID_STRING_CHAR);
                }
                PUTC(context, ch);
                break;
        }
    }
}

static int quark_parse_value(quark_context *context, quark_node *node) {
    switch (*context->json) {
        case '\0': return QUARK_PARSE_EXPECT_VALUE;
        case '\"': return quark_parse_string(context, node);
        case 'n':  return quark_parse_literal(context, node, "null", QUARK_NULL);
        case 't':  return quark_parse_literal(context, node, "true", QUARK_TRUE);
        case 'f':  return quark_parse_literal(context, node, "false", QUARK_FALSE);
        default :  return quark_parse_number(context, node);
    }
}

int quark_parse(quark_node *node, const char *json) {
    assert(node != NULL);

    int rtn;
    quark_context context;
    node->type = QUARK_NULL;

    context.json = json;
    context.stack.value = NULL;
    context.stack.size = 0;
    context.stack.top = 0;

    quark_parse_whitespace(&context);
    if ((rtn = quark_parse_value(&context, node)) == QUARK_PARSE_OK) {
        quark_parse_whitespace(&context);
        if (*context.json != '\0') {
            node->type = QUARK_NULL;
            rtn = QUARK_PARSE_ROOT_NODE_SINGULAR;
        }
    }

    assert(context.stack.top == 0);
    free(context.stack.value);
    context.stack.size = context.stack.top = 0;
    return rtn;
}

void quark_free(quark_node *node) {
    assert(node != NULL);

    switch (node->type) {
        case QUARK_STRING:
            free(node->value.s.s);
            node->value.s.len = 0;
            break;

        default:
            break;
    }

    node->type = QUARK_NULL;
}

const char* quark_get_string(const quark_node *node) {
    assert(node != NULL && node->type == QUARK_STRING);
    return node->value.s.s;
}

size_t quark_get_string_length(const quark_node *node) {
    assert(node != NULL && node->type == QUARK_STRING);
    return node->value.s.len;
}

void quark_set_string(quark_node *node, const char *string, size_t len) {
    assert(node != NULL && (string != NULL || len == 0));
    quark_free(node);
    node->value.s.s = (char *)malloc(len + 1);
    memcpy(node->value.s.s, string, len);
    node->value.s.s[len] = '\0';
    node->value.s.len = len;
    node->type = QUARK_STRING;
}

int quark_get_boolean(const quark_node *node) {
    assert(node != NULL && (node->type == QUARK_TRUE || node->type == QUARK_FALSE));
    return node->type == QUARK_TRUE;
}

void quark_set_boolean(quark_node *node, int n) {
    assert(node != NULL);
    quark_free(node);
    node->type = n ? QUARK_TRUE : QUARK_FALSE;
}

double quark_get_number(const quark_node *node) {
    assert(node != NULL && node->type == QUARK_NUMBER);
    return node->value.n;
}

void quark_set_number(quark_node *node, double number) {
    assert(node != NULL);
    quark_free(node);
    node->value.n = number;
    node->type = QUARK_NUMBER;
}

quark_type quark_get_type(const quark_node *node) {
    return node->type;
}
