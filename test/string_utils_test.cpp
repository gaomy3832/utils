/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/string_utils.h"

TEST(StringUtils, tokenize) {
    std::vector<std::string> tokens;

    // Normal.
    std::string str1 = "a bb ccc dddd";
    tokens.clear();
    tokenize(str1, &tokens);

    ASSERT_EQ(4, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("bb", tokens[1]);
    ASSERT_EQ("ccc", tokens[2]);
    ASSERT_EQ("dddd", tokens[3]);

    // Multiple delims.
    const char* str2 = "a  b     c d";
    tokens.clear();
    tokenize(str2, &tokens);

    ASSERT_EQ(4, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("b", tokens[1]);
    ASSERT_EQ("c", tokens[2]);
    ASSERT_EQ("d", tokens[3]);

    // Multiple types of delim.
    std::string str3 = "a  b\tc\nd  \te";
    tokens.clear();
    tokenize(str3, &tokens, " \t\n");

    ASSERT_EQ(5, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("b", tokens[1]);
    ASSERT_EQ("c", tokens[2]);
    ASSERT_EQ("d", tokens[3]);
    ASSERT_EQ("e", tokens[4]);

    // Append to tokens.
    tokenize(str1, &tokens);

    ASSERT_EQ(9, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("b", tokens[1]);
    ASSERT_EQ("c", tokens[2]);
    ASSERT_EQ("d", tokens[3]);
    ASSERT_EQ("e", tokens[4]);
    ASSERT_EQ("a", tokens[5]);
    ASSERT_EQ("bb", tokens[6]);
    ASSERT_EQ("ccc", tokens[7]);
    ASSERT_EQ("dddd", tokens[8]);

    // nullptr for tokens.
    tokenize(str3, nullptr);
}

