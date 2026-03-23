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
 * @brief Tests for LaneDirection non-extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * LaneDirection ::= BIT STRING {
 *     ingressPath (0),
 *     egressPath (1)
 * } (SIZE (2))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Fixed form (2 bits): [F0 F1]
 * - No extension marker (non-extensible type)
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB of BIT STRING content (ingressPath)
 * - ASN.1 bit 1 = rightmost (egressPath)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_LaneDirection.h"
#include "J2735_internal_DE_LaneDirection_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test LaneDirection with all flags OFF.
 *
 * @par ASN.1 Definition:
 * @code
 * LaneDirection ::= BIT STRING {
 *     ingressPath (0),
 *     egressPath (1)
 * } (SIZE (2))
 * @endcode
 *
 * @par Test Vector:
 * - ingressPath: OFF (0)
 * - egressPath: OFF (0)
 *
 * @par Wire Format (2 bits total):
 * | Offset (bits) | Width | Field        | Value |
 * |---------------|-------|--------------|-------|
 * | 0             | 1     | ingressPath  | 0     |
 * | 1             | 1     | egressPath   | 0     |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                   |
 * |------|------|----------|--------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:1]=00 + pad(6)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_all_zeros(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:1]=00 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_LANE_DIRECTION_GET(payload),
                                 "All flags should be zero");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_DIRECTION_GET_INGRESS_PATH(payload),
                                  "ingressPath should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_DIRECTION_GET_EGRESS_PATH(payload),
                                  "egressPath should be OFF");
}

/**
 * @brief Test LaneDirection with all flags ON (0x3).
 *
 * @par ASN.1 Definition:
 * @code
 * LaneDirection ::= BIT STRING {
 *     ingressPath (0),
 *     egressPath (1)
 * } (SIZE (2))
 * @endcode
 *
 * @par Test Vector:
 * - ingressPath: ON (1)
 * - egressPath: ON (1)
 *
 * @par Wire Format (2 bits total):
 * | Offset (bits) | Width | Field        | Value |
 * |---------------|-------|--------------|-------|
 * | 0             | 1     | ingressPath  | 1     |
 * | 1             | 1     | egressPath   | 1     |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                   |
 * |------|------|----------|--------------------------|
 * | 0    | 0xC0 | 11000000 | flags[0:1]=11 + pad(6)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_all_ones(void) {
  static const uint8_t payload[] = {
      0xC0,                                          /* flags[0:1]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x03U, J2735_LANE_DIRECTION_GET(payload),
                                 "All flags should be ON (0x03)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_DIRECTION_GET_INGRESS_PATH(payload),
                                  "ingressPath should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_DIRECTION_GET_EGRESS_PATH(payload),
                                  "egressPath should be ON");
}

/**
 * @brief Test LaneDirection with only ingressPath (bit 0) set.
 *
 * @par ASN.1 Definition:
 * @code
 * LaneDirection ::= BIT STRING {
 *     ingressPath (0),
 *     egressPath (1)
 * } (SIZE (2))
 * @endcode
 *
 * @par Test Vector:
 * - ingressPath: ON (1)
 * - egressPath: OFF (0)
 *
 * @par Wire Format (2 bits total):
 * | Offset (bits) | Width | Field        | Value |
 * |---------------|-------|--------------|-------|
 * | 0             | 1     | ingressPath  | 1     |
 * | 1             | 1     | egressPath   | 0     |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                   |
 * |------|------|----------|--------------------------|
 * | 0    | 0x80 | 10000000 | flags[0:1]=10 + pad(6)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_single_bit_ingress_path(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* flags[0:1]=10 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x02U, J2735_LANE_DIRECTION_GET(payload),
                                 "Only ingressPath should be set (0x02)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_DIRECTION_GET_INGRESS_PATH(payload),
                                  "ingressPath should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_DIRECTION_GET_EGRESS_PATH(payload),
                                  "egressPath should be OFF");
}

/**
 * @brief Test LaneDirection with only egressPath (bit 1) set.
 *
 * @par ASN.1 Definition:
 * @code
 * LaneDirection ::= BIT STRING {
 *     ingressPath (0),
 *     egressPath (1)
 * } (SIZE (2))
 * @endcode
 *
 * @par Test Vector:
 * - ingressPath: OFF (0)
 * - egressPath: ON (1)
 *
 * @par Wire Format (2 bits total):
 * | Offset (bits) | Width | Field        | Value |
 * |---------------|-------|--------------|-------|
 * | 0             | 1     | ingressPath  | 0     |
 * | 1             | 1     | egressPath   | 1     |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                   |
 * |------|------|----------|--------------------------|
 * | 0    | 0x40 | 01000000 | flags[0:1]=01 + pad(6)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_single_bit_egress_path(void) {
  static const uint8_t payload[] = {
      0x40,                                          /* flags[0:1]=01 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_LANE_DIRECTION_GET(payload),
                                 "Only egressPath should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_DIRECTION_GET_INGRESS_PATH(payload),
                                  "ingressPath should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_DIRECTION_GET_EGRESS_PATH(payload),
                                  "egressPath should be ON");
}

/**
 * @brief Test LaneDirection SIZE macro returns fixed 2 bits.
 *
 * LaneDirection is a non-extensible BIT STRING, always 2 bits on wire.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_size(void) {
  static const uint8_t payload[] = {
      0xC0,                                          /* flags[0:1]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, J2735_LANE_DIRECTION_SIZE(payload),
                                   "SIZE should always be 2 for LaneDirection");
}

/**
 * @brief Test LaneDirection HAS_EXTENSION always returns false.
 *
 * LaneDirection is non-extensible, so HAS_EXTENSION must always return 0.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_has_extension(void) {
  static const uint8_t payload[] = {
      0xC0,                                          /* flags[0:1]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_LANE_DIRECTION_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext,
                            "HAS_EXTENSION should always be false for non-extensible type");
}

/**
 * @brief Test LaneDirection with deliberately misaligned buffer pointer.
 *
 * Forces unaligned memory access to verify safety on strict-alignment hardware.
 *
 * @par Test Vector:
 * - ingressPath: ON (1)
 * - egressPath: ON (1)
 *
 * @par Wire Format (2 bits total):
 * | Offset (bits) | Width | Field        | Value |
 * |---------------|-------|--------------|-------|
 * | 0             | 1     | ingressPath  | 1     |
 * | 1             | 1     | egressPath   | 1     |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                   |
 * |------|------|----------|--------------------------|
 * | 0    | 0xC0 | 11000000 | flags[0:1]=11 + pad(6)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_direction_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* junk byte for misalignment */
      0xC0,                                          /* flags[0:1]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x03U, J2735_LANE_DIRECTION_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON (0x03)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_DIRECTION_GET_INGRESS_PATH(unaligned_ptr),
                                  "Misaligned: ingressPath should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_DIRECTION_GET_EGRESS_PATH(unaligned_ptr),
                                  "Misaligned: egressPath should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all LaneDirection tests.
 */
void run_testsuite_lane_direction(void) {
  RUN_TEST(test_lane_direction_all_zeros);
  RUN_TEST(test_lane_direction_all_ones);
  RUN_TEST(test_lane_direction_single_bit_ingress_path);
  RUN_TEST(test_lane_direction_single_bit_egress_path);
  RUN_TEST(test_lane_direction_size);
  RUN_TEST(test_lane_direction_has_extension);
  RUN_TEST(test_lane_direction_misaligned_access);
}
