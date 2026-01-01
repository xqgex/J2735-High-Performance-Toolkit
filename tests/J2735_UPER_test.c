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

#include <inttypes.h>
#include <stdbool.h>
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
 * @par Encoding:
 * - Short form: bit[0] = 0, bits[1:7] = 0
 * - Binary: 00000000
 *
 * @par Expected:
 * - Length: 0
 * - Bits consumed: 8
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
 * @par Encoding:
 * - Short form: bit[0] = 0, bits[1:7] = 127
 * - Binary: 01111111
 *
 * @par Expected:
 * - Length: 127
 * - Bits consumed: 8
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
 * @par Encoding:
 * - Short form: bit[0] = 0, bits[1:7] = 1
 * - Binary: 00000001
 *
 * @par Expected:
 * - Length: 1
 * - Bits consumed: 8
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
 * @par Encoding:
 * - Long form: bits[0:1] = 10, bits[2:15] = 0
 * - Binary: 10000000 00000000
 *
 * @par Expected:
 * - Length: 0
 * - Bits consumed: 16
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
 * @par Encoding:
 * - Long form: bits[0:1] = 10, bits[2:15] = 128
 * - Binary: 10000000 10000000
 *
 * @par Expected:
 * - Length: 128
 * - Bits consumed: 16
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
 * @par Encoding:
 * - Long form: bits[0:1] = 10, bits[2:15] = 16383
 * - Binary: 10111111 11111111
 *
 * @par Expected:
 * - Length: 16383
 * - Bits consumed: 16
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
 * @par Encoding:
 * - Fragmented: bits[0:1] = 11
 * - Binary: 11xxxxxx...
 *
 * @par Expected:
 * - Length: 0 (error)
 * - Bits consumed: 0 (error)
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
 * @par Encoding:
 * - 3 bits padding + short form length=42
 * - Binary: xxx_00101010 (short form 0 + 42)
 *
 * @par Expected:
 * - Length: 42
 * - Bits consumed: 8
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
 * @par Encoding:
 * - Small form: bit[0] = 0, bits[1:6] = 0
 * - Binary: 0_000000
 *
 * @par Expected:
 * - Value: 0
 * - Bits consumed: 7
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
 * @par Encoding:
 * - Small form: bit[0] = 0, bits[1:6] = 63
 * - Binary: 0_111111
 *
 * @par Expected:
 * - Value: 63
 * - Bits consumed: 7
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
 * @par Encoding:
 * - Small form: bit[0] = 0, bits[1:6] = 5
 * - Binary: 0_000101
 *
 * @par Expected:
 * - Value: 5
 * - Bits consumed: 7
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
 * @par Encoding:
 * - Large form: 1 + len_det(1 octet) + value(0x40)
 * - Binary: 1_0000000_1 01000000
 *           ^         ^ ^
 *           large     len=1 value=64
 *
 * @par Expected:
 * - Value: 64
 * - Bits consumed: 1 + 8 + 8 = 17
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
 * @par Encoding:
 * - Large form: 1 + len_det(1 octet) + value(0x64)
 * - Binary: 1_0000000_1 01100100
 *
 * @par Expected:
 * - Value: 100
 * - Bits consumed: 17
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
 * @par Encoding:
 * - Large form: 1 + len_det(1 octet) + value(0xFF)
 * - Binary: 1_0000000_1 11111111
 *
 * @par Expected:
 * - Value: 255
 * - Bits consumed: 17
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
 * @par Encoding:
 * - Large form: 1 + len_det(2 octets) + value(0x0100)
 * - Binary: 1_0000001_0 00000001 00000000
 *           ^       ^           ^
 *           large   len=2       value=256
 *
 * @par Expected:
 * - Value: 256
 * - Bits consumed: 1 + 8 + 16 = 25
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
 * @par Encoding:
 * - Large form: 1 + len_det(2 octets) + value(0xFFFF)
 *
 * @par Expected:
 * - Value: 65535
 * - Bits consumed: 25
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
 * @par Encoding:
 * - Large form: 1 + len_det(4 octets) + value(0x12345678)
 *
 * @par Expected:
 * - Value: 0x12345678
 * - Bits consumed: 1 + 8 + 32 = 41
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
 * @par Encoding:
 * - Large form: 1 + len_det(5 octets) + ...
 *
 * @par Expected:
 * - Value: 0 (error)
 * - Bits consumed: 0 (error)
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
 * @par Encoding:
 * - Large form: 1 + fragmented_len_det (11_...)
 *
 * @par Expected:
 * - Value: 0 (error)
 * - Bits consumed: 0 (error)
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
 * @par Encoding:
 * - 3 bits padding + small form value=31
 * - Binary: xxx_0_011111 (7 bits: small form 0 + 31)
 *
 * @par Layout (10 bits):
 * - Bits 0-2: 111 (padding)
 * - Bit 3: 0 (small form)
 * - Bits 4-9: 011111 (value 31)
 * - Bytes: 0xE7, 0xC0 = 1110_0111 1100_0000
 *
 * @par Expected:
 * - Value: 31
 * - Bits consumed: 7
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
 * @par Encoding:
 * - nsnnwn = 0 (small form, 7 bits) → 1 extension slot
 * - bitmap = 0 (1 bit) → extension not present
 *
 * @par Expected:
 * - Extension bits: 7 + 1 = 8
 * - Result: 0 (success)
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
 * @par Encoding:
 * - nsnnwn = 0 (7 bits) → 1 extension slot
 * - bitmap = 1 (1 bit) → extension present
 * - length = 1 (8 bits, short form)
 * - content = 0xAB (8 bits)
 *
 * @par Layout (24 bits):
 * - Bits 0-6:   0_000000 (nsnnwn=0)
 * - Bit 7:      1 (bitmap=1)
 * - Bits 8-15:  0_0000001 (length=1)
 * - Bits 16-23: 10101011 (content=0xAB)
 * - Byte 0: 0_000000_1 = 0x01
 * - Byte 1: 0_0000001  = 0x01
 * - Byte 2: 10101011   = 0xAB
 *
 * @par Expected:
 * - Extension bits: 7 + 1 + 8 + 8 = 24
 * - Result: 0 (success)
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
 * @par Encoding:
 * - nsnnwn = 1 (7 bits) → 2 extension slots
 * - bitmap = 11 (2 bits) → both present
 * - ext0: length = 1, content = 0xAA (16 bits)
 * - ext1: length = 2, content = 0xBBCC (24 bits)
 *
 * @par Layout (49 bits):
 * - Bit stream: 0000001 11 00000001 10101010 00000010 1011101111001100
 * - Bytes: 0x03 0x80 0xD5 0x01 0x5D 0xE6 0x00
 *
 * @par Expected:
 * - Extension bits: 7 + 2 + 16 + 24 = 49
 * - Result: 0 (success)
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
 * @par Encoding:
 * - nsnnwn = 1 (7 bits) → 2 extension slots
 * - bitmap = 10 (2 bits) → first present, second absent
 * - ext0: length = 1, content = 0xAA (16 bits)
 *
 * @par Expected:
 * - Extension bits: 7 + 2 + 16 = 25
 * - Result: 0 (success)
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_two_slots_first_only(void) {
  static const uint8_t buf[] = {
      0x02, 0x80, 0xAA, /* 0_000001 0_1_000000 0_1010101 0: nsnnwn=1,bitmap=10,len=1,cont */
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
 * @par Encoding:
 * - nsnnwn = 1 (7 bits) → 2 extension slots
 * - bitmap = 01 (2 bits) → first absent, second present
 * - ext1: length = 1, content = 0xBB (16 bits)
 *
 * @par Layout (25 bits):
 * - Bit stream: 0000001 01 00000001 10111011
 * - Bytes: 0x02 0x80 0xDD 0x80
 *
 * @par Expected:
 * - Extension bits: 7 + 2 + 16 = 25
 * - Result: 0 (success)
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
 * @par Encoding:
 * - nsnnwn = 0 (7 bits) → 1 extension slot
 * - bitmap = 1 (1 bit) → present
 * - length = 0 (8 bits) → empty content
 *
 * @par Expected:
 * - Extension bits: 7 + 1 + 8 + 0 = 16
 * - Result: 0 (success)
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
 * @par Encoding:
 * - nsnnwn large form with fragmented length
 *
 * @par Expected:
 * - Extension bits: 0 (error)
 * - Result: -1 (error)
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
 * @par Encoding:
 * - nsnnwn = 0, bitmap = 1 (extension present)
 * - length = fragmented (error)
 *
 * @par Expected:
 * - Extension bits: 0 (error)
 * - Result: -1 (error)
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
 * @par Encoding:
 * - 5 bits padding + nsnnwn = 0, bitmap = 0
 *
 * @par Expected:
 * - Extension bits: 8
 * - Result: 0 (success)
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
 * @par Encoding:
 * - Large form nsnnwn with value 64 (too many extensions for 64-bit bitmap)
 *
 * @par Expected:
 * - Result: -1 (error, ext_count > 63 exceeds bitmap capacity)
 * - Extension bits: 0
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_inline_skip_extensions_too_many_extensions(void) {
  /* Large form nsnnwn encoding value 64:
   * bit[0] = 1 (large form)
   * length_det = short form, length = 1 byte
   * value = 64 = 0x40
   * Binary: 1_00000001_01000000 = 0x81, 0x40 */
  static const uint8_t buf[] = {
      0x81, 0x40,                                    /* large form nsnnwn = 64 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint32_t ext_bits = TEST_POISON_U32;
  int result = j2735_internal_inline_skip_extensions(buf, 0U, &ext_bits);

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, result, "Should return error for ext_count > 63");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, ext_bits, "Ext bits should be 0 on error");
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
}
