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
 * @brief Tests for LaneSharing non-extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * LaneSharing ::= BIT STRING {
 *     overlappingLaneDescriptionProvided (0),
 *     multipleLanesTreatedAsOneLane      (1),
 *     otherNonMotorizedTrafficTypes      (2),
 *     individualMotorizedVehicleTraffic  (3),
 *     busVehicleTraffic                  (4),
 *     taxiVehicleTraffic                 (5),
 *     pedestriansTraffic                 (6),
 *     cyclistVehicleTraffic              (7),
 *     trackedVehicleTraffic              (8),
 *     reserved                           (9)
 * } (SIZE (10))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Fixed form (10 bits): [F0 F1 F2 F3 F4 F5 F6 F7 F8 F9]
 * - No extension marker (non-extensible type)
 * - Spans 2 bytes (8 bits in byte 0, 2 bits in byte 1)
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB (overlappingLaneDescriptionProvided)
 * - ASN.1 bit 9 = rightmost (reserved)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_LaneSharing.h"
#include "J2735_internal_DE_LaneSharing_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test LaneSharing with all flags OFF.
 *
 * @par ASN.1 Definition:
 * @code
 * LaneSharing ::= BIT STRING {
 *     overlappingLaneDescriptionProvided (0),
 *     multipleLanesTreatedAsOneLane (1),
 *     otherNonMotorizedTrafficTypes (2),
 *     individualMotorizedVehicleTraffic (3),
 *     busVehicleTraffic (4), taxiVehicleTraffic (5),
 *     pedestriansTraffic (6), cyclistVehicleTraffic (7),
 *     trackedVehicleTraffic (8), reserved (9)
 * } (SIZE (10))
 * @endcode
 *
 * @par Test Vector:
 * - All 10 flags: OFF (0)
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 0000000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:7]=00000000         |
 * | 1    | 0x00 | 00000000 | flags[8:9]=00 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_all_zeros(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x00,                                          /* flags[8:9]=00 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x000U, J2735_LANE_SHARING_GET(payload),
                                  "All flags should be zero");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(payload),
      "overlappingLaneDescriptionProvided should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_SHARING_GET_RESERVED(payload),
                                  "reserved should be OFF");
}

/**
 * @brief Test LaneSharing with all flags ON (0x3FF), bits 0-4.
 *
 * Split into two functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - All 10 flags: ON (1)
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 1111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111         |
 * | 1    | 0xC0 | 11000000 | flags[8:9]=11 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_all_ones_bits_0_to_4(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0xC0,                                          /* flags[8:9]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x3FFU, J2735_LANE_SHARING_GET(payload),
                                  "All flags should be ON (0x3FF)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(payload),
      "bit 0: overlappingLaneDescriptionProvided should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_MULTIPLE_LANES_TREATED_AS_ONE_LANE(payload),
      "bit 1: multipleLanesTreatedAsOneLane should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_LANE_SHARING_GET_OTHER_NON_MOTORIZED_TRAFFIC_TYPES(payload),
                                  "bit 2: otherNonMotorizedTrafficTypes should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_INDIVIDUAL_MOTORIZED_VEHICLE_TRAFFIC(payload),
      "bit 3: individualMotorizedVehicleTraffic should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_BUS_VEHICLE_TRAFFIC(payload),
                                  "bit 4: busVehicleTraffic should be ON");
}

/**
 * @brief Test LaneSharing with all flags ON (0x3FF), bits 5-9.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111         |
 * | 1    | 0xC0 | 11000000 | flags[8:9]=11 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_all_ones_bits_5_to_9(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0xC0,                                          /* flags[8:9]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_TAXI_VEHICLE_TRAFFIC(payload),
                                  "bit 5: taxiVehicleTraffic should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_PEDESTRIANS_TRAFFIC(payload),
                                  "bit 6: pedestriansTraffic should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_CYCLIST_VEHICLE_TRAFFIC(payload),
                                  "bit 7: cyclistVehicleTraffic should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_TRACKED_VEHICLE_TRAFFIC(payload),
                                  "bit 8: trackedVehicleTraffic should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_RESERVED(payload),
                                  "bit 9: reserved should be ON");
}

/**
 * @brief Test LaneSharing with alternating pattern 1010101010 (0x2AA).
 *
 * @par Test Vector:
 * - Even bits ON, odd bits OFF
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 1010101010 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0xAA | 10101010 | flags[0:7]=10101010         |
 * | 1    | 0x80 | 10000000 | flags[8:9]=10 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_alternating_1010101010(void) {
  static const uint8_t payload[] = {
      0xAA,                                          /* flags[0:7]=10101010 */
      0x80,                                          /* flags[8:9]=10 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x2AAU, J2735_LANE_SHARING_GET(payload),
                                  "Alternating pattern should be 0x2AA");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(payload),
      "bit 0: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_LANE_SHARING_GET_MULTIPLE_LANES_TREATED_AS_ONE_LANE(payload),
      "bit 1: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_TRACKED_VEHICLE_TRAFFIC(payload),
                                  "bit 8: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_SHARING_GET_RESERVED(payload),
                                  "bit 9: should be OFF");
}

/**
 * @brief Test LaneSharing with alternating pattern 0101010101 (0x155).
 *
 * @par Test Vector:
 * - Odd bits ON, even bits OFF
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 0101010101 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x55 | 01010101 | flags[0:7]=01010101         |
 * | 1    | 0x40 | 01000000 | flags[8:9]=01 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_alternating_0101010101(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* flags[0:7]=01010101 */
      0x40,                                          /* flags[8:9]=01 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x155U, J2735_LANE_SHARING_GET(payload),
                                  "Inverse alternating pattern should be 0x155");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(payload),
      "bit 0: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_MULTIPLE_LANES_TREATED_AS_ONE_LANE(payload),
      "bit 1: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_SHARING_GET_TRACKED_VEHICLE_TRAFFIC(payload),
                                  "bit 8: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_RESERVED(payload),
                                  "bit 9: should be ON");
}

/**
 * @brief Test LaneSharing with only overlappingLaneDescriptionProvided (bit 0) set.
 *
 * @par Test Vector:
 * - overlappingLaneDescriptionProvided(0): ON, all others: OFF
 * - flags = 0x200
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 1000000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x80 | 10000000 | flags[0:7]=10000000         |
 * | 1    | 0x00 | 00000000 | flags[8:9]=00 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_single_bit_overlapping(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* flags[0:7]=10000000 */
      0x00,                                          /* flags[8:9]=00 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x200U, J2735_LANE_SHARING_GET(payload),
                                  "Only bit 0 should be set (0x200)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(payload),
      "overlappingLaneDescriptionProvided should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_LANE_SHARING_GET_RESERVED(payload),
                                  "reserved should be OFF");
}

/**
 * @brief Test LaneSharing with only reserved (bit 9) set.
 *
 * @par Test Vector:
 * - reserved(9): ON, all others: OFF
 * - flags = 0x001
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 0000000001 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x00 | 00000000 | flags[0:7]=00000000         |
 * | 1    | 0x40 | 01000000 | flags[8:9]=01 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_single_bit_reserved(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x40,                                          /* flags[8:9]=01 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x001U, J2735_LANE_SHARING_GET(payload),
                                  "Only bit 9 should be set (0x001)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(payload),
      "overlappingLaneDescriptionProvided should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_RESERVED(payload),
                                  "reserved should be ON");
}

/**
 * @brief Test LaneSharing SIZE macro returns fixed 10 bits.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_size(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* flags[0:7]=00000000 */
      0x00,                                          /* flags[8:9]=00 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(10U, J2735_LANE_SHARING_SIZE(payload),
                                   "SIZE should always be 10 for LaneSharing");
}

/**
 * @brief Test LaneSharing HAS_EXTENSION always returns false.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_has_extension(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* flags[0:7]=11111111 */
      0xC0,                                          /* flags[8:9]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_LANE_SHARING_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext,
                            "HAS_EXTENSION should always be false for non-extensible type");
}

/**
 * @brief Test LaneSharing with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - All 10 flags: ON (0x3FF)
 *
 * @par Wire Format (10 bits total):
 * | Offset (bits) | Width | Field       | Value      |
 * |---------------|-------|-------------|------------|
 * | 0             | 10    | flags[0:9]  | 1111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0xFF | 11111111 | flags[0:7]=11111111         |
 * | 1    | 0xC0 | 11000000 | flags[8:9]=11 + pad(6)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_lane_sharing_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0xFF,                                          /* flags[0:7]=11111111 */
      0xC0,                                          /* flags[8:9]=11 + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x3FFU, J2735_LANE_SHARING_GET(unaligned_ptr),
                                  "Misaligned: all flags should be ON (0x3FF)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(unaligned_ptr),
      "Misaligned: overlappingLaneDescriptionProvided should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_LANE_SHARING_GET_RESERVED(unaligned_ptr),
                                  "Misaligned: reserved should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all LaneSharing tests.
 */
void run_testsuite_lane_sharing(void) {
  RUN_TEST(test_lane_sharing_all_zeros);
  RUN_TEST(test_lane_sharing_all_ones_bits_0_to_4);
  RUN_TEST(test_lane_sharing_all_ones_bits_5_to_9);
  RUN_TEST(test_lane_sharing_alternating_1010101010);
  RUN_TEST(test_lane_sharing_alternating_0101010101);
  RUN_TEST(test_lane_sharing_single_bit_overlapping);
  RUN_TEST(test_lane_sharing_single_bit_reserved);
  RUN_TEST(test_lane_sharing_size);
  RUN_TEST(test_lane_sharing_has_extension);
  RUN_TEST(test_lane_sharing_misaligned_access);
}
