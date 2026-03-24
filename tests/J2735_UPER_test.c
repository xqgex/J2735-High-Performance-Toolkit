/**
 * Copyright 2026 Yogev Neumann
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2026 Yogev Neumann
 */
/**
 * @file
 * @author Yogev Neumann
 * @brief Unit tests for UPER encoding primitives (Phase 4).
 *
 * Tests for the shared UPER parsing functions:
 * - j2735_internal_inline_read_length_determinant()
 * - j2735_internal_inline_read_nsnnwn()
 * - j2735_internal_inline_skip_extensions()
 *
 * These are comprehensive edge-case tests covering all branches and error paths.
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_UPER_test.h"
#include "J2735_internal_inline.h"

/* ============================================================================
 * Test Constants
 * ============================================================================
 * Poison values used to verify output pointers are properly written.
 * These should be overwritten by the function under test.
 * ============================================================================
 */
/** @brief Poison value for uint16_t outputs (ensures pointer is written). */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-to-enum,modernize-macro-to-enum) */
#define TEST_POISON_U16 0xFFFFU
/** @brief Poison value for uint32_t outputs (ensures pointer is written). */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-to-enum,modernize-macro-to-enum) */
#define TEST_POISON_U32 0xFFFFFFFFU
/** @brief Non-zero bit offset for testing non-byte-aligned reads. */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-to-enum,modernize-macro-to-enum) */
#define TEST_NONZERO_BIT_OFFSET 5U

/* ============================================================================
 * j2735_internal_inline_read_length_determinant() Tests
 * ============================================================================
 * X.691 §11.9 Length Determinant (Unconstrained):
 * - Short form: 0xxxxxxx (8 bits) → length 0-127
 * - Long form:  10xxxxxx xxxxxxxx (16 bits) → length 0-16383
 * - Fragmented: 11xxxxxx... → not supported (returns 0)
 * ============================================================================
 */

/**
 * @brief Test length determinant short form: minimum value (0).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: short (bit[0] = 0)
 * - Length value: 0 (0x00)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field  | Value   |
 * |---------------|-------|--------|---------|
 * | 0             | 1     | form   | 0       |
 * | 1             | 7     | length | 0000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields              |
 * |------|------|----------|---------------------|
 * | 0    | 0x00 | 00000000 | form(0) + length(0) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_short_form_min(void) {
  static const uint8_t buf[] = {
      0x00,                                          /* 0_0000000: short form, length=0 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = TEST_POISON_U16;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8U, bits_consumed, "Should consume 8 bits for short form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0U, length, "Length should be 0");
}

/**
 * @brief Test length determinant short form: maximum value (127).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: short (bit[0] = 0)
 * - Length value: 127 (0x7F)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field  | Value   |
 * |---------------|-------|--------|---------|
 * | 0             | 1     | form   | 0       |
 * | 1             | 7     | length | 1111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x7F | 01111111 | form(0) + length(127) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_short_form_max(void) {
  static const uint8_t buf[] = {
      0x7F,                                          /* 0_1111111: short form, length=127 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8U, bits_consumed, "Should consume 8 bits for short form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(127U, length, "Length should be 127");
}

/**
 * @brief Test length determinant short form: typical value (1).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: short (bit[0] = 0)
 * - Length value: 1 (0x01)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field  | Value   |
 * |---------------|-------|--------|---------|
 * | 0             | 1     | form   | 0       |
 * | 1             | 7     | length | 0000001 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields              |
 * |------|------|----------|---------------------|
 * | 0    | 0x01 | 00000001 | form(0) + length(1) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_short_form_typical(void) {
  static const uint8_t buf[] = {
      0x01,                                          /* 0_0000001: short form, length=1 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8U, bits_consumed, "Should consume 8 bits for short form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(1U, length, "Length should be 1");
}

/**
 * @brief Test length determinant long form: minimum value (0).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: long (bits[0:1] = 10)
 * - Length value: 0 (0x0000)
 *
 * @par Wire Format (16 bits total):
 * | Offset (bits) | Width | Field  | Value          |
 * |---------------|-------|--------|----------------|
 * | 0             | 2     | form   | 10             |
 * | 2             | 14    | length | 00000000000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                  |
 * |------|------|----------|-------------------------|
 * | 0    | 0x80 | 10000000 | form(10) + length[13:8] |
 * | 1    | 0x00 | 00000000 | length[7:0]             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_long_form_min(void) {
  static const uint8_t buf[] = {
      0x80, 0x00,                                    /* 10_00000000000000: long form, length=0 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = TEST_POISON_U16;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(16U, bits_consumed, "Should consume 16 bits for long form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0U, length, "Length should be 0");
}

/**
 * @brief Test length determinant long form: just above short form range (128).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: long (bits[0:1] = 10)
 * - Length value: 128 (0x0080)
 *
 * @par Wire Format (16 bits total):
 * | Offset (bits) | Width | Field  | Value          |
 * |---------------|-------|--------|----------------|
 * | 0             | 2     | form   | 10             |
 * | 2             | 14    | length | 00000010000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                  |
 * |------|------|----------|-------------------------|
 * | 0    | 0x80 | 10000000 | form(10) + length[13:8] |
 * | 1    | 0x80 | 10000000 | length[7:0]             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_long_form_128(void) {
  static const uint8_t buf[] = {
      0x80, 0x80,                                    /* 10_00000010000000: long form, length=128 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(16U, bits_consumed, "Should consume 16 bits for long form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(128U, length, "Length should be 128");
}

/**
 * @brief Test length determinant long form: maximum value (16383).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: long (bits[0:1] = 10)
 * - Length value: 16383 (0x3FFF)
 *
 * @par Wire Format (16 bits total):
 * | Offset (bits) | Width | Field  | Value          |
 * |---------------|-------|--------|----------------|
 * | 0             | 2     | form   | 10             |
 * | 2             | 14    | length | 11111111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0xBF | 10111111 | form(10) + length[13:8]     |
 * | 1    | 0xFF | 11111111 | length[7:0]                 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_long_form_max(void) {
  static const uint8_t buf[] = {
      0xBF, 0xFF, /* 10_11111111111111: long form, length=16383 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(16U, bits_consumed, "Should consume 16 bits for long form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(16383U, length, "Length should be 16383");
}

/**
 * @brief Test length determinant fragmented form: returns error.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: fragmented (bits[0:1] = 11) — unsupported, returns error
 * - Expected: length = 0, bits consumed = 0
 *
 * @par Wire Format (2+ bits total):
 * | Offset (bits) | Width | Field | Value |
 * |---------------|-------|-------|-------|
 * | 0             | 2     | form  | 11    |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields          |
 * |------|------|----------|-----------------|
 * | 0    | 0xC0 | 11000000 | form(11) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_fragmented_error(void) {
  static const uint8_t buf[] = {
      0xC0, 0x00,                                    /* 11_...: fragmented (unsupported) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = TEST_POISON_U16;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, bits_consumed, "Should return 0 for fragmented encoding");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0U, length, "Length should be 0 on error");
}

/**
 * @brief Test length determinant with non-zero bit offset.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Bit offset: 3
 * - Form: short (bit[0] = 0)
 * - Length value: 42 (0x2A)
 *
 * @par Wire Format (8 bits at offset 3):
 * | Offset (bits) | Width | Field   | Value   |
 * |---------------|-------|---------|---------|
 * | 0             | 3     | padding | 111     |
 * | 3             | 1     | form    | 0       |
 * | 4             | 7     | length  | 0101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0xE5 | 11100101 | pad(111) + form(0) + length[6:3]    |
 * | 1    | 0x40 | 01000000 | length[2:0] + pad                   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_nonzero_bit_offset(void) {
  /* Bit offset 3: skip first 3 bits, then read 0_0101010 = 42 */
  /* Byte layout: 111_00101 010_00000 = 0xE5, 0x40 */
  static const uint8_t buf[] = {
      0xE5, 0x40,                                    /* 111_00101 010_00000: padding + len=42 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t length = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(buf, 3U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8U, bits_consumed, "Should consume 8 bits for short form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(42U, length, "Length should be 42");
}

/**
 * @brief Test read_length_determinant with deliberately misaligned buffer pointer.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.9 Length Determinant (Unconstrained)
 * length-determinant ::= CHOICE {
 *   short-form    BIT STRING (SIZE(8)),    -- 0xxxxxxx → 0..127
 *   long-form     BIT STRING (SIZE(16)),   -- 10xxxxxx xxxxxxxx → 0..16383
 *   fragmented    BIT STRING (SIZE(8+))    -- 11xxxxxx... (unsupported)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Buffer: &payload[1] (deliberately misaligned)
 * - Form: short (bit[0] = 0)
 * - Length value: 42 (0x2A)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field  | Value   |
 * |---------------|-------|--------|---------|
 * | 0             | 1     | form   | 0       |
 * | 1             | 7     | length | 0101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields               |
 * |------|------|----------|----------------------|
 * | 0    | 0xFF | 11111111 | junk (misalignment)  |
 * | 1    | 0x2A | 00101010 | form(0) + length(42) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_length_determinant_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0x2A,                                          /* 0_0101010: short form, length=42 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  uint16_t length = TEST_POISON_U16;
  uint8_t bits_consumed = j2735_internal_inline_read_length_determinant(unaligned_ptr, 0U, &length);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(8U, bits_consumed,
                                  "Misaligned: should consume 8 bits for short form");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(42U, length, "Misaligned: length should be 42");
}

/* ============================================================================
 * j2735_internal_inline_read_nsnnwn() Tests
 * ============================================================================
 * X.691 §11.6 Normally-Small Non-Negative Whole Number:
 * - Small form: 0xxxxxx (7 bits) → value 0-63
 * - Large form: 1 + length_det(octets) + value_bytes → value 64+
 * ============================================================================
 */

/**
 * @brief Test nsnnwn small form: minimum value (0).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: small (bit[0] = 0)
 * - Value: 0
 *
 * @par Wire Format (7 bits total):
 * | Offset (bits) | Width | Field | Value  |
 * |---------------|-------|-------|--------|
 * | 0             | 1     | form  | 0      |
 * | 1             | 6     | value | 000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x00 | 00000000 | form(0) + value(0) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_small_form_min(void) {
  static const uint8_t buf[] = {
      0x00,                                          /* 0_000000_0: small form, value=0 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = TEST_POISON_U32;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(7U, bits_consumed, "Should consume 7 bits for small form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, value, "Value should be 0");
}

/**
 * @brief Test nsnnwn small form: maximum value (63).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: small (bit[0] = 0)
 * - Value: 63 (0x3F)
 *
 * @par Wire Format (7 bits total):
 * | Offset (bits) | Width | Field | Value  |
 * |---------------|-------|-------|--------|
 * | 0             | 1     | form  | 0      |
 * | 1             | 6     | value | 111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x7E | 01111110 | form(0) + value(63) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_small_form_max(void) {
  static const uint8_t buf[] = {
      0x7E,                                          /* 0_111111_0: small form, value=63 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(7U, bits_consumed, "Should consume 7 bits for small form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(63U, value, "Value should be 63");
}

/**
 * @brief Test nsnnwn small form: typical value (5).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: small (bit[0] = 0)
 * - Value: 5 (0x05)
 *
 * @par Wire Format (7 bits total):
 * | Offset (bits) | Width | Field | Value  |
 * |---------------|-------|-------|--------|
 * | 0             | 1     | form  | 0      |
 * | 1             | 6     | value | 000101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x0A | 00001010 | form(0) + value(5) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_small_form_typical(void) {
  static const uint8_t buf[] = {
      0x0A,                                          /* 0_000101_0: small form, value=5 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(7U, bits_consumed, "Should consume 7 bits for small form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(5U, value, "Value should be 5");
}

/**
 * @brief Test nsnnwn large form: minimum large value (64).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 1 octet
 * - Value: 64 (0x40)
 *
 * @par Wire Format (17 bits total):
 * | Offset (bits) | Width | Field      | Value    |
 * |---------------|-------|------------|----------|
 * | 0             | 1     | prefix     | 1        |
 * | 1             | 8     | length_det | 00000001 |
 * | 9             | 8     | value      | 01000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x80 | 10000000 | prefix(1) + length_det[7:1]    |
 * | 1    | 0xA0 | 10100000 | length_det[0] + value[7:1]     |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_64(void) {
  static const uint8_t buf[] = {
      0x80, 0xA0, /* 1_0000000 1_0100000 0: large, len=1, val=64 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(17U, bits_consumed,
                                  "Should consume 17 bits for large form (1+8+8)");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(64U, value, "Value should be 64");
}

/**
 * @brief Test nsnnwn large form: value 100.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 1 octet
 * - Value: 100 (0x64)
 *
 * @par Wire Format (17 bits total):
 * | Offset (bits) | Width | Field      | Value    |
 * |---------------|-------|------------|----------|
 * | 0             | 1     | prefix     | 1        |
 * | 1             | 8     | length_det | 00000001 |
 * | 9             | 8     | value      | 01100100 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x80 | 10000000 | prefix(1) + length_det[7:1]    |
 * | 1    | 0xB2 | 10110010 | length_det[0] + value[7:1]     |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_100(void) {
  static const uint8_t buf[] = {
      0x80, 0xB2, /* 1_0000000 1_0110010 0: large, len=1, val=100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(17U, bits_consumed, "Should consume 17 bits for large form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(100U, value, "Value should be 100");
}

/**
 * @brief Test nsnnwn large form: value 255 (max 1 byte).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 1 octet
 * - Value: 255 (0xFF)
 *
 * @par Wire Format (17 bits total):
 * | Offset (bits) | Width | Field      | Value    |
 * |---------------|-------|------------|----------|
 * | 0             | 1     | prefix     | 1        |
 * | 1             | 8     | length_det | 00000001 |
 * | 9             | 8     | value      | 11111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x80 | 10000000 | prefix(1) + length_det[7:1]    |
 * | 1    | 0xFF | 11111111 | length_det[0] + value[7:1]     |
 * | 2    | 0x80 | 10000000 | value[0] + pad                 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_255(void) {
  static const uint8_t buf[] = {
      0x80, 0xFF, 0x80, /* 1_0000000 1_1111111 1_...: large, len=1, val=255 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(17U, bits_consumed, "Should consume 17 bits for large form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(255U, value, "Value should be 255");
}

/**
 * @brief Test nsnnwn large form: value 256 (needs 2 bytes).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 2 octets
 * - Value: 256 (0x0100)
 *
 * @par Wire Format (25 bits total):
 * | Offset (bits) | Width | Field      | Value            |
 * |---------------|-------|------------|------------------|
 * | 0             | 1     | prefix     | 1                |
 * | 1             | 8     | length_det | 00000010         |
 * | 9             | 16    | value      | 0000000100000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x81 | 10000001 | prefix(1) + length_det[7:1]    |
 * | 1    | 0x00 | 00000000 | length_det[0] + value[15:9]    |
 * | 2    | 0x80 | 10000000 | value[8:1]                     |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_256(void) {
  static const uint8_t buf[] = {
      0x81, 0x00, 0x80, /* 1_0000001 0_0000000 1_0000000 0: len=2, val=256 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(25U, bits_consumed,
                                  "Should consume 25 bits for large form (1+8+16)");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(256U, value, "Value should be 256");
}

/**
 * @brief Test nsnnwn large form: value 65535 (max 2 bytes).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 2 octets
 * - Value: 65535 (0xFFFF)
 *
 * @par Wire Format (25 bits total):
 * | Offset (bits) | Width | Field      | Value            |
 * |---------------|-------|------------|------------------|
 * | 0             | 1     | prefix     | 1                |
 * | 1             | 8     | length_det | 00000010         |
 * | 9             | 16    | value      | 1111111111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x81 | 10000001 | prefix(1) + length_det[7:1]    |
 * | 1    | 0x7F | 01111111 | length_det[0] + value[15:9]    |
 * | 2    | 0xFF | 11111111 | value[8:1]                     |
 * | 3    | 0x80 | 10000000 | value[0] + pad                 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_65535(void) {
  static const uint8_t buf[] = {
      0x81, 0x7F, 0xFF, 0x80, /* 1_0000001 0_1111111 11111111 1_...: len=2, val=65535 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(25U, bits_consumed, "Should consume 25 bits for large form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(65535U, value, "Value should be 65535");
}

/**
 * @brief Test nsnnwn large form: 4-byte value (max supported).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 4 octets
 * - Value: 0x12345678
 *
 * @par Wire Format (41 bits total):
 * | Offset (bits) | Width | Field      | Value                            |
 * |---------------|-------|------------|----------------------------------|
 * | 0             | 1     | prefix     | 1                                |
 * | 1             | 8     | length_det | 00000100                         |
 * | 9             | 32    | value      | 00010010001101000101011001111000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x82 | 10000010 | prefix(1) + length_det[7:1]    |
 * | 1    | 0x09 | 00001001 | length_det[0] + value[31:25]   |
 * | 2    | 0x1A | 00011010 | value[24:17]                   |
 * | 3    | 0x2B | 00101011 | value[16:9]                    |
 * | 4    | 0x3C | 00111100 | value[8:1]                     |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_4_bytes(void) {
  static const uint8_t buf[] = {
      0x82,                   /* 1_0000010 0: large, len_det starts */
      0x09, 0x1A, 0x2B, 0x3C, /* 0_0001001 0_0011010 0_0101011 0_0111100: len=4, val starts */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(41U, bits_consumed,
                                  "Should consume 41 bits for large form (1+8+32)");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0x12345678U, value, "Value should be 0x12345678");
}

/**
 * @brief Test nsnnwn large form: 5-byte value (error - exceeds uint32_t).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: short form, 5 octets — exceeds uint32_t, returns error
 * - Expected: value = 0, bits consumed = 0
 *
 * @par Wire Format (9+ bits total):
 * | Offset (bits) | Width | Field      | Value    |
 * |---------------|-------|------------|----------|
 * | 0             | 1     | prefix     | 1        |
 * | 1             | 8     | length_det | 00000101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x82 | 10000010 | prefix(1) + length_det[7:1]    |
 * | 1    | 0x80 | 10000000 | length_det[0] + pad            |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_large_form_5_bytes_error(void) {
  static const uint8_t buf[] = {
      0x82, 0x80, /* 1_0000010 1_...: large, len_det=5 (too large) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = TEST_POISON_U32;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, bits_consumed, "Should return 0 for value > 4 bytes");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, value, "Value should be 0 on error");
}

/**
 * @brief Test nsnnwn large form: fragmented length (error).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Form: large (bit[0] = 1)
 * - Length determinant: fragmented form (bits[1:2] = 11) — unsupported, returns error
 * - Expected: value = 0, bits consumed = 0
 *
 * @par Wire Format (3+ bits total):
 * | Offset (bits) | Width | Field          | Value |
 * |---------------|-------|----------------|-------|
 * | 0             | 1     | prefix         | 1     |
 * | 1             | 2     | length_det_tag | 11    |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                            |
 * |------|------|----------|-----------------------------------|
 * | 0    | 0xE0 | 11100000 | prefix(1) + length_det(11) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_fragmented_error(void) {
  static const uint8_t buf[] = {
      0xE0,                                          /* 1_1100000: large + fragmented len det */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = TEST_POISON_U32;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, bits_consumed, "Should return 0 for fragmented encoding");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, value, "Value should be 0 on error");
}

/**
 * @brief Test nsnnwn with non-zero bit offset.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Bit offset: 3
 * - Form: small (bit[0] = 0)
 * - Value: 31 (0x1F)
 *
 * @par Wire Format (7 bits at offset 3):
 * | Offset (bits) | Width | Field   | Value  |
 * |---------------|-------|---------|--------|
 * | 0             | 3     | padding | 111    |
 * | 3             | 1     | form    | 0      |
 * | 4             | 6     | value   | 011111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                             |
 * |------|------|----------|------------------------------------|
 * | 0    | 0xE7 | 11100111 | pad(111) + form(0) + value[5:2]    |
 * | 1    | 0xC0 | 11000000 | value[1:0] + pad                   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_nonzero_bit_offset(void) {
  /* Bit offset 3: skip first 3 bits, then read 0_011111 = 31 */
  static const uint8_t buf[] = {
      0xE7, 0xC0,                                    /* 111_0_0111 11_000000: padding + val=31 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t value = 0U;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(buf, 3U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(7U, bits_consumed, "Should consume 7 bits for small form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(31U, value, "Value should be 31");
}

/**
 * @brief Test read_nsnnwn with deliberately misaligned buffer pointer.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §11.6 Normally-Small Non-Negative Whole Number
 * nsnnwn ::= CHOICE {
 *   small-form  BIT STRING (SIZE(7)),          -- 0xxxxxx → 0..63
 *   large-form  SEQUENCE {                     -- 1 + length_det + value
 *     prefix       BIT STRING (SIZE(1)),       -- always 1
 *     length       length-determinant,         -- octet count
 *     value        OCTET STRING (SIZE(1..4))   -- big-endian unsigned
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Buffer: &payload[1] (deliberately misaligned)
 * - Form: small (bit[0] = 0)
 * - Value: 10 (0x0A)
 *
 * @par Wire Format (7 bits total):
 * | Offset (bits) | Width | Field | Value  |
 * |---------------|-------|-------|--------|
 * | 0             | 1     | form  | 0      |
 * | 1             | 6     | value | 001010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0xFF | 11111111 | junk (misalignment)        |
 * | 1    | 0x14 | 00010100 | form(0) + value(10) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_read_nsnnwn_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0x14,                                          /* 0_001010_0: small form, value=10 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  uint32_t value = TEST_POISON_U32;
  uint8_t bits_consumed = j2735_internal_inline_read_nsnnwn(unaligned_ptr, 0U, &value);

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(7U, bits_consumed,
                                  "Misaligned: should consume 7 bits for small form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(10U, value, "Misaligned: value should be 10");
}

/* ============================================================================
 * j2735_internal_inline_skip_extensions() Tests
 * ============================================================================
 * Extension additions structure:
 * 1. nsnnwn: extension count - 1 (so 0 means 1 possible extension)
 * 2. bitmap: (count + 1) bits indicating presence
 * 3. For each present: length_det + content
 * ============================================================================
 */

/**
 * @brief Test skip extensions: 1 slot defined, none present.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 0 (small form) → 1 extension slot
 * - bitmap: 0 (1 bit) → extension not present
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field     | Value  |
 * |---------------|-------|-----------|--------|
 * | 0             | 1     | nsn_form  | 0      |
 * | 1             | 6     | ext_count | 000000 |
 * | 7             | 1     | bitmap    | 0      |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                          |
 * |------|------|----------|---------------------------------|
 * | 0    | 0x00 | 00000000 | nsn_form(0) + count(0) + bmp(0) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_one_slot_none_present(void) {
  static const uint8_t buf[] = {
      0x00,                                          /* 0_000000_0: nsnnwn=0, bitmap=0 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = TEST_POISON_U32;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(8U, ext_bits, "Should consume 8 bits (7 nsnnwn + 1 bitmap)");
}

/**
 * @brief Test skip extensions: 1 slot defined, present with 1 byte content.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 0 (small form) → 1 extension slot
 * - bitmap: 1 (1 bit) → extension present
 * - length: 1 byte (short form)
 * - content: 0xAB
 *
 * @par Wire Format (24 bits total):
 * | Offset (bits) | Width | Field     | Value    |
 * |---------------|-------|-----------|----------|
 * | 0             | 1     | nsn_form  | 0        |
 * | 1             | 6     | ext_count | 000000   |
 * | 7             | 1     | bitmap    | 1        |
 * | 8             | 8     | length    | 00000001 |
 * | 16            | 8     | content   | 10101011 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                          |
 * |------|------|----------|---------------------------------|
 * | 0    | 0x01 | 00000001 | nsn_form(0) + count(0) + bmp(1) |
 * | 1    | 0x01 | 00000001 | length(1)                       |
 * | 2    | 0xAB | 10101011 | content(0xAB)                   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_one_slot_present(void) {
  static const uint8_t buf[] = {
      0x01, 0x01, 0xAB,                              /* nsnnwn=0,bitmap=1,len=1,content=0xAB */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = 0U;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(24U, ext_bits, "Should consume 24 bits (7+1+8+8)");
}

/**
 * @brief Test skip extensions: 2 slots defined, both present.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 1 (small form) → 2 extension slots
 * - bitmap: 11 (2 bits) → both present
 * - ext0: length = 1, content = 0xAA (16 bits)
 * - ext1: length = 2, content = 0xBBCC (24 bits)
 *
 * @par Wire Format (49 bits total):
 * | Offset (bits) | Width | Field      | Value            |
 * |---------------|-------|------------|------------------|
 * | 0             | 1     | nsn_form   | 0                |
 * | 1             | 6     | ext_count  | 000001           |
 * | 7             | 2     | bitmap     | 11               |
 * | 9             | 8     | ext0_len   | 00000001         |
 * | 17            | 8     | ext0_data  | 10101010         |
 * | 25            | 8     | ext1_len   | 00000010         |
 * | 33            | 16    | ext1_data  | 1011101111001100 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0x03 | 00000011 | nsn_form(0) + count(1) + bitmap[1]  |
 * | 1    | 0x80 | 10000000 | bitmap[0] + ext0_len[7:1]           |
 * | 2    | 0xD5 | 11010101 | ext0_len[0] + ext0_data[7:1]        |
 * | 3    | 0x01 | 00000001 | ext0_data[0] + ext1_len[7:1]        |
 * | 4    | 0x5D | 01011101 | ext1_len[0] + ext1_data[15:9]       |
 * | 5    | 0xE6 | 11100110 | ext1_data[8:1]                      |
 * | 6    | 0x00 | 00000000 | ext1_data[0] + pad                  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_two_slots_both_present(void) {
  /* Layout: nsnnwn(7) + bitmap(2) + len0(8) + cont0(8) + len1(8) + cont1(16) = 49 bits */
  static const uint8_t buf[] = {
      0x03, 0x80, 0xD5, 0x01, 0x5D, 0xE6, 0x00,      /* computed bitstream */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = 0U;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(49U, ext_bits, "Should consume 49 bits");
}

/**
 * @brief Test skip extensions: 2 slots defined, first only present.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 1 (small form) → 2 extension slots
 * - bitmap: 10 (2 bits) → first present, second absent
 * - ext0: length = 1, content = 0xAA (16 bits)
 *
 * @par Wire Format (25 bits total):
 * | Offset (bits) | Width | Field     | Value    |
 * |---------------|-------|-----------|----------|
 * | 0             | 1     | nsn_form  | 0        |
 * | 1             | 6     | ext_count | 000001   |
 * | 7             | 2     | bitmap    | 10       |
 * | 9             | 8     | ext0_len  | 00000001 |
 * | 17            | 8     | ext0_data | 10101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0x03 | 00000011 | nsn_form(0) + count(1) + bitmap[1]  |
 * | 1    | 0x00 | 00000000 | bitmap[0] + ext0_len[7:1]           |
 * | 2    | 0xD5 | 11010101 | ext0_len[0] + ext0_data[7:1]        |
 * | 3    | 0x00 | 00000000 | ext0_data[0] + pad                  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_two_slots_first_only(void) {
  static const uint8_t buf[] = {
      0x03, 0x00, 0xD5, 0x00,                        /* nsnnwn=1,bitmap=10,len=1,content=0xAA */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = 0U;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(25U, ext_bits, "Should consume 25 bits (7+2+8+8)");
}

/**
 * @brief Test skip extensions: 2 slots defined, second only present.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 1 (small form) → 2 extension slots
 * - bitmap: 01 (2 bits) → first absent, second present
 * - ext1: length = 1, content = 0xBB (16 bits)
 *
 * @par Wire Format (25 bits total):
 * | Offset (bits) | Width | Field     | Value    |
 * |---------------|-------|-----------|----------|
 * | 0             | 1     | nsn_form  | 0        |
 * | 1             | 6     | ext_count | 000001   |
 * | 7             | 2     | bitmap    | 01       |
 * | 9             | 8     | ext1_len  | 00000001 |
 * | 17            | 8     | ext1_data | 10111011 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0x02 | 00000010 | nsn_form(0) + count(1) + bitmap[1]  |
 * | 1    | 0x80 | 10000000 | bitmap[0] + ext1_len[7:1]           |
 * | 2    | 0xDD | 11011101 | ext1_len[0] + ext1_data[7:1]        |
 * | 3    | 0x80 | 10000000 | ext1_data[0] + pad                  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_two_slots_second_only(void) {
  static const uint8_t buf[] = {
      0x02, 0x80, 0xDD, 0x80,                        /* nsnnwn=1,bitmap=01,len=1,content=0xBB */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = 0U;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(25U, ext_bits, "Should consume 25 bits (7+2+8+8)");
}

/**
 * @brief Test skip extensions: empty content (length = 0).
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 0 (small form) → 1 extension slot
 * - bitmap: 1 (1 bit) → present
 * - length: 0 bytes (short form) → empty content
 *
 * @par Wire Format (16 bits total):
 * | Offset (bits) | Width | Field     | Value    |
 * |---------------|-------|-----------|----------|
 * | 0             | 1     | nsn_form  | 0        |
 * | 1             | 6     | ext_count | 000000   |
 * | 7             | 1     | bitmap    | 1        |
 * | 8             | 8     | length    | 00000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                          |
 * |------|------|----------|---------------------------------|
 * | 0    | 0x01 | 00000001 | nsn_form(0) + count(0) + bmp(1) |
 * | 1    | 0x00 | 00000000 | length(0)                       |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_empty_content(void) {
  static const uint8_t buf[] = {
      0x01, 0x00, /* 0_000000 1_0000000 0: nsnnwn=0,bitmap=1,len=0 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = 0U;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(16U, ext_bits, "Should consume 16 bits (7+1+8+0)");
}

/**
 * @brief Test skip extensions: nsnnwn parse error.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): large form with fragmented length — returns error
 * - Expected: ext_bits = 0, result = -1
 *
 * @par Wire Format (3+ bits total):
 * | Offset (bits) | Width | Field          | Value |
 * |---------------|-------|----------------|-------|
 * | 0             | 1     | nsn_prefix     | 1     |
 * | 1             | 2     | length_det_tag | 11    |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0xE0 | 11100000 | nsn_prefix(1) + len_det(11) + pad   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_nsnnwn_error(void) {
  static const uint8_t buf[] = {
      0xE0,                                          /* 1_1100000: large nsnnwn + fragmented len */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = TEST_POISON_U32;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, result, "Should return error");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, ext_bits, "Ext bits should be 0 on error");
}

/**
 * @brief Test skip extensions: length determinant error in open type.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 0 (small form) → 1 extension slot
 * - bitmap: 1 (1 bit) → extension present
 * - length: fragmented form (bits[0:1] = 11) — returns error
 * - Expected: ext_bits = 0, result = -1
 *
 * @par Wire Format (10+ bits total):
 * | Offset (bits) | Width | Field      | Value  |
 * |---------------|-------|------------|--------|
 * | 0             | 1     | nsn_form   | 0      |
 * | 1             | 6     | ext_count  | 000000 |
 * | 7             | 1     | bitmap     | 1      |
 * | 8             | 2     | len_det_tag| 11     |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0x01 | 00000001 | nsn_form(0) + count(0) + bmp(1)     |
 * | 1    | 0xE0 | 11100000 | len_det(11) + pad                   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_length_error(void) {
  static const uint8_t buf[] = {
      0x01, 0xE0, /* 0_000000 1_1110000 0: nsnnwn=0,bitmap=1,fragmented len */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = TEST_POISON_U32;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, result, "Should return error");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, ext_bits, "Ext bits should be 0 on error");
}

/**
 * @brief Test skip extensions: non-zero start offset.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Bit offset: 5
 * - ext-count (nsnnwn): 0 (small form) → 1 extension slot
 * - bitmap: 0 (1 bit) → extension not present
 *
 * @par Wire Format (8 bits at offset 5):
 * | Offset (bits) | Width | Field     | Value  |
 * |---------------|-------|-----------|--------|
 * | 0             | 5     | padding   | 11111  |
 * | 5             | 1     | nsn_form  | 0      |
 * | 6             | 6     | ext_count | 000000 |
 * | 12            | 1     | bitmap    | 0      |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0xF8 | 11111000 | pad(11111) + nsn_form(0) + count[5:4]|
 * | 1    | 0x00 | 00000000 | count[3:0] + bmp(0) + pad           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_nonzero_offset(void) {
  static const uint8_t buf[] = {
      0xF8, 0x00, /* 11111_0_00 0000_0_000: padding + nsnnwn=0,bitmap=0 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = 0U;
  int result = j2735_internal_inline_skip_extensions(buf, TEST_NONZERO_BIT_OFFSET, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(8U, ext_bits, "Should consume 8 bits (7+1)");
}

/**
 * @brief Test skip extensions: ext_count > 63 returns error.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - ext-count (nsnnwn): 64 (large form) — exceeds 63-bit bitmap, returns error
 * - Expected: ext_bits = 0, result = -1
 *
 * @par Wire Format (17 bits total):
 * | Offset (bits) | Width | Field      | Value    |
 * |---------------|-------|------------|----------|
 * | 0             | 1     | nsn_prefix | 1        |
 * | 1             | 8     | length_det | 00000001 |
 * | 9             | 8     | value      | 01000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0x80 | 10000000 | nsn_prefix(1) + length_det[7:1]     |
 * | 1    | 0xA0 | 10100000 | length_det[0] + value[7:1]          |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_too_many_extensions(void) {
  /* Large form nsnnwn encoding value 64:
   * bit[0] = 1 (large form)
   * length_det = short form, length = 1 byte
   * value = 64 = 0x40
   * Binary: 1_00000001_01000000 = 0x80, 0xA0 */
  static const uint8_t buf[] = {
      0x80, 0xA0,                                    /* large form nsnnwn = 64 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = TEST_POISON_U32;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, result, "Should return error for ext_count > 63");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, ext_bits, "Ext bits should be 0 on error");
}

/**
 * @brief Test skip_extensions with deliberately misaligned buffer pointer.
 *
 * @par ASN.1 Definition:
 * @code
 * -- X.691 §18.8 Extension Additions
 * extension-additions ::= SEQUENCE {
 *   ext-count   nsnnwn,                        -- number of extensions - 1
 *   bitmap      BIT STRING (SIZE(ext-count+1)),-- presence bitmap
 *   extensions  SEQUENCE (SIZE(0..ext-count)) OF SEQUENCE {
 *     length      length-determinant,          -- open type wrapper
 *     content     OCTET STRING (SIZE(length))  -- extension content
 *   }
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Buffer: &payload[1] (deliberately misaligned)
 * - ext-count (nsnnwn): 0 (small form) → 1 extension slot
 * - bitmap: 1 (1 bit) → extension present
 * - length: 1 byte (short form)
 * - content: 0xAB
 *
 * @par Wire Format (24 bits total):
 * | Offset (bits) | Width | Field     | Value    |
 * |---------------|-------|-----------|----------|
 * | 0             | 1     | nsn_form  | 0        |
 * | 1             | 6     | ext_count | 000000   |
 * | 7             | 1     | bitmap    | 1        |
 * | 8             | 8     | length    | 00000001 |
 * | 16            | 8     | content   | 10101011 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                          |
 * |------|------|----------|---------------------------------|
 * | 0    | 0xFF | 11111111 | junk (misalignment)             |
 * | 1    | 0x01 | 00000001 | nsn_form(0) + count(0) + bmp(1) |
 * | 2    | 0x01 | 00000001 | length(1)                       |
 * | 3    | 0xAB | 10101011 | content(0xAB)                   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_misaligned_access(void) {
  /* Same encoding as test_inline_skip_extensions_one_slot_present:
   * nsnnwn small form: 0_000000 = ext_count 0 → 1 slot
   * bitmap: 1 = extension[0] present
   * length_det short form: 0_0000001 = 1 byte
   * extension content: 0xAB (8 bits)
   * Binary: 0000000_1_00000001_10101011
   * Bytes:  00000001 00000001 10101011
   *         0x01     0x01     0xAB       */
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0x01,                                          /* nsnnwn=0 + bitmap=1 */
      0x01,                                          /* length_det=00000001 (1 byte) */
      0xAB,                                          /* extension content (8 bits) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  uint32_t ext_bits = TEST_POISON_U32;
  int result = j2735_internal_inline_skip_extensions(unaligned_ptr, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Misaligned: should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(24U, ext_bits, "Misaligned: should consume 24 bits (7+1+8+8)");
}

/* ============================================================================
 * Test Suite Runner
 * ============================================================================
 */

void run_testsuite_uper(void) {
  /* j2735_internal_inline_read_length_determinant tests */
  RUN_TEST(test_inline_read_length_determinant_short_form_min);
  RUN_TEST(test_inline_read_length_determinant_short_form_max);
  RUN_TEST(test_inline_read_length_determinant_short_form_typical);
  RUN_TEST(test_inline_read_length_determinant_long_form_min);
  RUN_TEST(test_inline_read_length_determinant_long_form_128);
  RUN_TEST(test_inline_read_length_determinant_long_form_max);
  RUN_TEST(test_inline_read_length_determinant_fragmented_error);
  RUN_TEST(test_inline_read_length_determinant_nonzero_bit_offset);
  RUN_TEST(test_inline_read_length_determinant_misaligned_access);

  /* j2735_internal_inline_read_nsnnwn tests */
  RUN_TEST(test_inline_read_nsnnwn_small_form_min);
  RUN_TEST(test_inline_read_nsnnwn_small_form_max);
  RUN_TEST(test_inline_read_nsnnwn_small_form_typical);
  RUN_TEST(test_inline_read_nsnnwn_large_form_64);
  RUN_TEST(test_inline_read_nsnnwn_large_form_100);
  RUN_TEST(test_inline_read_nsnnwn_large_form_255);
  RUN_TEST(test_inline_read_nsnnwn_large_form_256);
  RUN_TEST(test_inline_read_nsnnwn_large_form_65535);
  RUN_TEST(test_inline_read_nsnnwn_large_form_4_bytes);
  RUN_TEST(test_inline_read_nsnnwn_large_form_5_bytes_error);
  RUN_TEST(test_inline_read_nsnnwn_fragmented_error);
  RUN_TEST(test_inline_read_nsnnwn_nonzero_bit_offset);
  RUN_TEST(test_inline_read_nsnnwn_misaligned_access);

  /* j2735_internal_inline_skip_extensions tests */
  RUN_TEST(test_inline_skip_extensions_one_slot_none_present);
  RUN_TEST(test_inline_skip_extensions_one_slot_present);
  RUN_TEST(test_inline_skip_extensions_two_slots_both_present);
  RUN_TEST(test_inline_skip_extensions_two_slots_first_only);
  RUN_TEST(test_inline_skip_extensions_two_slots_second_only);
  RUN_TEST(test_inline_skip_extensions_empty_content);
  RUN_TEST(test_inline_skip_extensions_nsnnwn_error);
  RUN_TEST(test_inline_skip_extensions_length_error);
  RUN_TEST(test_inline_skip_extensions_nonzero_offset);
  RUN_TEST(test_inline_skip_extensions_too_many_extensions);
  RUN_TEST(test_inline_skip_extensions_misaligned_access);
}
