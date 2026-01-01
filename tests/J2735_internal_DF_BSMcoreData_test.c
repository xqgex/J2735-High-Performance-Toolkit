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
 * @brief Sanity tests for BSMcoreData.
 *
 * The data frame BSMcoreData is a simple case with no extensions or optional fields.
 */

#include <inttypes.h>
#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DF_BSMcoreData.h"
#include "J2735_internal_DF_BSMcoreData_test.h"

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

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  uint8_t msg_cnt = J2735_BSM_CORE_DATA_GET_MSG_CNT(payload);
  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  int32_t lat = J2735_BSM_CORE_DATA_GET_LAT(payload);

  TEST_ASSERT_EQUAL_INT_MESSAGE(10, msg_cnt, "msgCnt should be 10");
  TEST_ASSERT_EQUAL_INT_MESSAGE(410123450, lat, "lat should be 410123450");
}

void run_testsuite_bsm_core_data(void) { RUN_TEST(test_bsm_core_data_fixed_data); }
