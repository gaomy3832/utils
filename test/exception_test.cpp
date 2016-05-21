/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/exception.h"
#include <cstring>

static const char* msg = "Test";

TEST(Exception, RangeException) {
    try {
        throw RangeException(msg);
        ASSERT_TRUE(false);
    } catch (RangeException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, KeyInUseException) {
    try {
        throw KeyInUseException(msg);
        ASSERT_TRUE(false);
    } catch (KeyInUseException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, MemoryException) {
    try {
        throw MemoryException(msg);
        ASSERT_TRUE(false);
    } catch (MemoryException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, FileException) {
    try {
        throw FileException(msg);
        ASSERT_TRUE(false);
    } catch (FileException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, PermissionException) {
    try {
        throw PermissionException(msg);
        ASSERT_TRUE(false);
    } catch (PermissionException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, NullPointerException) {
    try {
        throw NullPointerException(msg);
        ASSERT_TRUE(false);
    } catch (NullPointerException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, UnknownTypeException) {
    try {
        throw UnknownTypeException(msg);
        ASSERT_TRUE(false);
    } catch (UnknownTypeException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, InvalidArgumentException) {
    try {
        throw InvalidArgumentException(msg);
        ASSERT_TRUE(false);
    } catch (InvalidArgumentException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, TimeoutException) {
    try {
        throw TimeoutException(msg);
        ASSERT_TRUE(false);
    } catch (TimeoutException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

TEST(Exception, InternalException) {
    try {
        throw InternalException(msg);
        ASSERT_TRUE(false);
    } catch (InternalException& e) {
        ASSERT_EQ(0, std::strcmp(msg, e.what()));
    }
}

