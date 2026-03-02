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
 * @brief Tests for PublicSafetyDirectingTrafficSubType extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * PublicSafetyDirectingTrafficSubType ::= BIT STRING {
 *     unavailable                                (0),
 *     policeAndTrafficOfficers                   (1),
 *     trafficControlPersons                      (2),
 *     railroadCrossingGuards                     (3),
 *     civilDefenseNationalGuardMilitaryPolice    (4),
 *     emergencyOrganizationPersonnel             (5),
 *     highwayServiceVehiclePersonnel             (6)
 * } (SIZE (7, ...))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Non-extended form (8 bits): [ext=0][7 flag bits] — exact byte, no padding
 * - Extended form (15 bits): [ext=1][nsnnwn=7 bits][7 flag bits]
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_PublicSafetyDirectingTrafficSubType.h"
#include "J2735_internal_DE_PublicSafetyDirectingTrafficSubType_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test PublicSafetyDirectingTrafficSubType with non-extended form, typical value.
 *
 * @par Test Vector:
 * - Flags: 0x55 = 1010101
 *
 * @par Wire Format (8 bits total, no padding):
 * | Offset (bits) | Width | Field    | Value    |
 * |---------------|-------|----------|----------|
 * | 0             | 1     | ext_bit  | 0        |
 * | 1             | 7     | flags    | 1010101  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x55 | 01010101 | ext(0)+flags(1010101) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_non_extended(void) {
  static const uint8_t payload[] = {
      0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool is_ext = J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(payload);
  TEST_ASSERT_FALSE_MESSAGE(is_ext, "Extension bit should be 0 for non-extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x55U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "Flags should be 0x55 for non-extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(8U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_SIZE(payload),
                                   "Size should be 8 for non-extended form");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType with extended form, all flags ON.
 *
 * @par Test Vector:
 * - nsnnwn value: 7 (small form: 0 + 000111 = 0b0000111)
 * - Flags: 0x7F (all 7 bits set)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x87 | 10000111 | ext(1)+nsnnwn(0000111)       |
 * | 1    | 0xFE | 11111110 | flags(1111111)+pad(1)        |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_extended(void) {
  static const uint8_t payload[] = {
      0x87, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool is_ext = J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(payload);
  TEST_ASSERT_TRUE_MESSAGE(is_ext, "Extension bit should be 1 for extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x7FU, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "Flags should be 0x7F for extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(15U,
                                   J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_SIZE(payload),
                                   "Size should be 15 for extended form");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType individual flag accessors (all ON).
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x7F | 01111111 | ext(0)+flags(1111111) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_non_extended_flags(void) {
  static const uint8_t payload[] = {
      0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_UNAVAILABLE(payload),
      "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_POLICE_AND_TRAFFIC_OFFICERS(payload),
      "bit 1: policeAndTrafficOfficers should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_TRAFFIC_CONTROL_PERSONS(payload),
      "bit 2: trafficControlPersons should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_RAILROAD_CROSSING_GUARDS(payload),
      "bit 3: railroadCrossingGuards should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U,
      J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_CIVIL_DEFENSE_NATIONAL_GUARD_MILITARY_POLICE(
          payload),
      "bit 4: civilDefenseNationalGuardMilitaryPolice should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U,
      J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_EMERGENCY_ORGANIZATION_PERSONNEL(payload),
      "bit 5: emergencyOrganizationPersonnel should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U,
      J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_HIGHWAY_SERVICE_VEHICLE_PERSONNEL(payload),
      "bit 6: highwayServiceVehiclePersonnel should be ON");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType SIZE for non-extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_size_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(8U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_SIZE(payload),
                                   "Non-extended size should be 8 bits");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType SIZE for extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_size_extended(void) {
  static const uint8_t payload[] = {
      0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(15U,
                                   J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_SIZE(payload),
                                   "Extended size should be 15 bits");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType all zeros, non-extended.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_all_zeros_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool is_ext = J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(payload);
  TEST_ASSERT_FALSE_MESSAGE(is_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "All flags should be zero");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType all root flags ON, non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x7F | 01111111 | ext(0)+flags(1111111) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_non_extended_all_flags_on(void) {
  static const uint8_t payload[] = {
      0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool is_ext = J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(payload);
  TEST_ASSERT_FALSE_MESSAGE(is_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x7FU, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "All 7 flags should be ON (0x7F)");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType extended form, all flags zero.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_extended_all_zeros(void) {
  static const uint8_t payload[] = {
      0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool is_ext = J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(payload);
  TEST_ASSERT_TRUE_MESSAGE(is_ext, "Should be extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "All flags should be zero in extended form");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType alternating 1010101 (0x55), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x55 | 01010101 | ext(0)+flags(1010101) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_non_extended_alternating_1010101(void) {
  static const uint8_t payload[] = {
      0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x55U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "Alternating pattern should be 0x55");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_UNAVAILABLE(payload),
      "bit 0: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_POLICE_AND_TRAFFIC_OFFICERS(payload),
      "bit 1: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_TRAFFIC_CONTROL_PERSONS(payload),
      "bit 2: should be ON");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType alternating 0101010 (0x2A), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x2A | 00101010 | ext(0)+flags(0101010) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_non_extended_alternating_0101010(void) {
  static const uint8_t payload[] = {
      0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x2AU, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "Inverse alternating pattern should be 0x2A");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_UNAVAILABLE(payload),
      "bit 0: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_POLICE_AND_TRAFFIC_OFFICERS(payload),
      "bit 1: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_TRAFFIC_CONTROL_PERSONS(payload),
      "bit 2: should be OFF");
}

/**
 * @brief Test single bit 0 (unavailable), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x40 | 01000000 | ext(0)+flags(1000000) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_single_bit_0_unavailable(void) {
  static const uint8_t payload[] = {
      0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x40U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "Only bit 0 should be set (0x40)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_UNAVAILABLE(payload),
      "unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U,
      J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_HIGHWAY_SERVICE_VEHICLE_PERSONNEL(payload),
      "highwayServiceVehiclePersonnel should be OFF");
}

/**
 * @brief Test single bit 6 (highwayServiceVehiclePersonnel), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x01 | 00000001 | ext(0)+flags(0000001) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_single_bit_6_highway_service(void) {
  static const uint8_t payload[] = {
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(payload),
                                 "Only bit 6 should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_UNAVAILABLE(payload),
      "unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U,
      J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_HIGHWAY_SERVICE_VEHICLE_PERSONNEL(payload),
      "highwayServiceVehiclePersonnel should be ON");
}

/**
 * @brief Test PublicSafetyDirectingTrafficSubType with misaligned buffer pointer.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_directing_traffic_sub_type_misaligned_access(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* junk byte for misalignment */
      0x7F,                                          /* ext(0)+flags(1111111) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  bool is_ext = J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(unaligned_ptr);
  TEST_ASSERT_FALSE_MESSAGE(is_ext, "Misaligned: should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x7FU,
                                 J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all PublicSafetyDirectingTrafficSubType tests.
 */
void run_testsuite_public_safety_directing_traffic_sub_type(void) {
  RUN_TEST(test_public_safety_directing_traffic_sub_type_non_extended);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_extended);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_non_extended_flags);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_size_non_extended);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_size_extended);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_all_zeros_non_extended);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_non_extended_all_flags_on);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_extended_all_zeros);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_non_extended_alternating_1010101);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_non_extended_alternating_0101010);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_single_bit_0_unavailable);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_single_bit_6_highway_service);
  RUN_TEST(test_public_safety_directing_traffic_sub_type_misaligned_access);
}
