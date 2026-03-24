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
 * @brief Tests for AllowedManeuvers non-extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * AllowedManeuvers ::= BIT STRING {
 *     maneuverStraightAllowed       (0),
 *     maneuverLeftAllowed           (1),
 *     maneuverRightAllowed          (2),
 *     maneuverUTurnAllowed          (3),
 *     maneuverLeftTurnOnRedAllowed  (4),
 *     maneuverRightTurnOnRedAllowed (5),
 *     maneuverLaneChangeAllowed     (6),
 *     maneuverNoStoppingAllowed     (7),
 *     yieldAllwaysRequired          (8),
 *     goWithHalt                    (9),
 *     caution                       (10),
 *     reserved1                     (11)
 * } (SIZE (12))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Fixed form (12 bits): [F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11]
 * - No extension marker (non-extensible type)
 * - Spans 2 bytes (8 bits in byte 0, 4 bits in byte 1)
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0  = leftmost/MSB (maneuverStraightAllowed)
 * - ASN.1 bit 11 = rightmost (reserved1)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_AllowedManeuvers.h"
#include "J2735_internal_DE_AllowedManeuvers_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test AllowedManeuvers with all flags OFF.
 *
 * @par ASN.1 Definition:
 * @code
 * AllowedManeuvers ::= BIT STRING {
 *     maneuverStraightAllowed (0), maneuverLeftAllowed (1),
 *     maneuverRightAllowed (2), maneuverUTurnAllowed (3),
 *     maneuverLeftTurnOnRedAllowed (4), maneuverRightTurnOnRedAllowed (5),
 *     maneuverLaneChangeAllowed (6), maneuverNoStoppingAllowed (7),
 *     yieldAllwaysRequired (8), goWithHalt (9),
 *     caution (10), reserved1 (11)
 * } (SIZE (12))
 * @endcode
 *
 * @par Test Vector:
 * - All 12 flags: OFF (0)
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 000000000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:7]=00000000          |
 * | 1    | 0x00 | 00000000 | flags[8:11]=0000 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_all_zeros(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x00,                                          /* flags[8:11]=0000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x000U, J2735_ALLOWED_MANEUVERS_GET(payload),
                                  "All flags should be zero");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(payload),
                                  "maneuverStraightAllowed should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(payload),
                                  "reserved1 should be OFF");
}

/**
 * @brief Test AllowedManeuvers with all flags ON (0xFFF), bits 0-5.
 *
 * Split into two functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - All 12 flags: ON (1)
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 111111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111          |
 * | 1    | 0xF0 | 11110000 | flags[8:11]=1111 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_all_ones_bits_0_to_5(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0xF0,                                          /* flags[8:11]=1111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xFFFU, J2735_ALLOWED_MANEUVERS_GET(payload),
                                  "All flags should be ON (0xFFF)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(payload),
                                  "bit 0: maneuverStraightAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LEFT_ALLOWED(payload),
                                  "bit 1: maneuverLeftAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_RIGHT_ALLOWED(payload),
                                  "bit 2: maneuverRightAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_UT_URN_ALLOWED(payload),
                                  "bit 3: maneuverUTurnAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LEFT_TURN_ON_RED_ALLOWED(payload),
      "bit 4: maneuverLeftTurnOnRedAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_RIGHT_TURN_ON_RED_ALLOWED(payload),
      "bit 5: maneuverRightTurnOnRedAllowed should be ON");
}

/**
 * @brief Test AllowedManeuvers with all flags ON (0xFFF), bits 6-11.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111          |
 * | 1    | 0xF0 | 11110000 | flags[8:11]=1111 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_all_ones_bits_6_to_11(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0xF0,                                          /* flags[8:11]=1111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LANE_CHANGE_ALLOWED(payload),
                                  "bit 6: maneuverLaneChangeAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_ALLOWED_MANEUVERS_GET_MANEUVER_NO_STOPPING_ALLOWED(payload),
                                  "bit 7: maneuverNoStoppingAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_YIELD_ALLWAYS_REQUIRED(payload),
                                  "bit 8: yieldAllwaysRequired should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_GO_WITH_HALT(payload),
                                  "bit 9: goWithHalt should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_CAUTION(payload),
                                  "bit 10: caution should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(payload),
                                  "bit 11: reserved1 should be ON");
}

/**
 * @brief Test AllowedManeuvers with alternating pattern 101010101010 (0xAAA).
 *
 * @par Test Vector:
 * - Even bits ON, odd bits OFF
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 101010101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0xAA | 10101010 | flags[0:7]=10101010          |
 * | 1    | 0xA0 | 10100000 | flags[8:11]=1010 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_alternating_101010101010(void) {
  static const uint8_t payload[] = {
      0xAA,                                          /* flags[0:7]=10101010 */
      0xA0,                                          /* flags[8:11]=1010 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xAAAU, J2735_ALLOWED_MANEUVERS_GET(payload),
                                  "Alternating pattern should be 0xAAA");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(payload), "bit 0: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LEFT_ALLOWED(payload),
                                  "bit 1: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_YIELD_ALLWAYS_REQUIRED(payload),
                                  "bit 8: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_GO_WITH_HALT(payload),
                                  "bit 9: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_CAUTION(payload),
                                  "bit 10: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(payload),
                                  "bit 11: should be OFF");
}

/**
 * @brief Test AllowedManeuvers with alternating pattern 010101010101 (0x555).
 *
 * @par Test Vector:
 * - Odd bits ON, even bits OFF
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 010101010101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x55 | 01010101 | flags[0:7]=01010101          |
 * | 1    | 0x50 | 01010000 | flags[8:11]=0101 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_alternating_010101010101(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* flags[0:7]=01010101 */
      0x50,                                          /* flags[8:11]=0101 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x555U, J2735_ALLOWED_MANEUVERS_GET(payload),
                                  "Inverse alternating pattern should be 0x555");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(payload), "bit 0: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LEFT_ALLOWED(payload),
                                  "bit 1: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_YIELD_ALLWAYS_REQUIRED(payload),
                                  "bit 8: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_GO_WITH_HALT(payload),
                                  "bit 9: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_CAUTION(payload),
                                  "bit 10: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(payload),
                                  "bit 11: should be ON");
}

/**
 * @brief Test AllowedManeuvers with only maneuverStraightAllowed (bit 0) set.
 *
 * @par Test Vector:
 * - maneuverStraightAllowed(0): ON, all others: OFF
 * - flags = 0x800
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 100000000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x80 | 10000000 | flags[0:7]=10000000          |
 * | 1    | 0x00 | 00000000 | flags[8:11]=0000 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_single_bit_straight_allowed(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* flags[0:7]=10000000 */
      0x00,                                          /* flags[8:11]=0000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x800U, J2735_ALLOWED_MANEUVERS_GET(payload),
                                  "Only bit 0 should be set (0x800)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(payload),
                                  "maneuverStraightAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(payload),
                                  "reserved1 should be OFF");
}

/**
 * @brief Test AllowedManeuvers with only reserved1 (bit 11) set.
 *
 * @par Test Vector:
 * - reserved1(11): ON, all others: OFF
 * - flags = 0x001
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 000000000001 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:7]=00000000          |
 * | 1    | 0x10 | 00010000 | flags[8:11]=0001 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_single_bit_reserved_1(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x10,                                          /* flags[8:11]=0001 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x001U, J2735_ALLOWED_MANEUVERS_GET(payload),
                                  "Only bit 11 should be set (0x001)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(payload),
                                  "maneuverStraightAllowed should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(payload),
                                  "reserved1 should be ON");
}

/**
 * @brief Test AllowedManeuvers SIZE macro returns fixed 12 bits.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_size(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x00,                                          /* flags[8:11]=0000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(12U, J2735_ALLOWED_MANEUVERS_SIZE(payload),
                                   "SIZE should always be 12 for AllowedManeuvers");
}

/**
 * @brief Test AllowedManeuvers HAS_EXTENSION always returns false.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_has_extension(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0xF0,                                          /* flags[8:11]=1111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_ALLOWED_MANEUVERS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext,
                            "HAS_EXTENSION should always be false for non-extensible type");
}

/**
 * @brief Test AllowedManeuvers with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - All 12 flags: ON (0xFFF)
 *
 * @par Wire Format (12 bits total):
 * | Offset (bits) | Width | Field        | Value        |
 * |---------------|-------|--------------|--------------|
 * | 0             | 12    | flags[0:11]  | 111111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111          |
 * | 1    | 0xF0 | 11110000 | flags[8:11]=1111 + pad(4)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_allowed_maneuvers_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0xFF,                                          /* flags[0:7]=11111111 */
      0xF0,                                          /* flags[8:11]=1111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xFFFU, J2735_ALLOWED_MANEUVERS_GET(unaligned_ptr),
                                  "Misaligned: all flags should be ON (0xFFF)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(unaligned_ptr),
      "Misaligned: maneuverStraightAllowed should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(unaligned_ptr),
                                  "Misaligned: reserved1 should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all AllowedManeuvers tests.
 */
void run_testsuite_allowed_maneuvers(void) {
  RUN_TEST(test_allowed_maneuvers_all_zeros);
  RUN_TEST(test_allowed_maneuvers_all_ones_bits_0_to_5);
  RUN_TEST(test_allowed_maneuvers_all_ones_bits_6_to_11);
  RUN_TEST(test_allowed_maneuvers_alternating_101010101010);
  RUN_TEST(test_allowed_maneuvers_alternating_010101010101);
  RUN_TEST(test_allowed_maneuvers_single_bit_straight_allowed);
  RUN_TEST(test_allowed_maneuvers_single_bit_reserved_1);
  RUN_TEST(test_allowed_maneuvers_size);
  RUN_TEST(test_allowed_maneuvers_has_extension);
  RUN_TEST(test_allowed_maneuvers_misaligned_access);
}
