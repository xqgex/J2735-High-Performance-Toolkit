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
 * @brief Tests for UserSizeAndBehaviour extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * UserSizeAndBehaviour ::= BIT STRING {
 *     unavailable   (0),
 *     smallStature  (1),
 *     largeStature  (2),
 *     erraticMoving (3),
 *     slowMoving    (4)
 * } (SIZE (5, ...))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Non-extended form (6 bits): [ext=0][5 flag bits]
 * - Extended form (13 bits): [ext=1][nsnnwn=7 bits][5 flag bits]
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB (unavailable)
 * - ASN.1 bit 4 = rightmost (slowMoving)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_UserSizeAndBehaviour.h"
#include "J2735_internal_DE_UserSizeAndBehaviour_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test UserSizeAndBehaviour with non-extended form, typical value.
 *
 * @par Test Vector:
 * - Extended: NO (root form)
 * - Flags: 0x15 = 10101 (5 bits: unavailable=1,small=0,large=1,erratic=0,slow=1)
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field    | Value |
 * |---------------|-------|----------|-------|
 * | 0             | 1     | ext_bit  | 0     |
 * | 1             | 5     | flags    | 10101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x54 | 01010100 | ext(0)+flags(10101)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_non_extended(void) {
  /* Wire: [ext=0][10101] = 0 10101 xx = 01010100 = 0x54 */
  static const uint8_t payload[] = {
      0x54,                                          /* ext(0)+flags(10101)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Extension bit should be 0 for non-extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x15U, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "Flags should be 0x15 for non-extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(6U, J2735_USER_SIZE_AND_BEHAVIOUR_SIZE(payload),
                                   "Size should be 6 for non-extended form");
}

/**
 * @brief Test UserSizeAndBehaviour with extended form, all flags ON.
 *
 * @par Test Vector:
 * - Extended: YES
 * - nsnnwn value: 5 (small form: 0 + 000101 = 0b0000101)
 * - Flags: 0x1F (all 5 bits set)
 *
 * @par Wire Format (13 bits total):
 * | Offset (bits) | Width | Field    | Value              |
 * |---------------|-------|----------|--------------------|
 * | 0             | 1     | ext_bit  | 1                  |
 * | 1             | 7     | nsnnwn   | 0000101 (=5)       |
 * | 8             | 5     | flags    | 11111              |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x82 | 10000010 | ext(1)+nsnnwn(0000101)       |
 * | 1    | 0xF8 | 11111000 | flags(11111)+pad(3)          |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_extended(void) {
  /* nsnnwn small form for 5: 0 + 000101 = 0b0000101
   * Wire: [ext=1][0000101][11111]
   * = 1 0000101 11111 xxx = 10000101 11111000 = 0x85 0xF8
   *
   * Wait — need to recalculate nsnnwn encoding.
   * nsnnwn for "normally small non-negative whole number":
   *   If value <= 63: [0][6-bit value]
   *   value = 5: [0][000101] = 0000101
   * Wire: [1][0000101][11111] = 1_0000101_11111_xxx
   * byte 0: 10000101 = 0x85
   * byte 1: 11111xxx = 0xF8
   */
  static const uint8_t payload[] = {
      0x85,                                          /* ext(1)+nsnnwn(0000101) */
      0xF8,                                          /* flags(11111)+pad(3) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Extension bit should be 1 for extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x1FU, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "Flags should be 0x1F for extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(13U, J2735_USER_SIZE_AND_BEHAVIOUR_SIZE(payload),
                                   "Size should be 13 for extended form");
}

/**
 * @brief Test UserSizeAndBehaviour individual flag accessors (non-extended, all ON).
 *
 * @par Test Vector:
 * - Extended: NO
 * - Flags: 0x1F = 11111 (all 5 flags ON)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x7C | 01111100 | ext(0)+flags(11111)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_non_extended_flags(void) {
  /* Wire: [ext=0][11111] = 0 11111 xx = 01111100 = 0x7C */
  static const uint8_t payload[] = {
      0x7C,                                          /* ext(0)+flags(11111)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SMALL_STATURE(payload),
                                  "bit 1: smallStature should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_LARGE_STATURE(payload),
                                  "bit 2: largeStature should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_ERRATIC_MOVING(payload),
                                  "bit 3: erraticMoving should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SLOW_MOVING(payload),
                                  "bit 4: slowMoving should be ON");
}

/**
 * @brief Test UserSizeAndBehaviour SIZE macro for non-extended form.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x00 | 00000000 | ext(0)+flags(00000)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_size_non_extended(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* ext(0)+flags(00000)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(6U, J2735_USER_SIZE_AND_BEHAVIOUR_SIZE(payload),
                                   "Non-extended size should be 6 bits");
}

/**
 * @brief Test UserSizeAndBehaviour SIZE macro for extended form.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x85 | 10000101 | ext(1)+nsnnwn(0000101)       |
 * | 1    | 0x00 | 00000000 | flags(00000)+pad(3)          |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_size_extended(void) {
  static const uint8_t payload[] = {
      0x85,                                          /* ext(1)+nsnnwn(0000101) */
      0x00,                                          /* flags(00000)+pad(3) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(13U, J2735_USER_SIZE_AND_BEHAVIOUR_SIZE(payload),
                                   "Extended size should be 13 bits");
}

/**
 * @brief Test UserSizeAndBehaviour with all zeros, non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x00 | 00000000 | ext(0)+flags(00000)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_all_zeros_non_extended(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* ext(0)+flags(00000)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "All flags should be zero");
}

/**
 * @brief Test UserSizeAndBehaviour with all root flags ON, non-extended.
 *
 * @par Test Vector:
 * - Flags: 0x1F = 11111
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x7C | 01111100 | ext(0)+flags(11111)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_non_extended_all_flags_on(void) {
  static const uint8_t payload[] = {
      0x7C,                                          /* ext(0)+flags(11111)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x1FU, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "All 5 flags should be ON (0x1F)");
}

/**
 * @brief Test UserSizeAndBehaviour with extended form, all flags zero.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x85 | 10000101 | ext(1)+nsnnwn(0000101)       |
 * | 1    | 0x00 | 00000000 | flags(00000)+pad(3)          |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_extended_all_zeros(void) {
  static const uint8_t payload[] = {
      0x85,                                          /* ext(1)+nsnnwn(0000101) */
      0x00,                                          /* flags(00000)+pad(3) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Should be extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "All flags should be zero in extended form");
}

/**
 * @brief Test UserSizeAndBehaviour alternating pattern 10101 (0x15), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x54 | 01010100 | ext(0)+flags(10101)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_non_extended_alternating_10101(void) {
  /* Wire: [ext=0][10101] = 0 10101 xx = 01010100 = 0x54 */
  static const uint8_t payload[] = {
      0x54,                                          /* ext(0)+flags(10101)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x15U, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "Alternating pattern should be 0x15");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SMALL_STATURE(payload),
                                  "bit 1: smallStature should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_LARGE_STATURE(payload),
                                  "bit 2: largeStature should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_ERRATIC_MOVING(payload),
                                  "bit 3: erraticMoving should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SLOW_MOVING(payload),
                                  "bit 4: slowMoving should be ON");
}

/**
 * @brief Test UserSizeAndBehaviour alternating pattern 01010 (0x0A), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x28 | 00101000 | ext(0)+flags(01010)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_non_extended_alternating_01010(void) {
  /* Wire: [ext=0][01010] = 0 01010 xx = 00101000 = 0x28 */
  static const uint8_t payload[] = {
      0x28,                                          /* ext(0)+flags(01010)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x0AU, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "Inverse alternating pattern should be 0x0A");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SMALL_STATURE(payload),
                                  "bit 1: smallStature should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_LARGE_STATURE(payload),
                                  "bit 2: largeStature should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_ERRATIC_MOVING(payload),
                                  "bit 3: erraticMoving should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SLOW_MOVING(payload),
                                  "bit 4: slowMoving should be OFF");
}

/**
 * @brief Test UserSizeAndBehaviour single bit 0 (unavailable), non-extended.
 *
 * @par Test Vector:
 * - Flags: 0x10 = 10000 (only unavailable ON)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x40 | 01000000 | ext(0)+flags(10000)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_single_bit_0_unavailable(void) {
  /* Wire: [ext=0][10000] = 0 10000 xx = 01000000 = 0x40 */
  static const uint8_t payload[] = {
      0x40,                                          /* ext(0)+flags(10000)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x10U, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "Only bit 0 should be set (0x10)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_UNAVAILABLE(payload),
                                  "unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SLOW_MOVING(payload),
                                  "slowMoving should be OFF");
}

/**
 * @brief Test UserSizeAndBehaviour single bit 4 (slowMoving), non-extended.
 *
 * @par Test Vector:
 * - Flags: 0x01 = 00001 (only slowMoving ON)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x04 | 00000100 | ext(0)+flags(00001)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_single_bit_4_slow_moving(void) {
  /* Wire: [ext=0][00001] = 0 00001 xx = 00000100 = 0x04 */
  static const uint8_t payload[] = {
      0x04,                                          /* ext(0)+flags(00001)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_USER_SIZE_AND_BEHAVIOUR_GET(payload),
                                 "Only bit 4 should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_UNAVAILABLE(payload),
                                  "unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_USER_SIZE_AND_BEHAVIOUR_GET_SLOW_MOVING(payload),
                                  "slowMoving should be ON");
}

/**
 * @brief Test UserSizeAndBehaviour with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - Extended: NO
 * - Flags: 0x1F (all 5 flags ON)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x7C | 01111100 | ext(0)+flags(11111)+pad(2)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_user_size_and_behaviour_misaligned_access(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* junk byte for misalignment */
      0x7C,                                          /* ext(0)+flags(11111)+pad(2) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  bool has_ext = J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(unaligned_ptr);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Misaligned: should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x1FU, J2735_USER_SIZE_AND_BEHAVIOUR_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all UserSizeAndBehaviour tests.
 */
void run_testsuite_user_size_and_behaviour(void) {
  RUN_TEST(test_user_size_and_behaviour_non_extended);
  RUN_TEST(test_user_size_and_behaviour_extended);
  RUN_TEST(test_user_size_and_behaviour_non_extended_flags);
  RUN_TEST(test_user_size_and_behaviour_size_non_extended);
  RUN_TEST(test_user_size_and_behaviour_size_extended);
  RUN_TEST(test_user_size_and_behaviour_all_zeros_non_extended);
  RUN_TEST(test_user_size_and_behaviour_non_extended_all_flags_on);
  RUN_TEST(test_user_size_and_behaviour_extended_all_zeros);
  RUN_TEST(test_user_size_and_behaviour_non_extended_alternating_10101);
  RUN_TEST(test_user_size_and_behaviour_non_extended_alternating_01010);
  RUN_TEST(test_user_size_and_behaviour_single_bit_0_unavailable);
  RUN_TEST(test_user_size_and_behaviour_single_bit_4_slow_moving);
  RUN_TEST(test_user_size_and_behaviour_misaligned_access);
}
