#include <stdio.h>
#include "quark.h"

static int quark_parse_null(quark_context *context, quark_node *node) {
    context->json++;

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
