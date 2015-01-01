#include <stdio.h>
#include <assert.h>
#include "quark.h"

#define EXPECT(c, ch)\
    do {\
        assert(*c->json == (ch)); c->json++;\
    } while(0)

typedef struct {
    const char *json;
} quark_context;

static int quark_parse_true(quark_context *context, quark_node *node) {
    EXPECT(context, 't');

    if (
        context->json[0] != 'r' ||
        context->json[1] != 'u' ||
        context->json[2] != 'e') {
        return QUARK_PARSE_INVALID_VALUE;
    }

    context->json += 3;
    node->type = QUARK_TRUE;

    return QUARK_PARSE_OK;
}

static int quark_parse_false(quark_context *context, quark_node *node) {
    EXPECT(context, 'f');

    if (
        context->json[0] != 'a' ||
        context->json[1] != 'l' ||
        context->json[2] != 's' ||
        context->json[3] != 'e') {
        return QUARK_PARSE_INVALID_VALUE;
    }

    context->json += 4;
    node->type = QUARK_FALSE;

    return QUARK_PARSE_OK;
}

static int quark_parse_null(quark_context *context, quark_node *node) {
    EXPECT(context, 'n');

    if (context->json[0] != 'u' || context->json[1] != 'l' || context->json[2] != 'l') {
        return QUARK_PARSE_INVALID_VALUE;
    }

    context->json += 3;
    node->type = QUARK_NULL;

    return QUARK_PARSE_OK;
}

static void quark_parse_whitespace(quark_context *context) {
    const char *p = context->json;

    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }

    context->json = p;
}

static int quark_parse_value(quark_context *context, quark_node *node) {
    switch (*context->json) {
        case 'n':  return quark_parse_null(context, node);
        case 't':  return quark_parse_true(context, node);
        case 'f':  return quark_parse_false(context, node);
        case '\0': return QUARK_PARSE_EXPECT_VALUE;
        default :  return QUARK_PARSE_INVALID_VALUE;
    }
}

int quark_parse(quark_node *node, const char *json) {
    quark_context context;

    context.json = json;
    node->type = QUARK_NULL;

    quark_parse_whitespace(&context);
    return quark_parse_value(&context, node);
}

quark_type quark_get_type(const quark_node *node) {
    return node->type;
}
