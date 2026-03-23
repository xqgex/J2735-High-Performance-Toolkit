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
 * @brief Tests for PublicSafetyAndRoadWorkerActivity extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * PublicSafetyAndRoadWorkerActivity ::= BIT STRING {
 *     unavailable        (0),
 *     workingOnRoad      (1),
 *     settingUpClosures  (2),
 *     respondingToEvents (3),
 *     directingTraffic   (4),
 *     otherActivities    (5)
 * } (SIZE (6, ...))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Non-extended form (7 bits): [ext=0][6 flag bits]
 * - Extended form (14 bits): [ext=1][nsnnwn=7 bits][6 flag bits]
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_PublicSafetyAndRoadWorkerActivity.h"
#include "J2735_internal_DE_PublicSafetyAndRoadWorkerActivity_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity with non-extended form, typical value.
 *
 * @par Test Vector:
 * - Flags: 0x2A = 101010 (unavailable=1,workingOnRoad=0,settingUpClosures=1,
 *                          respondingToEvents=0,directingTraffic=1,otherActivities=0)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x54 | 01010100 | ext(0)+flags(101010)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_non_extended(void) {
  static const uint8_t payload[] = {
      0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Extension bit should be 0 for non-extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x2AU, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "Flags should be 0x2A for non-extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(7U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SIZE(payload),
                                   "Size should be 7 for non-extended form");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity with extended form, all flags ON.
 *
 * @par Test Vector:
 * - nsnnwn value: 6 (small form: 0 + 000110 = 0b0000110)
 * - Flags: 0x3F (all 6 bits set)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x86 | 10000110 | ext(1)+nsnnwn(0000110)       |
 * | 1    | 0xFC | 11111100 | flags(111111)+pad(2)         |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_extended(void) {
  static const uint8_t payload[] = {
      0x86, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Extension bit should be 1 for extended form");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3FU, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "Flags should be 0x3F for extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(14U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SIZE(payload),
                                   "Size should be 14 for extended form");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity individual flag accessors (non-extended, all ON).
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x7E | 01111110 | ext(0)+flags(111111)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_non_extended_flags(void) {
  static const uint8_t payload[] = {
      0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_UNAVAILABLE(payload),
      "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_WORKING_ON_ROAD(payload),
      "bit 1: workingOnRoad should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_SETTING_UP_CLOSURES(payload),
      "bit 2: settingUpClosures should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_RESPONDING_TO_EVENTS(payload),
      "bit 3: respondingToEvents should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_DIRECTING_TRAFFIC(payload),
      "bit 4: directingTraffic should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_OTHER_ACTIVITIES(payload),
      "bit 5: otherActivities should be ON");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity SIZE for non-extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_size_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(7U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SIZE(payload),
                                   "Non-extended size should be 7 bits");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity SIZE for extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_size_extended(void) {
  static const uint8_t payload[] = {
      0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(14U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SIZE(payload),
                                   "Extended size should be 14 bits");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity all zeros, non-extended.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_all_zeros_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "All flags should be zero");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity all root flags ON, non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x7E | 01111110 | ext(0)+flags(111111)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_non_extended_all_flags_on(void) {
  static const uint8_t payload[] = {
      0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3FU, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "All 6 flags should be ON (0x3F)");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity extended form, all flags zero.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_extended_all_zeros(void) {
  static const uint8_t payload[] = {
      0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Should be extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x00U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "All flags should be zero in extended form");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity alternating 101010 (0x2A), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x54 | 01010100 | ext(0)+flags(101010)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_non_extended_alternating_101010(void) {
  static const uint8_t payload[] = {
      0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x2AU, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "Alternating pattern should be 0x2A");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_UNAVAILABLE(payload),
      "bit 0: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_WORKING_ON_ROAD(payload),
      "bit 1: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_SETTING_UP_CLOSURES(payload),
      "bit 2: should be ON");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity alternating 010101 (0x15), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x2A | 00101010 | ext(0)+flags(010101)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_non_extended_alternating_010101(void) {
  static const uint8_t payload[] = {
      0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x15U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "Inverse alternating pattern should be 0x15");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_UNAVAILABLE(payload),
      "bit 0: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_WORKING_ON_ROAD(payload),
      "bit 1: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_SETTING_UP_CLOSURES(payload),
      "bit 2: should be OFF");
}

/**
 * @brief Test single bit 0 (unavailable), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x40 | 01000000 | ext(0)+flags(100000)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_single_bit_0_unavailable(void) {
  static const uint8_t payload[] = {
      0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x20U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "Only bit 0 should be set (0x20)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_UNAVAILABLE(payload),
      "unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_OTHER_ACTIVITIES(payload),
      "otherActivities should be OFF");
}

/**
 * @brief Test single bit 5 (otherActivities), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x02 | 00000010 | ext(0)+flags(000001)+pad(1)|
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_single_bit_5_other_activities(void) {
  static const uint8_t payload[] = {
      0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x01U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(payload),
                                 "Only bit 5 should be set (0x01)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_UNAVAILABLE(payload),
      "unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_OTHER_ACTIVITIES(payload),
      "otherActivities should be ON");
}

/**
 * @brief Test PublicSafetyAndRoadWorkerActivity with misaligned buffer pointer.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_public_safety_and_road_worker_activity_misaligned_access(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* junk byte for misalignment */
      0x7E,                                          /* ext(0)+flags(111111)+pad(1) */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  bool has_ext = J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(unaligned_ptr);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Misaligned: should be non-extended");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3FU,
                                 J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(unaligned_ptr),
                                 "Misaligned: all flags should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all PublicSafetyAndRoadWorkerActivity tests.
 */
void run_testsuite_public_safety_and_road_worker_activity(void) {
  RUN_TEST(test_public_safety_and_road_worker_activity_non_extended);
  RUN_TEST(test_public_safety_and_road_worker_activity_extended);
  RUN_TEST(test_public_safety_and_road_worker_activity_non_extended_flags);
  RUN_TEST(test_public_safety_and_road_worker_activity_size_non_extended);
  RUN_TEST(test_public_safety_and_road_worker_activity_size_extended);
  RUN_TEST(test_public_safety_and_road_worker_activity_all_zeros_non_extended);
  RUN_TEST(test_public_safety_and_road_worker_activity_non_extended_all_flags_on);
  RUN_TEST(test_public_safety_and_road_worker_activity_extended_all_zeros);
  RUN_TEST(test_public_safety_and_road_worker_activity_non_extended_alternating_101010);
  RUN_TEST(test_public_safety_and_road_worker_activity_non_extended_alternating_010101);
  RUN_TEST(test_public_safety_and_road_worker_activity_single_bit_0_unavailable);
  RUN_TEST(test_public_safety_and_road_worker_activity_single_bit_5_other_activities);
  RUN_TEST(test_public_safety_and_road_worker_activity_misaligned_access);
}
