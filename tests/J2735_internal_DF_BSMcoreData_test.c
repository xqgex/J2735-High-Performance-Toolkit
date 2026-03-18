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
 * @brief Tests for BSMcoreData non-extensible SEQUENCE.
 *
 * The data frame BSMcoreData is a simple case with no extensions or optional fields.
 */

#include <inttypes.h>
#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DF_BSMcoreData.h"
#include "J2735_internal_DF_BSMcoreData_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test BSMcoreData field extraction (fixed-layout SEQUENCE).
 *
 * @par ASN.1 Definition:
 * @code
 * BSMcoreData ::= SEQUENCE {
 *   msgCnt       MsgCount,       --  7 bits (unsigned, 0..127)
 *   id           TemporaryID,    -- 32 bits (OCTET STRING SIZE(4))
 *   secMark      DSecond,        -- 16 bits (unsigned, 0..65535)
 *   lat          Latitude,       -- 31 bits (signed)
 *   long         Longitude,
 *   elev         Elevation,
 *   accuracy     PositionalAccuracy,
 *   transmission TransmissionState,
 *   speed        Speed,
 *   heading      Heading,
 *   angle        SteeringWheelAngle,
 *   accelSet     AccelerationSet4Way,
 *   brakes       BrakeSystemStatus,
 *   size         VehicleSize
 * }
 * @endcode
 *
 * @par Test Vector:
 * - msgCnt:  10 (0x0A)
 * - id:      0xDEADBEEF
 * - secMark: 45000 (0xAFC8)
 * - lat:     410123450 (0x1871FCBA)
 *
 * @par Wire Format (86 bits shown):
 * | Offset (bits) | Width | Field   | Value                           |
 * |---------------|-------|---------|---------------------------------|
 * | 0             | 7     | msgCnt  | 0001010                         |
 * | 7             | 32    | id      | 11011110101011011011111011101111 |
 * | 39            | 16    | secMark | 1010111111001000                |
 * | 55            | 31    | lat     | 0110000111000111110010111010    |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x15 | 00010101 | msgCnt[6:0] + id[31]        |
 * | 1    | 0xBD | 10111101 | id[30:23]                   |
 * | 2    | 0x5B | 01011011 | id[22:15]                   |
 * | 3    | 0x7D | 01111101 | id[14:7]                    |
 * | 4    | 0xDF | 11011111 | id[6:0] + secMark[15]       |
 * | 5    | 0x5F | 01011111 | secMark[14:7]               |
 * | 6    | 0x90 | 10010000 | secMark[6:0] + lat[30]      |
 * | 7    | 0x61 | 01100001 | lat[29:22]                  |
 * | 8    | 0xC7 | 11000111 | lat[21:14]                  |
 * | 9    | 0xF2 | 11110010 | lat[13:6]                   |
 * | 10   | 0xE8 | 11101000 | lat[5:0] + padding          |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_bsm_core_data_fixed_data(void) {
  static const uint8_t payload[] = {
      0x15,                                          /* msgCnt[6:0] + id[31] */
      0xBD,                                          /* id[30:23] */
      0x5B,                                          /* id[22:15] */
      0x7D,                                          /* id[14:7] */
      0xDF,                                          /* id[6:0] + secMark[15] */
      0x5F,                                          /* secMark[14:7] */
      0x90,                                          /* secMark[6:0] + lat[30] */
      0x61,                                          /* lat[29:22] */
      0xC7,                                          /* lat[21:14] */
      0xF2,                                          /* lat[13:6] */
      0xE8,                                          /* lat[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  uint8_t msg_cnt = J2735_BSM_CORE_DATA_GET_MSG_CNT(payload);
  int32_t lat = J2735_BSM_CORE_DATA_GET_LAT(payload);

  TEST_ASSERT_EQUAL_INT_MESSAGE(10, msg_cnt, "msgCnt should be 10");
  TEST_ASSERT_EQUAL_INT_MESSAGE(410123450, lat, "lat should be 410123450");
}

/**
 * @brief Test BSMcoreData with negative latitude at southern boundary.
 *
 * @par ASN.1 Definition:
 * @code
 * BSMcoreData ::= SEQUENCE {
 *   ...
 *   lat          Latitude,             -- 31 bits (signed, -900000000..900000001)
 *   ...
 * }
 * @endcode
 *
 * @par Test Vector:
 * - msgCnt: 0
 * - id: 0x00000000
 * - secMark: 0
 * - lat: -900000000 (minimum valid, represents -90.0 degrees)
 *
 * @par Wire Format (first 86 bits):
 * | Offset (bits) | Width | Field   | Value                                   |
 * |---------------|-------|---------|-----------------------------------------|
 * | 0             | 7     | msgCnt  | 0000000 (0)                             |
 * | 7             | 32    | id      | 00000000000000000000000000000000        |
 * | 39            | 16    | secMark | 0000000000000000                        |
 * | 55            | 31    | lat     | 1001010100100010101100000000000 (-900M) |
 *
 * @par Byte Encoding (first 11 bytes):
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x00 | 00000000 | msgCnt[6:0] + id[31]         |
 * | 1    | 0x00 | 00000000 | id[30:23]                    |
 * | 2    | 0x00 | 00000000 | id[22:15]                    |
 * | 3    | 0x00 | 00000000 | id[14:7]                     |
 * | 4    | 0x00 | 00000000 | id[6:0] + secMark[15]        |
 * | 5    | 0x00 | 00000000 | secMark[14:7]                |
 * | 6    | 0x00 | 00000000 | secMark[6:0] + lat[30]       |
 * | 7    | 0x95 | 10010101 | lat[29:22] (0x95)            |
 * | 8    | 0x22 | 00100010 | lat[21:14] (0x22)            |
 * | 9    | 0xB0 | 10110000 | lat[13:6] (0xB0)             |
 * | 10   | 0x00 | 00000000 | lat[5:0] + long[31:30]       |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_bsm_core_data_latitude_negative_min(void) {
  /*
   * Latitude = -900000000 encoded as a 31-bit two's complement value.
   *
   * In the BSMcoreData wire format, latitude starts at bit 55 of the payload:
   *   - byte 6, bit 0 carries lat[30] (the sign bit for this 31-bit field)
   *   - byte 7 carries  lat[29:22]
   *   - byte 8 carries  lat[21:14]
   *   - byte 9 carries  lat[13:6]
   *   - byte 10 bits 7:2 carry lat[5:0] (bits 1:0 are the start of longitude)
   *
   * This payload chooses secMark such that its lower 7 bits are zero, so
   * byte 6 = 0x01 encodes lat[30] = 1 (negative latitude) with no overlap
   * in the secMark field. Bytes 7–10 then hold the remaining latitude bits.
   */
  static const uint8_t payload[] = {
      0x00, /* msgCnt[6:0] + id[31] */
      0x00, /* id[30:23] */
      0x00, /* id[22:15] */
      0x00, /* id[14:7] */
      0x00, /* id[6:0] + secMark[15] */
      0x00, /* secMark[14:7] */
      0x01, /* secMark[6:0] + lat[30] (lat[30]=1 for negative) */
      0x29, /* lat[29:22] = 0x29 */
      0x6C, /* lat[21:14] = 0x6C */
      0x5C, /* lat[13:6] = 0x5C */
      0x00, /* lat[5:0] + long[31:30] = 000000 00 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  int32_t lat = J2735_BSM_CORE_DATA_GET_LAT(payload);
  TEST_ASSERT_EQUAL_INT32_MESSAGE(-900000000, lat, "lat should be -900000000 (min valid)");
}

/**
 * @brief Test BSMcoreData with maximum positive latitude.
 *
 * @par ASN.1 Definition:
 * @code
 * BSMcoreData ::= SEQUENCE {
 *   ...
 *   lat          Latitude,             -- 31 bits (signed, -900000000..900000001)
 *   ...
 * }
 * @endcode
 *
 * @par Test Vector:
 * - lat: 900000000 (represents +90.0 degrees, max practical value)
 *
 * @par Wire Format:
 * | Offset (bits) | Width | Field | Value                                   |
 * |---------------|-------|-------|-----------------------------------------|
 * | 55            | 31    | lat   | 0110101101000001110010100000000 (+900M) |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_bsm_core_data_latitude_positive_max(void) {
  /*
   * 900000000 = 0x35A4E900
   * 31-bit: 0 110101101000001110010100000000
   *
   * Byte 6 bit 0 = lat[30] = 0
   * Byte 7 = lat[29:22] = 11010110 = 0xD6
   * Byte 8 = lat[21:14] = 10000011 = 0x83
   * Byte 9 = lat[13:6] = 10010100 = 0x94
   * Byte 10 bits 7:2 = lat[5:0] = 000000
   */
  static const uint8_t payload[] = {
      0x00, /* msgCnt[6:0] + id[31] */
      0x00, /* id[30:23] */
      0x00, /* id[22:15] */
      0x00, /* id[14:7] */
      0x00, /* id[6:0] + secMark[15] */
      0x00, /* secMark[14:7] */
      0x00, /* secMark[6:0] + lat[30] (lat[30]=0 for positive) */
      0xD6, /* lat[29:22] = 0xD6 */
      0x93, /* lat[21:14] = 0x93 */
      0xA4, /* lat[13:6] = 0xA4 */
      0x00, /* lat[5:0] + long[31:30] = 000000 00 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  int32_t lat = J2735_BSM_CORE_DATA_GET_LAT(payload);
  TEST_ASSERT_EQUAL_INT32_MESSAGE(900000000, lat, "lat should be 900000000 (max practical)");
}

/**
 * @brief Test BSMcoreData with negative steering wheel angle (sign extension).
 *
 * @par ASN.1 Definition:
 * @code
 * BSMcoreData ::= SEQUENCE {
 *   ...
 *   angle        SteeringWheelAngle,   --  8 bits (signed, -126..127)
 *   ...
 * }
 * @endcode
 *
 * @par Test Vector:
 * - angle: -126 (minimum valid, 0x82 in two's complement)
 *
 * @par Wire Format:
 * angle is at bit offset 197, spanning bytes 24-25.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_bsm_core_data_steering_angle_negative(void) {
  /*
   * -126 in 8-bit two's complement = 0x82 = 10000010
   * angle starts at bit 197
   *
   * From wire format:
   * | +24    | Heading (Bits 4-0) | SteeringWheelAngle (Bits 7-5)                   |
   * | +25    | SteeringWheelAngle (Bits 4-0) | AccelerationSet4Way (Bits 47-45)     |
   *
   * So: byte 24 bits 2:0 = angle[7:5] = 00000_100 = 0x04 = 4
   *     byte 25 bits 7:3 = angle[4:0] = 00010_000 = 0x10 = 2
   *
   * byte 24 = 0x04 (just angle bits, heading zeros)
   * byte 25 = 0x10 (angle[4:0]=2 shifted to bits 7:3)
   */
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 0-7 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 8-15 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 16-23 */
      0x04, /* byte 24: heading[4:0](00000) + angle[7:5](100) = 00000100 */
      0x10, /* byte 25: angle[4:0](00010) + accelSet[47:45](000) = 00010000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 26-33 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* safety padding */
  };

  int8_t angle = J2735_BSM_CORE_DATA_GET_ANGLE(payload);
  TEST_ASSERT_EQUAL_INT8_MESSAGE(-126, angle, "angle should be -126 (min valid)");
}

/**
 * @brief Test BSMcoreData with maximum positive steering wheel angle.
 *
 * @par Test Vector:
 * - angle: 127 (maximum, 0x7F)
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_bsm_core_data_steering_angle_positive_max(void) {
  /*
   * 127 in 8-bit = 0x7F = 01111111
   * From wire format:
   * | +24    | Heading (Bits 4-0) | SteeringWheelAngle (Bits 7-5)                   |
   * | +25    | SteeringWheelAngle (Bits 4-0) | AccelerationSet4Way (Bits 47-45)     |
   *
   * So: byte 24 bits 2:0 = angle[7:5] = 011 = 3
   *     byte 25 bits 7:3 = angle[4:0] = 11111 = 31
   *
   * byte 24 = 0x03 (just angle bits, heading zeros)
   * byte 25 = 0xF8 (angle[4:0]=31 shifted to bits 7:3)
   */
  static const uint8_t payload[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 0-7 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 8-15 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 16-23 */
      0x03, /* byte 24: heading[4:0](00000) + angle[7:5](011) = 00000011 */
      0xF8, /* byte 25: angle[4:0](11111) + accelSet[47:45](000) = 11111000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bytes 26-33 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* safety padding */
  };

  int8_t angle = J2735_BSM_CORE_DATA_GET_ANGLE(payload);
  TEST_ASSERT_EQUAL_INT8_MESSAGE(127, angle, "angle should be 127 (max)");
}

/**
 * @brief Test BSMcoreData with misaligned buffer access.
 *
 * Since this is an embedded library, we must verify correct operation when
 * the buffer is not aligned to a natural boundary. This tests the packed-cast
 * optimization used by J2735_READ_BITS.
 *
 * Uses the same test vector as test_bsm_core_data_fixed_data but with
 * a 1-byte offset to force misalignment.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_bsm_core_data_misaligned_access(void) {
  /* Same payload as test_bsm_core_data_fixed_data with 1-byte prefix for misalignment */
  static const uint8_t payload[] = {
      0xFF, /* padding byte to force misalignment */
      0x15, /* msgCnt[6:0] + id[31] */
      0xBD, /* id[30:23] */
      0x5B, /* id[22:15] */
      0x7D, /* id[14:7] */
      0xDF, /* id[6:0] + secMark[15] */
      0x5F, /* secMark[14:7] */
      0x90, /* secMark[6:0] + lat[30] */
      0x61, /* lat[29:22] */
      0xC7, /* lat[21:14] */
      0xF2, /* lat[13:6] */
      0xE8, /* lat[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Offset pointer by 1 byte to force misalignment */
  const uint8_t *unaligned_ptr = &payload[1];

  uint8_t msg_cnt = J2735_BSM_CORE_DATA_GET_MSG_CNT(unaligned_ptr);
  int32_t lat = J2735_BSM_CORE_DATA_GET_LAT(unaligned_ptr);

  TEST_ASSERT_EQUAL_INT_MESSAGE(10, msg_cnt, "msgCnt should be 10 (misaligned)");
  TEST_ASSERT_EQUAL_INT_MESSAGE(410123450, lat, "lat should be 410123450 (misaligned)");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

void run_testsuite_bsm_core_data(void) {
  /* Happy path tests */
  RUN_TEST(test_bsm_core_data_fixed_data);

  /* Boundary value tests - signed fields */
  RUN_TEST(test_bsm_core_data_latitude_negative_min);
  RUN_TEST(test_bsm_core_data_latitude_positive_max);
  RUN_TEST(test_bsm_core_data_steering_angle_negative);
  RUN_TEST(test_bsm_core_data_steering_angle_positive_max);

  /* Misalignment tests */
  RUN_TEST(test_bsm_core_data_misaligned_access);
}
