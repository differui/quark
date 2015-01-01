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
            fprintf(\
                stderr,\
                "%s:%d expect: " format " actual: " format "\n",\
                __func__,\
                __LINE__,\
                expect,\
                actual\
            );\
            main_rtn = 1;\
        }\
    } while (0);

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d");
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%f");
#define TEST_ERROR(expect, json)\
    do {\
        quark_node node;\
        node.type = QUARK_FALSE;\
        EXPECT_EQ_INT(expect, quark_parse(&node, json));\
        EXPECT_EQ_INT(QUARK_NULL, quark_get_type(&node));\
    } while(0);
#define TEST_NUMBER(expect, json)\
    do {\
        quark_node node;\
        EXPECT_EQ_INT(QUARK_PARSE_OK, quark_parse(&node, json));\
        EXPECT_EQ_INT(QUARK_NUMBER, quark_get_type(&node));\
        EXPECT_EQ_DOUBLE(expect, quark_get_number(&node));\
    } while(0);
#define TEST_LITERAL(expect, json)\
    do {\
        quark_node node;\
        node.type = QUARK_NULL;\
        EXPECT_EQ_INT(QUARK_PARSE_OK, quark_parse(&node, json));\
        EXPECT_EQ_INT(expect, quark_get_type(&node));\
    } while(0);

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000");

    // boundary values
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_null() {
    TEST_LITERAL(QUARK_NULL, "null");
}

static void test_parse_true() {
    TEST_LITERAL(QUARK_TRUE, "true");
}

static void test_parse_false() {
    TEST_LITERAL(QUARK_FALSE, "false");
}

static void test_parse_expect_value() {
    TEST_ERROR(QUARK_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(QUARK_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "fal");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "tru");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "?");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, ".123");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1.");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1..");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1..2");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1ee2");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1EE2");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "--1");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1e++1");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1e--1");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1e+1+");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "1e-1-");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(QUARK_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_single() {
    quark_node node;

    TEST_ERROR(QUARK_PARSE_ROOT_NODE_SINGULAR, "null a");
    TEST_ERROR(QUARK_PARSE_ROOT_NODE_SINGULAR, "123 123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(QUARK_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(QUARK_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse() {
    test_parse_number();
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_single();
    test_parse_number_too_big();
}

int main() {
    test_parse();

    printf(
        "%d total, %d passed, %d failed, rate %3.2f%%.\n",
        test_count,
        test_pass,
        test_count - test_pass,
        test_pass * 100.0 / test_count
    );

    return main_rtn;
}
