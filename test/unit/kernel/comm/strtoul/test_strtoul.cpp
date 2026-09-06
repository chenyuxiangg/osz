// Unit tests for OSZ's `strtoul` parser (`kernel/comm/src/strtoul.c`).
//
// The OSZ parser mirrors the semantics of the C standard `strtoul`,
// but adds a small twist for the sign character: when a leading '-'
// is present the returned value is the two's-complement negation of the
// parsed magnitude (so callers can detect negative inputs even on
// unsigned-typed returns).

#include <gtest/gtest.h>

#include <climits>
#include <cstring>

extern "C" {
#include "strtoul.h"
}

namespace {

TEST(Strtoul, DecimalPositive) {
    EXPECT_EQ(strtoul("123", nullptr, 10), 123u);
}

TEST(Strtoul, DecimalZero) {
    EXPECT_EQ(strtoul("0", nullptr, 10), 0u);
}

TEST(Strtoul, PlusSignAccepted) {
    EXPECT_EQ(strtoul("+42", nullptr, 10), 42u);
}

TEST(Strtoul, NegativeSignYieldsTwoComplement) {
    // The implementation returns -(long)result cast to unsigned long,
    // so on a 64-bit `unsigned long` this is ULLONG_MAX - 122.
    unsigned long got = strtoul("-123", nullptr, 10);
    unsigned long expected = static_cast<unsigned long>(-(long)123);
    EXPECT_EQ(got, expected);
    EXPECT_EQ(got, ULONG_MAX - 122);
}

TEST(Strtoul, LeadingWhitespaceSkipped) {
    EXPECT_EQ(strtoul("   \t 7", nullptr, 10), 7u);
}

TEST(Strtoul, AutoDetectHex) {
    // base=0 with a "0x" prefix -> base 16.
    EXPECT_EQ(strtoul("0x1A", nullptr, 0), 0x1Au);
    EXPECT_EQ(strtoul("0XFF", nullptr, 0), 0xFFu);
}

TEST(Strtoul, AutoDetectOctal) {
    // base=0 with a leading 0 but no "x" -> base 8.
    EXPECT_EQ(strtoul("010", nullptr, 0), 010u);  // 8 in decimal
}

TEST(Strtoul, AutoDetectDecimal) {
    EXPECT_EQ(strtoul("123", nullptr, 0), 123u);
}

TEST(Strtoul, HexWithExplicitBase) {
    EXPECT_EQ(strtoul("deadbeef", nullptr, 16), 0xdeadbeefu);
    // "0x" prefix is also accepted when base=16.
    EXPECT_EQ(strtoul("0xdead", nullptr, 16), 0xdeadu);
}

TEST(Strtoul, BinaryBase) {
    EXPECT_EQ(strtoul("1010", nullptr, 2), 10u);
}

TEST(Strtoul, NoValidDigitsReturnsZero) {
    EXPECT_EQ(strtoul("abc", nullptr, 10), 0u);
    EXPECT_EQ(strtoul("",    nullptr, 10), 0u);
}

TEST(Strtoul, InvalidBaseReturnsZero) {
    EXPECT_EQ(strtoul("123", nullptr, 1),  0u);
    EXPECT_EQ(strtoul("123", nullptr, 37), 0u);
}

TEST(Strtoul, EndptrPointsAtFirstInvalidChar) {
    char *end = nullptr;
    unsigned long got = strtoul("123abc", &end, 10);
    EXPECT_EQ(got, 123u);
    ASSERT_NE(end, nullptr);
    // end should point at the 'a' that stopped the parse — i.e. the
    // SAME pointer that strchr would return.
    const char *expected = std::strchr("123abc", 'a');
    ASSERT_NE(expected, nullptr);
    EXPECT_EQ(end, expected);
    EXPECT_EQ(*end, 'a');
}

TEST(Strtoul, EndptrUnchangedWhenNoValidDigits) {
    const char *input = "abc";
    char *end = nullptr;
    unsigned long got = strtoul(input, &end, 10);
    EXPECT_EQ(got, 0u);
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end, input);
}

TEST(Strtoul, OverflowClampsToUlongMax) {
    // Any value past ULONG_MAX should saturate to ULONG_MAX.
    unsigned long got = strtoul("99999999999999999999999", nullptr, 10);
    EXPECT_EQ(got, ULONG_MAX);
}

TEST(Strtoul, MaxDecimalValue) {
    // Exactly ULONG_MAX should parse cleanly.
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%lu", ULONG_MAX);
    EXPECT_EQ(strtoul(buf, nullptr, 10), ULONG_MAX);
}

}  // namespace