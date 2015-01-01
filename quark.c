#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include "quark.h"

#define EXPECT(c, ch)\
    do {\
        assert(*c->json == (ch)); c->json++;\
    } while(0)
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_E(c) ((c) == 'e' || (c) == 'E')
#define IS_NOT_E(c) ((c) != 'e' && (c) != 'E')

typedef struct {
    const char *json;
} quark_context;

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
    node->number = strtod(context->json, &end);

    if (errno == ERANGE && (node->number == HUGE_VAL || node->number == -HUGE_VAL))
        return QUARK_PARSE_NUMBER_TOO_BIG;

    node->type = QUARK_NUMBER;
    context->json = end;
    return QUARK_PARSE_OK;
}

static void quark_parse_whitespace(quark_context *context) {
    const char *p = context->json;

    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;

    context->json = p;
}

static int quark_parse_value(quark_context *context, quark_node *node) {
    switch (*context->json) {
        case '\0': return QUARK_PARSE_EXPECT_VALUE;
        case 'n':  return quark_parse_literal(context, node, "null", QUARK_NULL);
        case 't':  return quark_parse_literal(context, node, "true", QUARK_TRUE);
        case 'f':  return quark_parse_literal(context, node, "false", QUARK_FALSE);
        default :  return quark_parse_number(context, node);
    }
}

int quark_parse(quark_node *node, const char *json) {
    int rtn;
    quark_context context;

    context.json = json;
    node->type = QUARK_NULL;

    quark_parse_whitespace(&context);
    if ((rtn = quark_parse_value(&context, node)) == QUARK_PARSE_OK) {
        quark_parse_whitespace(&context);

        if (*context.json != '\0') {
            node->type = QUARK_NULL;
            rtn = QUARK_PARSE_ROOT_NODE_SINGULAR;
        }
    }

    return rtn;
}

double quark_get_number(const quark_node *node) {
    assert(node != NULL && node->type == QUARK_NUMBER);

    return node->number;
}

quark_type quark_get_type(const quark_node *node) {
    return node->type;
}
