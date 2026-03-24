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
 * @brief Tests for TransitStatus non-extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * TransitStatus ::= BIT STRING {
 *     none       (0),
 *     anAdaUse   (1),
 *     aBikeLoad  (2),
 *     doorOpen   (3),
 *     occM       (4),
 *     occL       (5)
 * } (SIZE (6))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Fixed form (6 bits): [F0 F1 F2 F3 F4 F5]
 * - No extension marker (non-extensible type)
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB of BIT STRING content (none)
 * - ASN.1 bit 5 = rightmost (occL)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_TransitStatus.h"
#include "J2735_internal_DE_TransitStatus_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test TransitStatus with all flags OFF.
 *
 * @par ASN.1 Definition:
 * @code
 * TransitStatus ::= BIT STRING {
 *     none (0), anAdaUse (1), aBikeLoad (2),
 *     doorOpen (3), occM (4), occL (5)
 * } (SIZE (6))
 * @endcode
 *
 * @par Test Vector:
 * - All 6 flags: OFF (0)
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:5]=000000 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_all_zeros(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:5]=000000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_TRANSIT_STATUS_GET(payload),
                                 "All flags should be zero");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_NONE(payload), "none should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_AN_ADA_USE(payload),
                                  "anAdaUse should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_A_BIKE_LOAD(payload),
                                  "aBikeLoad should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_DOOR_OPEN(payload),
                                  "doorOpen should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_OCC_M(payload),
                                  "occM should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_OCC_L(payload),
                                  "occL should be OFF");
}

/**
 * @brief Test TransitStatus with all flags ON (0x3F).
 *
 * @par ASN.1 Definition:
 * @code
 * TransitStatus ::= BIT STRING {
 *     none (0), anAdaUse (1), aBikeLoad (2),
 *     doorOpen (3), occM (4), occL (5)
 * } (SIZE (6))
 * @endcode
 *
 * @par Test Vector:
 * - All 6 flags: ON (1)
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0xFC | 11111100 | flags[0:5]=111111 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_all_ones(void) {
  static const uint8_t payload[] = {
      0xFC,                                          /* flags[0:5]=111111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3FU, J2735_TRANSIT_STATUS_GET(payload),
                                 "All flags should be ON (0x3F)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_NONE(payload), "none should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_AN_ADA_USE(payload),
                                  "anAdaUse should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_A_BIKE_LOAD(payload),
                                  "aBikeLoad should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_DOOR_OPEN(payload),
                                  "doorOpen should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_OCC_M(payload), "occM should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_OCC_L(payload), "occL should be ON");
}

/**
 * @brief Test TransitStatus with alternating pattern 101010 (0x2A).
 *
 * @par Test Vector:
 * - none(0): ON, anAdaUse(1): OFF, aBikeLoad(2): ON,
 *   doorOpen(3): OFF, occM(4): ON, occL(5): OFF
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0xA8 | 10101000 | flags[0:5]=101010 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_alternating_101010(void) {
  static const uint8_t payload[] = {
      0xA8,                                          /* flags[0:5]=101010 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x2AU, J2735_TRANSIT_STATUS_GET(payload),
                                 "Alternating pattern should be 0x2A");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_NONE(payload),
                                  "bit 0: none should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_AN_ADA_USE(payload),
                                  "bit 1: anAdaUse should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_A_BIKE_LOAD(payload),
                                  "bit 2: aBikeLoad should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_DOOR_OPEN(payload),
                                  "bit 3: doorOpen should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_OCC_M(payload),
                                  "bit 4: occM should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_OCC_L(payload),
                                  "bit 5: occL should be OFF");
}

/**
 * @brief Test TransitStatus with alternating pattern 010101 (0x15).
 *
 * @par Test Vector:
 * - none(0): OFF, anAdaUse(1): ON, aBikeLoad(2): OFF,
 *   doorOpen(3): ON, occM(4): OFF, occL(5): ON
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 010101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x54 | 01010100 | flags[0:5]=010101 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_alternating_010101(void) {
  static const uint8_t payload[] = {
      0x54,                                          /* flags[0:5]=010101 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x15U, J2735_TRANSIT_STATUS_GET(payload),
                                 "Inverse alternating pattern should be 0x15");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_NONE(payload),
                                  "bit 0: none should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_AN_ADA_USE(payload),
                                  "bit 1: anAdaUse should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_A_BIKE_LOAD(payload),
                                  "bit 2: aBikeLoad should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_DOOR_OPEN(payload),
                                  "bit 3: doorOpen should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_OCC_M(payload),
                                  "bit 4: occM should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_OCC_L(payload),
                                  "bit 5: occL should be ON");
}

/**
 * @brief Test TransitStatus with only none (bit 0) set.
 *
 * @par Test Vector:
 * - none(0): ON, all others: OFF
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 100000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x80 | 10000000 | flags[0:5]=100000 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_single_bit_none(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* flags[0:5]=100000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x20U, J2735_TRANSIT_STATUS_GET(payload),
                                 "Only none should be set (0x20)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_NONE(payload), "none should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_OCC_L(payload),
                                  "occL should be OFF");
}

/**
 * @brief Test TransitStatus with only occL (bit 5) set.
 *
 * @par Test Vector:
 * - occL(5): ON, all others: OFF
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 000001 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x04 | 00000100 | flags[0:5]=000001 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_single_bit_occ_l(void) {
  static const uint8_t payload[] = {
      0x04,                                          /* flags[0:5]=000001 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_TRANSIT_STATUS_GET(payload),
                                 "Only occL should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRANSIT_STATUS_GET_NONE(payload), "none should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_OCC_L(payload), "occL should be ON");
}

/**
 * @brief Test TransitStatus SIZE macro returns fixed 6 bits.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_size(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:5]=000000 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(6U, J2735_TRANSIT_STATUS_SIZE(payload),
                                   "SIZE should always be 6 for TransitStatus");
}

/**
 * @brief Test TransitStatus HAS_EXTENSION always returns false.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_has_extension(void) {
  static const uint8_t payload[] = {
      0xFC,                                          /* flags[0:5]=111111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_TRANSIT_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext,
                            "HAS_EXTENSION should always be false for non-extensible type");
}

/**
 * @brief Test TransitStatus with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - All 6 flags: ON (0x3F)
 *
 * @par Wire Format (6 bits total):
 * | Offset (bits) | Width | Field       | Value  |
 * |---------------|-------|-------------|--------|
 * | 0             | 6     | flags[0:5]  | 111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0xFC | 11111100 | flags[0:5]=111111 + pad(2) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_transit_status_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0xFC,                                          /* flags[0:5]=111111 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3FU, J2735_TRANSIT_STATUS_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON (0x3F)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_NONE(unaligned_ptr),
                                  "Misaligned: none should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRANSIT_STATUS_GET_OCC_L(unaligned_ptr),
                                  "Misaligned: occL should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all TransitStatus tests.
 */
void run_testsuite_transit_status(void) {
  RUN_TEST(test_transit_status_all_zeros);
  RUN_TEST(test_transit_status_all_ones);
  RUN_TEST(test_transit_status_alternating_101010);
  RUN_TEST(test_transit_status_alternating_010101);
  RUN_TEST(test_transit_status_single_bit_none);
  RUN_TEST(test_transit_status_single_bit_occ_l);
  RUN_TEST(test_transit_status_size);
  RUN_TEST(test_transit_status_has_extension);
  RUN_TEST(test_transit_status_misaligned_access);
}
