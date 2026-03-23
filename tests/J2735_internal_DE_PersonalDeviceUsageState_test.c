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
 * @brief Tests for PersonalDeviceUsageState extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * PersonalDeviceUsageState ::= BIT STRING {
 *     unavailable      (0),
 *     other            (1),
 *     idle             (2),
 *     listeningToAudio (3),
 *     typing           (4),
 *     calling          (5),
 *     playingGames     (6),
 *     reading          (7),
 *     viewing          (8)
 * } (SIZE (9, ...))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Non-extended form (10 bits): [ext=0][9 flag bits]
 * - Extended form (17 bits): [ext=1][nsnnwn=7 bits][9 flag bits]
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_PersonalDeviceUsageState.h"
#include "J2735_internal_DE_PersonalDeviceUsageState_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test PersonalDeviceUsageState with non-extended form, typical value.
 *
 * @par Test Vector:
 * - Flags: 0x0155 = 101010101
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0)+flags[0..6](1010101)      |
 * | 1    | 0x40 | 01000000 | flags[7..8](01)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_non_extended(void) {
  static const uint8_t payload[] = {
      0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PERSONAL_DEVICE_USAGE_STATE_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Extension bit should be 0 for non-extended form");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0155U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "Flags should be 0x0155 for non-extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(10U, J2735_PERSONAL_DEVICE_USAGE_STATE_SIZE(payload),
                                   "Size should be 10 for non-extended form");
}

/**
 * @brief Test PersonalDeviceUsageState with extended form, all flags ON.
 *
 * @par Test Vector:
 * - nsnnwn value: 9 (small form: 0 + 001001 = 0b0001001)
 * - Flags: 0x01FF (all 9 bits set)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x89 | 10001001 | ext(1)+nsnnwn(0001001)       |
 * | 1    | 0xFF | 11111111 | flags[0..7]                  |
 * | 2    | 0x80 | 10000000 | flags[8]+pad(7)              |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_extended(void) {
  static const uint8_t payload[] = {
      0x89, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PERSONAL_DEVICE_USAGE_STATE_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Extension bit should be 1 for extended form");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x01FFU, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "Flags should be 0x01FF for extended form");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(17U, J2735_PERSONAL_DEVICE_USAGE_STATE_SIZE(payload),
                                   "Size should be 17 for extended form");
}

/**
 * @brief Test PersonalDeviceUsageState individual flag accessors (non-extended, all ON).
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x7F | 01111111 | ext(0)+flags[0..6](1111111)      |
 * | 1    | 0xC0 | 11000000 | flags[7..8](11)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_non_extended_flags(void) {
  static const uint8_t payload[] = {
      0x7F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_UNAVAILABLE(payload),
                                  "bit 0: unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_OTHER(payload),
                                  "bit 1: other should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_IDLE(payload),
                                  "bit 2: idle should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_PERSONAL_DEVICE_USAGE_STATE_GET_LISTENING_TO_AUDIO(payload),
                                  "bit 3: listeningToAudio should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_TYPING(payload),
                                  "bit 4: typing should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_CALLING(payload),
                                  "bit 5: calling should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_PLAYING_GAMES(payload),
                                  "bit 6: playingGames should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_READING(payload),
                                  "bit 7: reading should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_VIEWING(payload),
                                  "bit 8: viewing should be ON");
}

/**
 * @brief Test PersonalDeviceUsageState SIZE for non-extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_size_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(10U, J2735_PERSONAL_DEVICE_USAGE_STATE_SIZE(payload),
                                   "Non-extended size should be 10 bits");
}

/**
 * @brief Test PersonalDeviceUsageState SIZE for extended form.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_size_extended(void) {
  static const uint8_t payload[] = {
      0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT32_MESSAGE(17U, J2735_PERSONAL_DEVICE_USAGE_STATE_SIZE(payload),
                                   "Extended size should be 17 bits");
}

/**
 * @brief Test PersonalDeviceUsageState all zeros, non-extended.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_all_zeros_non_extended(void) {
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PERSONAL_DEVICE_USAGE_STATE_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0000U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "All flags should be zero");
}

/**
 * @brief Test PersonalDeviceUsageState all root flags ON, non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x7F | 01111111 | ext(0)+flags[0..6](1111111)      |
 * | 1    | 0xC0 | 11000000 | flags[7..8](11)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_non_extended_all_flags_on(void) {
  static const uint8_t payload[] = {
      0x7F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PERSONAL_DEVICE_USAGE_STATE_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Should be non-extended");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x01FFU, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "All 9 flags should be ON (0x01FF)");
}

/**
 * @brief Test PersonalDeviceUsageState extended form, all flags zero.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_extended_all_zeros(void) {
  static const uint8_t payload[] = {
      0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_ext = J2735_PERSONAL_DEVICE_USAGE_STATE_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Should be extended");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0000U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "All flags should be zero in extended form");
}

/**
 * @brief Test PersonalDeviceUsageState alternating 101010101 (0x155), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0)+flags[0..6](1010101)      |
 * | 1    | 0x40 | 01000000 | flags[7..8](01)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_non_extended_alternating_101010101(void) {
  static const uint8_t payload[] = {
      0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0155U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "Alternating pattern should be 0x0155");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_UNAVAILABLE(payload),
                                  "bit 0: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_OTHER(payload),
                                  "bit 1: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_IDLE(payload),
                                  "bit 2: should be ON");
}

/**
 * @brief Test PersonalDeviceUsageState alternating 010101010 (0x0AA), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x2A | 00101010 | ext(0)+flags[0..6](0101010)      |
 * | 1    | 0x80 | 10000000 | flags[7..8](10)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_non_extended_alternating_010101010(void) {
  static const uint8_t payload[] = {
      0x2A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x00AAU, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "Inverse alternating pattern should be 0x00AA");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_UNAVAILABLE(payload),
                                  "bit 0: should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_OTHER(payload),
                                  "bit 1: should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_IDLE(payload),
                                  "bit 2: should be OFF");
}

/**
 * @brief Test single bit 0 (unavailable), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x40 | 01000000 | ext(0)+flags[0..6](1000000)      |
 * | 1    | 0x00 | 00000000 | flags[7..8](00)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_single_bit_0_unavailable(void) {
  static const uint8_t payload[] = {
      0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0100U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "Only bit 0 should be set (0x0100)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_UNAVAILABLE(payload),
                                  "unavailable should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_VIEWING(payload),
                                  "viewing should be OFF");
}

/**
 * @brief Test single bit 8 (viewing), non-extended.
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x00 | 00000000 | ext(0)+flags[0..6](0000000)      |
 * | 1    | 0x40 | 01000000 | flags[7..8](01)+pad(6)           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_single_bit_8_viewing(void) {
  static const uint8_t payload[] = {
      0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0001U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(payload),
                                  "Only bit 8 should be set (0x0001)");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_UNAVAILABLE(payload),
                                  "unavailable should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_PERSONAL_DEVICE_USAGE_STATE_GET_VIEWING(payload),
                                  "viewing should be ON");
}

/**
 * @brief Test PersonalDeviceUsageState with misaligned buffer pointer.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_personal_device_usage_state_misaligned_access(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* junk byte for misalignment */
      0x7F, 0xC0,                                    /* ext(0)+all flags ON */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  bool has_ext = J2735_PERSONAL_DEVICE_USAGE_STATE_HAS_EXTENSION(unaligned_ptr);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Misaligned: should be non-extended");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x01FFU, J2735_PERSONAL_DEVICE_USAGE_STATE_GET(unaligned_ptr),
                                  "Misaligned: all flags should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all PersonalDeviceUsageState tests.
 */
void run_testsuite_personal_device_usage_state(void) {
  RUN_TEST(test_personal_device_usage_state_non_extended);
  RUN_TEST(test_personal_device_usage_state_extended);
  RUN_TEST(test_personal_device_usage_state_non_extended_flags);
  RUN_TEST(test_personal_device_usage_state_size_non_extended);
  RUN_TEST(test_personal_device_usage_state_size_extended);
  RUN_TEST(test_personal_device_usage_state_all_zeros_non_extended);
  RUN_TEST(test_personal_device_usage_state_non_extended_all_flags_on);
  RUN_TEST(test_personal_device_usage_state_extended_all_zeros);
  RUN_TEST(test_personal_device_usage_state_non_extended_alternating_101010101);
  RUN_TEST(test_personal_device_usage_state_non_extended_alternating_010101010);
  RUN_TEST(test_personal_device_usage_state_single_bit_0_unavailable);
  RUN_TEST(test_personal_device_usage_state_single_bit_8_viewing);
  RUN_TEST(test_personal_device_usage_state_misaligned_access);
}
