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

typedef struct {
    double number;
    quark_type type;
} quark_node;

enum quark_result {
    QUARK_PARSE_OK,
    QUARK_PARSE_EXPECT_VALUE,
    QUARK_PARSE_INVALID_VALUE,
    QUARK_PARSE_ROOT_NODE_SINGULAR,
    QUARK_PARSE_NUMBER_TOO_BIG
};

int quark_parse(quark_node *, const char *);
double quark_get_number(const quark_node *);
quark_type quark_get_type(const quark_node *);
#endif
