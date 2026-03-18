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
 * @brief Tests for ApproachOrLane CHOICE type.
 *
 * ApproachOrLane is a non-extensible CHOICE with 2 alternatives.
 * This validates CHOICE index reading and alternative value extraction.
 *
 * All tests use the efficient single-I/O pattern:
 * @code
 * uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(buf);
 * switch (J2735_APPROACH_OR_LANE_WHICH(raw9)) { ... }
 * @endcode
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DF_ApproachOrLane.h"
#include "J2735_internal_DF_ApproachOrLane_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test ApproachOrLane with 'approach' alternative selected (typical value).
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: approach (index = 0)
 * - Value: 5 (0x05) - typical mid-range value
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field    | Value |
 * |---------------|-------|----------|-------|
 * | 0             | 1     | index    | 0     |
 * | 1             | 4     | approach | 0101  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x28 | 00101000 | index(0) + approach(0101) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_approach_typical(void) {
  static const uint8_t payload[] = {
      0x28,                                          /* index(0) + approach(0101) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Single I/O read (9 bits) - all subsequent operations are pure computation */
  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(payload);

  /* Verify WHICH returns the correct alternative index (1 bit) */
  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_APPROACH, which,
                                  "WHICH should return 0 for approach");

  /* Verify GET_APPROACH returns the correct value (4 bits) */
  uint8_t const approach_value = J2735_APPROACH_OR_LANE_GET_APPROACH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5U, approach_value, "approach value should be 5");

  /* Verify SIZE calculation (returns 5 or 9) */
  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5U, size, "size should be 5 bits (1 index + 4 value)");
}

/**
 * @brief Test ApproachOrLane with 'lane' alternative selected (typical value).
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: lane (index = 1)
 * - Value: 171 (0xAB) - typical mid-range value with alternating bits
 *
 * @par Wire Format (9 bits total):
 * | Offset (bits) | Width | Field | Value    |
 * |---------------|-------|-------|----------|
 * | 0             | 1     | index | 1        |
 * | 1             | 8     | lane  | 10101011 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0xD5 | 11010101 | index(1) + lane[7:1](1010101)  |
 * | 1    | 0x80 | 10000000 | lane[0](1) + padding(0000000)  |
 *
 * Note: The lane value is 0xAB = 10101011b. Under UPER bit-packing, the MSB 7 bits
 * (1010101) are placed after the index bit in byte 0, while the LSB (1) becomes the
 * MSB of byte 1. This split explains why lane[7:1] are in byte 0 and lane[0] is in byte 1.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_lane_typical(void) {
  static const uint8_t payload[] = {
      0xD5,                                          /* index(1) + lane[7:1](1010101) */
      0x80,                                          /* lane[0](1) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Single I/O read (9 bits) - all subsequent operations are pure computation */
  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(payload);

  /* Verify WHICH returns the correct alternative index (1 bit) */
  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_LANE, which,
                                  "WHICH should return 1 for lane");

  /* Verify GET_LANE returns the correct value (8 bits) */
  uint8_t const lane_value = J2735_APPROACH_OR_LANE_GET_LANE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xABU, lane_value, "lane value should be 0xAB");

  /* Verify SIZE calculation (returns 5 or 9) */
  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(9U, size, "size should be 9 bits (1 index + 8 value)");
}

/**
 * @brief Test ApproachOrLane boundary: approach with minimum value 0.
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: approach (index = 0)
 * - Value: 0 (minimum) - per J2735 spec, 0 means "unknown"
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field    | Value |
 * |---------------|-------|----------|-------|
 * | 0             | 1     | index    | 0     |
 * | 1             | 4     | approach | 0000  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x00 | 00000000 | index(0) + approach(0000) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_approach_boundary_min(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* index(0) + approach(0000) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(payload);

  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_APPROACH, which,
                                  "WHICH should return 0 for approach");

  uint8_t const approach_value = J2735_APPROACH_OR_LANE_GET_APPROACH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, approach_value, "approach value should be 0 (minimum)");

  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5U, size, "size should be 5 bits for approach");
}

/**
 * @brief Test ApproachOrLane boundary: approach with maximum value 15.
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: approach (index = 0)
 * - Value: 15 (0x0F) - maximum for 4-bit field
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field    | Value |
 * |---------------|-------|----------|-------|
 * | 0             | 1     | index    | 0     |
 * | 1             | 4     | approach | 1111  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x78 | 01111000 | index(0) + approach(1111) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_approach_boundary_max(void) {
  static const uint8_t payload[] = {
      0x78,                                          /* index(0) + approach(1111) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(payload);

  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_APPROACH, which,
                                  "WHICH should return 0 for approach");

  uint8_t const approach_value = J2735_APPROACH_OR_LANE_GET_APPROACH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(15U, approach_value, "approach value should be 15 (maximum)");

  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5U, size, "size should be 5 bits for approach");
}

/**
 * @brief Test ApproachOrLane boundary: lane with minimum value 0.
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: lane (index = 1)
 * - Value: 0 (minimum) - per J2735 spec, 0 means "unavailable/unknown"
 *
 * @par Wire Format (9 bits total):
 * | Offset (bits) | Width | Field | Value    |
 * |---------------|-------|-------|----------|
 * | 0             | 1     | index | 1        |
 * | 1             | 8     | lane  | 00000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x80 | 10000000 | index(1) + lane[7:1](0000000)  |
 * | 1    | 0x00 | 00000000 | lane[0](0) + padding(0000000)  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_lane_boundary_min(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* index(1) + lane[7:1](0000000) */
      0x00,                                          /* lane[0](0) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(payload);

  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_LANE, which,
                                  "WHICH should return 1 for lane");

  uint8_t const lane_value = J2735_APPROACH_OR_LANE_GET_LANE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, lane_value, "lane value should be 0 (minimum)");

  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(9U, size, "size should be 9 bits for lane");
}

/**
 * @brief Test ApproachOrLane boundary: lane with maximum value 255.
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: lane (index = 1)
 * - Value: 255 (0xFF) - maximum for 8-bit field (reserved per spec)
 *
 * @par Wire Format (9 bits total):
 * | Offset (bits) | Width | Field | Value    |
 * |---------------|-------|-------|----------|
 * | 0             | 1     | index | 1        |
 * | 1             | 8     | lane  | 11111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0xFF | 11111111 | index(1) + lane[7:1](1111111)  |
 * | 1    | 0x80 | 10000000 | lane[0](1) + padding(0000000)  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_lane_boundary_max(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* index(1) + lane[7:1](1111111) */
      0x80,                                          /* lane[0](1) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(payload);

  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_LANE, which,
                                  "WHICH should return 1 for lane");

  uint8_t const lane_value = J2735_APPROACH_OR_LANE_GET_LANE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(255U, lane_value, "lane value should be 255 (maximum)");

  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(9U, size, "size should be 9 bits for lane");
}

/**
 * @brief Test ApproachOrLane with misaligned buffer access (approach alternative).
 *
 * Since this is an embedded library, we must verify correct operation when
 * the buffer is not aligned to a natural boundary. This tests the packed-cast
 * optimization used by J2735_READ_BITS.
 *
 * @par ASN.1 Definition:
 * @code
 * ApproachOrLane ::= CHOICE {
 *   approach  ApproachID,  -- 4 bits, INTEGER (0..15)
 *   lane      LaneID       -- 8 bits, INTEGER (0..255)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - Alternative: approach (index = 0)
 * - Value: 10 (0x0A)
 *
 * @par Wire Format (5 bits total):
 * | Offset (bits) | Width | Field    | Value |
 * |---------------|-------|----------|-------|
 * | 0             | 1     | index    | 0     |
 * | 1             | 4     | approach | 1010  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                           |
 * |------|------|----------|----------------------------------|
 * | 0    | 0x50 | 01010000 | index(0) + approach(1010) + pad  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_approach_or_lane_misaligned_access(void) {
  /* Deliberately misalign buffer by placing padding byte at start */
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0x50,                                          /* index(0) + approach(1010) + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Offset pointer by 1 byte to force misalignment */
  const uint8_t *unaligned_ptr = &payload[1];

  uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(unaligned_ptr);

  uint8_t const which = J2735_APPROACH_OR_LANE_WHICH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(J2735_CHOICE_APPROACH_OR_LANE_APPROACH, which,
                                  "WHICH should return 0 for approach (misaligned)");

  uint8_t const approach_value = J2735_APPROACH_OR_LANE_GET_APPROACH(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(10U, approach_value,
                                  "approach value should be 10 (misaligned access)");

  uint8_t const size = J2735_APPROACH_OR_LANE_SIZE(raw9);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5U, size, "size should be 5 bits for approach (misaligned)");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

void run_testsuite_approach_or_lane(void) {
  /* Happy path tests */
  RUN_TEST(test_approach_or_lane_approach_typical);
  RUN_TEST(test_approach_or_lane_lane_typical);

  /* Boundary value tests - approach */
  RUN_TEST(test_approach_or_lane_approach_boundary_min);
  RUN_TEST(test_approach_or_lane_approach_boundary_max);

  /* Boundary value tests - lane */
  RUN_TEST(test_approach_or_lane_lane_boundary_min);
  RUN_TEST(test_approach_or_lane_lane_boundary_max);

  /* Misalignment tests */
  RUN_TEST(test_approach_or_lane_misaligned_access);
}
