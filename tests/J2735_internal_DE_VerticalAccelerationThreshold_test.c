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
 * @brief Tests for VerticalAccelerationThreshold non-extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * VerticalAccelerationThreshold ::= BIT STRING {
 *     notEquipped (0),
 *     leftFront   (1),
 *     leftRear    (2),
 *     rightFront  (3),
 *     rightRear   (4)
 * } (SIZE (5))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Fixed form (5 bits): [F0 F1 F2 F3 F4]
 * - No extension marker (non-extensible type)
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB of BIT STRING content (notEquipped)
 * - ASN.1 bit 4 = rightmost (rightRear)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_VerticalAccelerationThreshold.h"
#include "J2735_internal_DE_VerticalAccelerationThreshold_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test VerticalAccelerationThreshold with all flags OFF.
 *
 * @par ASN.1 Definition:
 * @code
 * VerticalAccelerationThreshold ::= BIT STRING {
 *     notEquipped (0), leftFront (1), leftRear (2),
 *     rightFront (3), rightRear (4)
 * } (SIZE (5))
 * @endcode
 *
 * @par Test Vector:
 * - All 5 flags: OFF (0)
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 00000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:4]=00000 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_all_zeros(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:4]=00000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(payload),
                                 "All flags should be zero");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(payload),
                                  "notEquipped should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_FRONT(payload),
                                  "leftFront should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(payload),
                                  "rightRear should be OFF");
}

/**
 * @brief Test VerticalAccelerationThreshold with all flags ON (0x1F).
 *
 * @par ASN.1 Definition:
 * @code
 * VerticalAccelerationThreshold ::= BIT STRING {
 *     notEquipped (0), leftFront (1), leftRear (2),
 *     rightFront (3), rightRear (4)
 * } (SIZE (5))
 * @endcode
 *
 * @par Test Vector:
 * - All 5 flags: ON (1)
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 11111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0xF8 | 11111000 | flags[0:4]=11111 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_all_ones(void) {
  static const uint8_t payload[] = {
      0xF8,                                          /* flags[0:4]=11111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x1FU, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(payload),
                                 "All flags should be ON (0x1F)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(payload),
                                  "notEquipped should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_FRONT(payload),
                                  "leftFront should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_REAR(payload),
                                  "leftRear should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_FRONT(payload),
                                  "rightFront should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(payload),
                                  "rightRear should be ON");
}

/**
 * @brief Test VerticalAccelerationThreshold with alternating pattern 10101 (0x15).
 *
 * @par Test Vector:
 * - notEquipped(0): ON, leftFront(1): OFF, leftRear(2): ON,
 *   rightFront(3): OFF, rightRear(4): ON
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 10101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0xA8 | 10101000 | flags[0:4]=10101 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_alternating_10101(void) {
  static const uint8_t payload[] = {
      0xA8,                                          /* flags[0:4]=10101 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x15U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(payload),
                                 "Alternating pattern should be 0x15");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(payload),
                                  "bit 0: notEquipped should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_FRONT(payload),
                                  "bit 1: leftFront should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_REAR(payload),
                                  "bit 2: leftRear should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_FRONT(payload),
                                  "bit 3: rightFront should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(payload),
                                  "bit 4: rightRear should be ON");
}

/**
 * @brief Test VerticalAccelerationThreshold with alternating pattern 01010 (0x0A).
 *
 * @par Test Vector:
 * - notEquipped(0): OFF, leftFront(1): ON, leftRear(2): OFF,
 *   rightFront(3): ON, rightRear(4): OFF
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 01010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x50 | 01010000 | flags[0:4]=01010 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_alternating_01010(void) {
  static const uint8_t payload[] = {
      0x50,                                          /* flags[0:4]=01010 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x0AU, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(payload),
                                 "Inverse alternating pattern should be 0x0A");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(payload),
                                  "bit 0: notEquipped should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_FRONT(payload),
                                  "bit 1: leftFront should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_LEFT_REAR(payload),
                                  "bit 2: leftRear should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_FRONT(payload),
                                  "bit 3: rightFront should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(payload),
                                  "bit 4: rightRear should be OFF");
}

/**
 * @brief Test VerticalAccelerationThreshold with only notEquipped (bit 0) set.
 *
 * @par Test Vector:
 * - notEquipped(0): ON, all others: OFF
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 10000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x80 | 10000000 | flags[0:4]=10000 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_single_bit_not_equipped(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* flags[0:4]=10000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x10U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(payload),
                                 "Only notEquipped should be set (0x10)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(payload),
                                  "notEquipped should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(payload),
                                  "rightRear should be OFF");
}

/**
 * @brief Test VerticalAccelerationThreshold with only rightRear (bit 4) set.
 *
 * @par Test Vector:
 * - rightRear(4): ON, all others: OFF
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 00001 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x08 | 00001000 | flags[0:4]=00001 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_single_bit_right_rear(void) {
  static const uint8_t payload[] = {
      0x08,                                          /* flags[0:4]=00001 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(payload),
                                 "Only rightRear should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(payload),
                                  "notEquipped should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(payload),
                                  "rightRear should be ON");
}

/**
 * @brief Test VerticalAccelerationThreshold SIZE macro returns fixed 5 bits.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_size(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:4]=00000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(5U, J2735_VERTICAL_ACCELERATION_THRESHOLD_SIZE(payload),
                                   "SIZE should always be 5 for VerticalAccelerationThreshold");
}

/**
 * @brief Test VerticalAccelerationThreshold HAS_EXTENSION always returns false.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_has_extension(void) {
  static const uint8_t payload[] = {
      0xF8,                                          /* flags[0:4]=11111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_VERTICAL_ACCELERATION_THRESHOLD_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext,
                            "HAS_EXTENSION should always be false for non-extensible type");
}

/**
 * @brief Test VerticalAccelerationThreshold with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - All 5 flags: ON (0x1F)
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field       | Value |
 * |---------------|-------|-------------|-------|
 * | 0             | 5     | flags[0:4]  | 11111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0xF8 | 11111000 | flags[0:4]=11111 + pad(3) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vertical_acceleration_threshold_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0xF8,                                          /* flags[0:4]=11111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x1FU, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON (0x1F)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_NOT_EQUIPPED(unaligned_ptr),
      "Misaligned: notEquipped should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VERTICAL_ACCELERATION_THRESHOLD_GET_RIGHT_REAR(unaligned_ptr),
      "Misaligned: rightRear should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all VerticalAccelerationThreshold tests.
 */
void run_testsuite_vertical_acceleration_threshold(void) {
  RUN_TEST(test_vertical_acceleration_threshold_all_zeros);
  RUN_TEST(test_vertical_acceleration_threshold_all_ones);
  RUN_TEST(test_vertical_acceleration_threshold_alternating_10101);
  RUN_TEST(test_vertical_acceleration_threshold_alternating_01010);
  RUN_TEST(test_vertical_acceleration_threshold_single_bit_not_equipped);
  RUN_TEST(test_vertical_acceleration_threshold_single_bit_right_rear);
  RUN_TEST(test_vertical_acceleration_threshold_size);
  RUN_TEST(test_vertical_acceleration_threshold_has_extension);
  RUN_TEST(test_vertical_acceleration_threshold_misaligned_access);
}
