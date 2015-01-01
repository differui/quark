#ifndef QUARK_H__
#define QUARK_H__

#include <stddef.h> /* size_t */

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
    const char *json;
    struct {
        char *value;
        size_t size;
        size_t top;
    } stack;
} quark_context;

typedef struct {
    union {
        struct {
            char *s;
            size_t len;
        } s;
        double n;
    } value;
    quark_type type;
} quark_node;

enum quark_result {
    QUARK_PARSE_OK,
    QUARK_PARSE_EXPECT_VALUE,
    QUARK_PARSE_INVALID_VALUE,
    QUARK_PARSE_ROOT_NODE_SINGULAR,
    QUARK_PARSE_NUMBER_TOO_BIG,
    QUARK_PARSE_MISS_QUOTATION_MARK,
    QUARK_PARSE_INVALID_STRING_ESCAPE,
    QUARK_PARSE_INVALID_STRING_CHAR,
    QUARK_PARSE_INVALID_UNICODE_HEX,
    QUARK_PARSE_INVALID_UNICODE_SURROGATE
};

void* quark_context_push(quark_context *, size_t);
void* quark_context_pop(quark_context *, size_t);

const char* quark_parse_hex4(const char *, unsigned *);
char* quark_encode_utf8(unsigned);

int quark_parse(quark_node *, const char *);
void quark_free(quark_node *);
quark_type quark_get_type(const quark_node *);

#define quark_init(v) do { (v)->type = QUARK_NULL; } while(0);
#define quark_set_null(v) quark_free(v);

int quark_get_boolean(const quark_node *);
void quark_set_boolean(quark_node *, int b);

double quark_get_number(const quark_node *);
void quark_set_number(quark_node *, double n);

const char* quark_get_string(const quark_node *);
void quark_set_string(quark_node *, const char *, size_t len);
size_t quark_get_string_length(const quark_node *);
#endif /* QUARK_H__ */
