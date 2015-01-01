#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quark.h"

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

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%f");
#define EXPECT_EQ_STRING(expect, actual, alength) EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp((expect), (actual), (alength)) == 0, expect, actual, "%s");
#define EXPECT_EQ_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_EQ_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
#define TEST_PARSE_ERROR(error, json)\
    do {\
        quark_node node;\
        node.type = QUARK_FALSE;\
        EXPECT_EQ_INT(error, quark_parse(&node, json));\
        EXPECT_EQ_INT(QUARK_NULL, quark_get_type(&node));\
    } while(0);
#define TEST_PARSE_NUMBER(expect, json)\
    do {\
        quark_node node;\
        EXPECT_EQ_INT(QUARK_PARSE_OK, quark_parse(&node, json));\
        EXPECT_EQ_INT(QUARK_NUMBER, quark_get_type(&node));\
        EXPECT_EQ_DOUBLE(expect, quark_get_number(&node));\
    } while(0);
#define TEST_PARSE_STRING(expect, json)\
    do {\
        quark_node node;\
        EXPECT_EQ_INT(QUARK_PARSE_OK, quark_parse(&node, json));\
        EXPECT_EQ_INT(QUARK_STRING, quark_get_type(&node));\
        EXPECT_EQ_STRING(expect, quark_get_string(&node), quark_get_string_length(&node));\
    } while(0);
#define TEST_PARSE_LITERAL(expect, json)\
    do {\
        quark_node node;\
        EXPECT_EQ_INT(QUARK_PARSE_OK, quark_parse(&node, json));\
        EXPECT_EQ_INT(expect, quark_get_type(&node));\
    } while(0);
#define TEST_ACCESS_STRING(expect, len)\
    do {\
        quark_node node;\
        quark_init(&node);\
        quark_set_string(&node, expect, len);\
        EXPECT_EQ_STRING(expect, quark_get_string(&node), quark_get_string_length(&node));\
        quark_free(&node);\
    } while(0);
#define TEST_ACCESS_NUMBER(expect)\
    do {\
        quark_node node;\
        quark_init(&node);\
        quark_set_number(&node, expect);\
        EXPECT_EQ_DOUBLE(expect, quark_get_number(&node));\
        quark_free(&node);\
    } while(0);

static void test_parse_number() {
    TEST_PARSE_NUMBER(0.0, "0");
    TEST_PARSE_NUMBER(0.0, "-0");
    TEST_PARSE_NUMBER(0.0, "-0.0");
    TEST_PARSE_NUMBER(1.0, "1");
    TEST_PARSE_NUMBER(-1.0, "-1");
    TEST_PARSE_NUMBER(1.5, "1.5");
    TEST_PARSE_NUMBER(-1.5, "-1.5");
    TEST_PARSE_NUMBER(3.1416, "3.1416");
    TEST_PARSE_NUMBER(1E10, "1E10");
    TEST_PARSE_NUMBER(1e10, "1e10");
    TEST_PARSE_NUMBER(1E+10, "1E+10");
    TEST_PARSE_NUMBER(1E-10, "1E-10");
    TEST_PARSE_NUMBER(-1E10, "-1E10");
    TEST_PARSE_NUMBER(-1e10, "-1e10");
    TEST_PARSE_NUMBER(-1E+10, "-1E+10");
    TEST_PARSE_NUMBER(-1E-10, "-1E-10");
    TEST_PARSE_NUMBER(1.234E+10, "1.234E+10");
    TEST_PARSE_NUMBER(1.234E-10, "1.234E-10");
    TEST_PARSE_NUMBER(0.0, "1e-10000");

    // boundary values
    TEST_PARSE_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_PARSE_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_PARSE_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_PARSE_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_PARSE_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_PARSE_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_PARSE_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_PARSE_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_PARSE_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_null() {
    TEST_PARSE_LITERAL(QUARK_NULL, "null");
}

static void test_parse_true() {
    TEST_PARSE_LITERAL(QUARK_TRUE, "true");
}

static void test_parse_false() {
    TEST_PARSE_LITERAL(QUARK_FALSE, "false");
}

static void test_parse_expect_value() {
    TEST_PARSE_ERROR(QUARK_PARSE_EXPECT_VALUE, "");
    TEST_PARSE_ERROR(QUARK_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "nul");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "fal");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "tru");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "?");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "+0");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "+1");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, ".123");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1.");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1..");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1..2");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1ee2");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1EE2");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "--1");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1e++1");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1e--1");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1e+1+");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "1e-1-");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "INF");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "inf");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "NAN");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_single() {
    quark_node node;

    TEST_PARSE_ERROR(QUARK_PARSE_ROOT_NODE_SINGULAR, "null a");
    TEST_PARSE_ERROR(QUARK_PARSE_ROOT_NODE_SINGULAR, "123 123");
}

static void test_parse_number_too_big() {
    TEST_PARSE_ERROR(QUARK_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_PARSE_ERROR(QUARK_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void text_parse_string() {
    TEST_PARSE_STRING("", "\"\"");
    TEST_PARSE_STRING("Hello", "\"Hello\"");
    TEST_PARSE_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_PARSE_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_PARSE_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_PARSE_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    // TEST_PARSE_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    // TEST_PARSE_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    // TEST_PARSE_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    // TEST_PARSE_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_missing_quotation_mark() {
    TEST_PARSE_ERROR(QUARK_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_PARSE_ERROR(QUARK_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_PARSE_ERROR(QUARK_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_access_string() {
    TEST_ACCESS_STRING("", 0);
    TEST_ACCESS_STRING("hello", 5);
    TEST_ACCESS_STRING("hello world", 11);
    TEST_ACCESS_STRING("hello\0world", 11);
}

static void test_access_number() {
    TEST_ACCESS_NUMBER(0.0);
    TEST_ACCESS_NUMBER(2.5);
    TEST_ACCESS_NUMBER(-3.5);
    TEST_ACCESS_NUMBER(-24.0);
}

static void test_access_boolean() {
    quark_node node;
    quark_init(&node);
    quark_set_string(&node, "a", 1);
    quark_set_boolean(&node, 1);
    EXPECT_EQ_TRUE(quark_get_boolean(&node));
    quark_set_boolean(&node, 0);
    EXPECT_EQ_FALSE(quark_get_boolean(&node));
}

static void test_access_null() {
    quark_node node;
    quark_init(&node);
    quark_set_string(&node, "a", 1);
    quark_set_null(&node);
    EXPECT_EQ_INT(QUARK_NULL, quark_get_type(&node));
}

static void test() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    text_parse_string();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_single();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    // test_parse_invalid_unicode_hex();
    // test_parse_invalid_unicode_surrogate();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main() {
    test();

    printf(
        "%d total, %d passed, %d failed, rate %3.2f%%.\n",
        test_count,
        test_pass,
        test_count - test_pass,
        test_pass * 100.0 / test_count
    );

    return main_rtn;
}
