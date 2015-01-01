#include <stdio.h>
#include "quark.h"
// #include <stdlib.h>
// #include <string.h>

int test_count = 0;
int test_pass = 0;
int main_rtn = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)\
    do {\
        test_count++;\
        if (equality) {\
            test_pass++;\
        } else {\
            fprintf(stderr, "%s:%d expect: " format " actual: " format "\n", "quark", 0, expect, actual);\
            main_rtn = 1;\
        }\
    } while (0);

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d");

static void test_parse_null() {
    quark_node node;
    node.type = QUARK_NULL;

    EXPECT_EQ_INT(QUARK_PARSE_OK, quark_parse(&node, "null"));
    EXPECT_EQ_INT(QUARK_NULL, quark_get_type(&node));
}

static void test_parse() {
    test_parse_null();
}

int main() {
    test_parse();

    printf(
        "%d/%d (%3.2f%%) passed\n",
        test_pass,
        test_count,
        test_pass * 100.0 / test_count
    );

    return main_rtn;
}
