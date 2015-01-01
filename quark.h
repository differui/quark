#ifndef QUARK_H__
#define QUARK_H__

typedef enum {
    QUARK_NULL,
    QUARK_TRUE,
    QUARK_FALSE,
    QUARK_NUMBER,
    QUARK_STRING,
    QUARK_ARRAY,
    QUARK_OBJECT
} quark_type;

enum quark_result {
    QUARK_PARSE_OK,
    QUARK_PARSE_EXPECT_VALUE,
    QUARK_PARSE_INVALID_VALUE,
    QUARK_PARSE_ROOT_NODE_SINGULAR
};

typedef struct {
    quark_type type;
} quark_node;

typedef struct {
    const char *json;
} quark_context;

int quark_parse(quark_node *, const char *);
quark_type quark_get_type(const quark_node *);
#endif
