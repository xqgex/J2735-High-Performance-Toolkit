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
 * @brief Tests for TrafficLightOperationStatus extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * TrafficLightOperationStatus ::= BIT STRING {
 *     manual     (0),
 *     flashing   (1),
 *     off        (2),
 *     actuated   (3),
 *     transition (4),
 *     priority   (5),
 *     phase      (6),
 *     reserved   (7)
 * } (SIZE (8, ...))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Non-extended form (9 bits): [ext=0][8 flag bits]
 * - Extended form (16 bits): [ext=1][nsnnwn=7 bits][8 flag bits] — exact 2 bytes
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB of BIT STRING content (manual)
 * - ASN.1 bit 7 = rightmost root bit (reserved)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_TrafficLightOperationStatus.h"
#include "J2735_internal_DE_TrafficLightOperationStatus_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test TrafficLightOperationStatus with non-extended form, typical value.
 *
 * @par Test Vector:
 * - Flags: 0xAA = 10101010
 *
 * @par Wire Format (9 bits total):
 * | Offset (bits) | Width | Field    | Value     |
 * |---------------|-------|----------|-----------|
 * | 0             | 1     | ext_bit  | 0         |
 * | 1             | 8     | flags    | 10101010  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0)+flags[0..6](1010101)    |
 * | 1    | 0x00 | 00000000 | flags[7](0)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_non_extended(void) {
  static const uint8_t payload[] = {
      0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_TRAFFIC_LIGHT_OPERATION_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Extension bit should be 0 for non-extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xAAU, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "Flags should be 0xAA for non-extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(9U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_SIZE(payload),
                                   "Size should be 9 for non-extended form");
}

/**
 * @brief Test TrafficLightOperationStatus with extended form, all flags ON.
 *
 * @par Test Vector:
 * - nsnnwn value: 8 (small form: 0 + 001000 = 0b0001000)
 * - Flags: 0xFF (all 8 bits set)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x88 | 10001000 | ext(1)+nsnnwn(0001000)       |
 * | 1    | 0xFF | 11111111 | flags[0..7](11111111)        |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_extended(void) {
  static const uint8_t payload[] = {
      0x88, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_TRAFFIC_LIGHT_OPERATION_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Extension bit should be 1 for extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFFU, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "Flags should be 0xFF for extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(16U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_SIZE(payload),
                                   "Size should be 16 for extended form");
}

/**
 * @brief Test TrafficLightOperationStatus individual flag accessors (all ON).
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x7F | 01111111 | ext(0)+flags[0..6](1111111)    |
 * | 1    | 0x80 | 10000000 | flags[7](1)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_non_extended_flags(void) {
  static const uint8_t payload[] = {
      0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_MANUAL(payload),
                                  "bit 0: manual should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_FLASHING(payload),
                                  "bit 1: flashing should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_OFF(payload),
                                  "bit 2: off should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_ACTUATED(payload),
                                  "bit 3: actuated should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_TRANSITION(payload),
                                  "bit 4: transition should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_PRIORITY(payload),
                                  "bit 5: priority should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_PHASE(payload),
                                  "bit 6: phase should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_RESERVED(payload),
                                  "bit 7: reserved should be ON");
}

/**
 * @brief Test TrafficLightOperationStatus SIZE for non-extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_size_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(9U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_SIZE(payload),
                                   "Non-extended size should be 9 bits");
}

/**
 * @brief Test TrafficLightOperationStatus SIZE for extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_size_extended(void) {
  static const uint8_t payload[] = {
      0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(16U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_SIZE(payload),
                                   "Extended size should be 16 bits");
}

/**
 * @brief Test TrafficLightOperationStatus all zeros, non-extended.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_all_zeros_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_TRAFFIC_LIGHT_OPERATION_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "All flags should be zero");
}

/**
 * @brief Test TrafficLightOperationStatus all root flags ON, non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x7F | 01111111 | ext(0)+flags[0..6](1111111)    |
 * | 1    | 0x80 | 10000000 | flags[7](1)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_non_extended_all_flags_on(void) {
  static const uint8_t payload[] = {
      0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_TRAFFIC_LIGHT_OPERATION_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFFU, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "All 8 flags should be ON (0xFF)");
}

/**
 * @brief Test TrafficLightOperationStatus extended form, all flags zero.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_extended_all_zeros(void) {
  static const uint8_t payload[] = {
      0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_TRAFFIC_LIGHT_OPERATION_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Should be extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "All flags should be zero in extended form");
}

/**
 * @brief Test TrafficLightOperationStatus alternating 10101010 (0xAA), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0)+flags[0..6](1010101)    |
 * | 1    | 0x00 | 00000000 | flags[7](0)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_non_extended_alternating_10101010(void) {
  static const uint8_t payload[] = {
      0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xAAU, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "Alternating pattern should be 0xAA");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_MANUAL(payload),
                                  "bit 0: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_FLASHING(payload),
                                  "bit 1: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_OFF(payload),
                                  "bit 2: should be ON");
}

/**
 * @brief Test TrafficLightOperationStatus alternating 01010101 (0x55), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x2A | 00101010 | ext(0)+flags[0..6](0101010)    |
 * | 1    | 0x80 | 10000000 | flags[7](1)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_non_extended_alternating_01010101(void) {
  static const uint8_t payload[] = {
      0x2A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x55U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "Inverse alternating pattern should be 0x55");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_MANUAL(payload),
                                  "bit 0: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_FLASHING(payload),
                                  "bit 1: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_OFF(payload),
                                  "bit 2: should be OFF");
}

/**
 * @brief Test single bit 0 (manual), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x40 | 01000000 | ext(0)+flags[0..6](1000000)    |
 * | 1    | 0x00 | 00000000 | flags[7](0)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_single_bit_0_manual(void) {
  static const uint8_t payload[] = {
      0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x80U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "Only bit 0 should be set (0x80)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_MANUAL(payload),
                                  "manual should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_RESERVED(payload),
                                  "reserved should be OFF");
}

/**
 * @brief Test single bit 7 (reserved), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x00 | 00000000 | ext(0)+flags[0..6](0000000)    |
 * | 1    | 0x80 | 10000000 | flags[7](1)+pad(7)             |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_single_bit_7_reserved(void) {
  static const uint8_t payload[] = {
      0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(payload),
                                 "Only bit 7 should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_MANUAL(payload),
                                  "manual should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_RESERVED(payload),
                                  "reserved should be ON");
}

/**
 * @brief Test TrafficLightOperationStatus with misaligned buffer pointer.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_traffic_light_operation_status_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0x7F, 0x80,                                    /* ext(0)+all flags ON */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  bool has_ext = J2735_TRAFFIC_LIGHT_OPERATION_STATUS_HAS_EXTENSION(unaligned_ptr);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Misaligned: should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFFU, J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all TrafficLightOperationStatus tests.
 */
void run_testsuite_traffic_light_operation_status(void) {
  RUN_TEST(test_traffic_light_operation_status_non_extended);
  RUN_TEST(test_traffic_light_operation_status_extended);
  RUN_TEST(test_traffic_light_operation_status_non_extended_flags);
  RUN_TEST(test_traffic_light_operation_status_size_non_extended);
  RUN_TEST(test_traffic_light_operation_status_size_extended);
  RUN_TEST(test_traffic_light_operation_status_all_zeros_non_extended);
  RUN_TEST(test_traffic_light_operation_status_non_extended_all_flags_on);
  RUN_TEST(test_traffic_light_operation_status_extended_all_zeros);
  RUN_TEST(test_traffic_light_operation_status_non_extended_alternating_10101010);
  RUN_TEST(test_traffic_light_operation_status_non_extended_alternating_01010101);
  RUN_TEST(test_traffic_light_operation_status_single_bit_0_manual);
  RUN_TEST(test_traffic_light_operation_status_single_bit_7_reserved);
  RUN_TEST(test_traffic_light_operation_status_misaligned_access);
}
