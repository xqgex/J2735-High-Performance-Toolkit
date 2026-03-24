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
 * @brief Tests for GNSSstatus non-extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * GNSSstatus ::= BIT STRING {
 *     unavailable                (0),
 *     isHealthy                  (1),
 *     isMonitored                (2),
 *     baseStationType            (3),
 *     aPDOPofUnder5              (4),
 *     inViewOfUnder5             (5),
 *     localCorrectionsPresent    (6),
 *     networkCorrectionsPresent  (7)
 * } (SIZE (8))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Fixed form (8 bits): [F0 F1 F2 F3 F4 F5 F6 F7]
 * - No extension marker (non-extensible type)
 * - Byte-aligned: exactly 1 byte on wire
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB of BIT STRING content (unavailable)
 * - ASN.1 bit 7 = rightmost (networkCorrectionsPresent)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_GNSSstatus.h"
#include "J2735_internal_DE_GNSSstatus_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test GNSSstatus with all flags OFF.
 *
 * @par ASN.1 Definition:
 * @code
 * GNSSstatus ::= BIT STRING {
 *     unavailable (0), isHealthy (1), isMonitored (2),
 *     baseStationType (3), aPDOPofUnder5 (4), inViewOfUnder5 (5),
 *     localCorrectionsPresent (6), networkCorrectionsPresent (7)
 * } (SIZE (8))
 * @endcode
 *
 * @par Test Vector:
 * - All 8 flags: OFF (0)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 00000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0x00 | 00000000 | flags[0:7]=00000000 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_all_zeros(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_GNSS_STATUS_GET(payload), "All flags should be zero");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_UNAVAILABLE(payload),
                                  "unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_IS_HEALTHY(payload),
                                  "isHealthy should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_NETWORK_CORRECTIONS_PRESENT(payload),
                                  "networkCorrectionsPresent should be OFF");
}

/**
 * @brief Test GNSSstatus with all flags ON (0xFF), bits 0-4.
 *
 * Split into two functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - All 8 flags: ON (1)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 11111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_all_ones_bits_0_to_4(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFFU, J2735_GNSS_STATUS_GET(payload),
                                 "All flags should be ON (0xFF)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_IS_HEALTHY(payload),
                                  "bit 1: isHealthy should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_IS_MONITORED(payload),
                                  "bit 2: isMonitored should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_BASE_STATION_TYPE(payload),
                                  "bit 3: baseStationType should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_A_PDOP_OF_UNDER_5(payload),
                                  "bit 4: aPDOPofUnder5 should be ON");
}

/**
 * @brief Test GNSSstatus with all flags ON (0xFF), bits 5-7.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_all_ones_bits_5_to_7(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_IN_VIEW_OF_UNDER_5(payload),
                                  "bit 5: inViewOfUnder5 should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_LOCAL_CORRECTIONS_PRESENT(payload),
                                  "bit 6: localCorrectionsPresent should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_NETWORK_CORRECTIONS_PRESENT(payload),
                                  "bit 7: networkCorrectionsPresent should be ON");
}

/**
 * @brief Test GNSSstatus with alternating pattern 10101010 (0xAA).
 *
 * @par Test Vector:
 * - unavailable(0): ON, isHealthy(1): OFF, isMonitored(2): ON,
 *   baseStationType(3): OFF, aPDOPofUnder5(4): ON, inViewOfUnder5(5): OFF,
 *   localCorrectionsPresent(6): ON, networkCorrectionsPresent(7): OFF
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 10101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0xAA | 10101010 | flags[0:7]=10101010 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_alternating_10101010(void) {
  static const uint8_t payload[] = {
      0xAA,                                          /* flags[0:7]=10101010 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xAAU, J2735_GNSS_STATUS_GET(payload),
                                 "Alternating pattern should be 0xAA");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_IS_HEALTHY(payload),
                                  "bit 1: isHealthy should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_IS_MONITORED(payload),
                                  "bit 2: isMonitored should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_BASE_STATION_TYPE(payload),
                                  "bit 3: baseStationType should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_A_PDOP_OF_UNDER_5(payload),
                                  "bit 4: aPDOPofUnder5 should be ON");
}

/**
 * @brief Test GNSSstatus with alternating pattern 01010101 (0x55).
 *
 * @par Test Vector:
 * - unavailable(0): OFF, isHealthy(1): ON, isMonitored(2): OFF,
 *   baseStationType(3): ON, aPDOPofUnder5(4): OFF, inViewOfUnder5(5): ON,
 *   localCorrectionsPresent(6): OFF, networkCorrectionsPresent(7): ON
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 01010101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0x55 | 01010101 | flags[0:7]=01010101 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_alternating_01010101(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* flags[0:7]=01010101 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x55U, J2735_GNSS_STATUS_GET(payload),
                                 "Inverse alternating pattern should be 0x55");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_IS_HEALTHY(payload),
                                  "bit 1: isHealthy should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_IS_MONITORED(payload),
                                  "bit 2: isMonitored should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_BASE_STATION_TYPE(payload),
                                  "bit 3: baseStationType should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_A_PDOP_OF_UNDER_5(payload),
                                  "bit 4: aPDOPofUnder5 should be OFF");
}

/**
 * @brief Test GNSSstatus with only unavailable (bit 0) set.
 *
 * @par Test Vector:
 * - unavailable(0): ON, all others: OFF
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 10000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0x80 | 10000000 | flags[0:7]=10000000 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_single_bit_unavailable(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* flags[0:7]=10000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x80U, J2735_GNSS_STATUS_GET(payload),
                                 "Only unavailable should be set (0x80)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_UNAVAILABLE(payload),
                                  "unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_NETWORK_CORRECTIONS_PRESENT(payload),
                                  "networkCorrectionsPresent should be OFF");
}

/**
 * @brief Test GNSSstatus with only networkCorrectionsPresent (bit 7) set.
 *
 * @par Test Vector:
 * - networkCorrectionsPresent(7): ON, all others: OFF
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 00000001 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0x01 | 00000001 | flags[0:7]=00000001 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_single_bit_network_corrections_present(void) {
  static const uint8_t payload[] = {
      0x01,                                          /* flags[0:7]=00000001 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_GNSS_STATUS_GET(payload),
                                 "Only networkCorrectionsPresent should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_GNSS_STATUS_GET_UNAVAILABLE(payload),
                                  "unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_NETWORK_CORRECTIONS_PRESENT(payload),
                                  "networkCorrectionsPresent should be ON");
}

/**
 * @brief Test GNSSstatus SIZE macro returns fixed 8 bits.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_size(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(8U, J2735_GNSS_STATUS_SIZE(payload),
                                   "SIZE should always be 8 for GNSSstatus");
}

/**
 * @brief Test GNSSstatus HAS_EXTENSION always returns false.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_has_extension(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_GNSS_STATUS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext,
                            "HAS_EXTENSION should always be false for non-extensible type");
}

/**
 * @brief Test GNSSstatus with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - All 8 flags: ON (0xFF)
 *
 * @par Wire Format (8 bits total):
 * | Offset (bits) | Width | Field       | Value    |
 * |---------------|-------|-------------|----------|
 * | 0             | 8     | flags[0:7]  | 11111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields        |
 * |------|------|----------|---------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111 |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_gnss_status_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0xFF,                                          /* flags[0:7]=11111111 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFFU, J2735_GNSS_STATUS_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON (0xFF)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_GNSS_STATUS_GET_UNAVAILABLE(unaligned_ptr),
                                  "Misaligned: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_GNSS_STATUS_GET_NETWORK_CORRECTIONS_PRESENT(unaligned_ptr),
                                  "Misaligned: networkCorrectionsPresent should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all GNSSstatus tests.
 */
void run_testsuite_gnss_status(void) {
  RUN_TEST(test_gnss_status_all_zeros);
  RUN_TEST(test_gnss_status_all_ones_bits_0_to_4);
  RUN_TEST(test_gnss_status_all_ones_bits_5_to_7);
  RUN_TEST(test_gnss_status_alternating_10101010);
  RUN_TEST(test_gnss_status_alternating_01010101);
  RUN_TEST(test_gnss_status_single_bit_unavailable);
  RUN_TEST(test_gnss_status_single_bit_network_corrections_present);
  RUN_TEST(test_gnss_status_size);
  RUN_TEST(test_gnss_status_has_extension);
  RUN_TEST(test_gnss_status_misaligned_access);
}
