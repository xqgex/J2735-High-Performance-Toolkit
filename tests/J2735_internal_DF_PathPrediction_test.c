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
 * @brief Sanity tests for PathPrediction.
 *
 * The data frame PathPrediction is a simple case with an extension field.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DF_PathPrediction.h"
#include "J2735_internal_DF_PathPrediction_test.h"

/**
 * @brief Test PathPrediction with NO extension (extension bit = 0).
 *
 * @par ASN.1 Definition:
 * @code
 * PathPrediction ::= SEQUENCE {
 *   radiusOfCurve  RadiusOfCurvature,  -- 16 bits (signed, -32767..32767)
 *   confidence     Confidence,         --  8 bits (unsigned, 0..200)
 *   ...                                -- Extensible
 * }
 * @endcode
 *
 * @par Test Vector:
 * - extension:     ABSENT
 * - radiusOfCurve: 1000 (0x03E8)
 * - confidence:    100 (0x64)
 *
 * @par Wire Format (25 bits total):
 * | Offset (bits) | Width | Field         | Value            |
 * |---------------|-------|---------------|------------------|
 * | 0             | 1     | extension     | 0 (not extended) |
 * | 1             | 16    | radiusOfCurve | 0000001111101000 |
 * | 17            | 8     | confidence    | 01100100         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x01 | 00000001 | ext(0) + radiusOfCurve[15:9] |
 * | 1    | 0xF4 | 11110100 | radiusOfCurve[8:1]           |
 * | 2    | 0x32 | 00110010 | radiusOfCurve[0] + conf[7:1] |
 * | 3    | 0x00 | 00000000 | conf[0] + padding            |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_path_prediction_no_extension(void) {
  static const uint8_t payload[] = {
      0x01,                                          /* ext(0) + radiusOfCurve[15:9] */
      0xF4,                                          /* radiusOfCurve[8:1] */
      0x32,                                          /* radiusOfCurve[0] + conf[7:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  bool has_ext = J2735_PATH_PREDICTION_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Extension should be absent");

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  int16_t radius = J2735_PATH_PREDICTION_GET_RADIUS_OF_CURVE(payload);
  TEST_ASSERT_EQUAL_INT16_MESSAGE(1000, radius, "radiusOfCurve should be 1000");

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  uint8_t conf = J2735_PATH_PREDICTION_GET_CONFIDENCE(payload);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(100U, conf, "confidence should be 100");

  uint32_t size_bits = 0U;
  int result = j2735_inline_path_prediction_size(payload, &size_bits);
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "size calculation should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(25U, size_bits, "size should be 25 bits");
}

/**
 * @brief Test PathPrediction with extension bit set and valid extension data.
 *
 * @par ASN.1 Definition:
 * @code
 * PathPrediction ::= SEQUENCE {
 *   radiusOfCurve  RadiusOfCurvature,  -- 16 bits (signed, -32767..32767)
 *   confidence     Confidence,         --  8 bits (unsigned, 0..200)
 *   ...                                -- Extensible
 * }
 * @endcode
 *
 * @par Test Vector:
 * - extension:     PRESENT
 * - radiusOfCurve: 500 (0x01F4)
 * - confidence:    50 (0x32)
 * - Extension additions: 1 extension (0xAB content)
 *
 * @par Wire Format (49 bits total):
 * | Offset (bits) | Width | Field                | Value            |
 * |---------------|-------|----------------------|------------------|
 * | 0             | 1     | extension            | 1 (extended)     |
 * | 1             | 16    | radiusOfCurve        | 0000000111110100 |
 * | 17            | 8     | confidence           | 00110010         |
 * | 25            | 7     | nsnnwn (ext count-1) | 0000000 (=0)     |
 * | 32            | 1     | bitmap               | 1 (ext 0 present)|
 * | 33            | 8     | length det           | 00000001 (=1)    |
 * | 41            | 8     | ext content          | 10101011 (0xAB)  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                              |
 * |------|------|----------|-------------------------------------|
 * | 0    | 0x80 | 10000000 | ext(1) + radiusOfCurve[15:9]        |
 * | 1    | 0xFA | 11111010 | radiusOfCurve[8:1]                  |
 * | 2    | 0x19 | 00011001 | radiusOfCurve[0] + conf[7:1]        |
 * | 3    | 0x00 | 00000000 | conf[0] + nsnnwn[6:0]               |
 * | 4    | 0x80 | 10000000 | bitmap(1) + lenDet[7:1]             |
 * | 5    | 0xD5 | 11010101 | lenDet[0] + content[7:1]            |
 * | 6    | 0x80 | 10000000 | content[0] + padding                |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_path_prediction_with_extension(void) {
  static const uint8_t payload[] = {
      0x80,                               /* ext(1) + radiusOfCurve[15:9] */
      0xFA,                               /* radiusOfCurve[8:1] */
      0x19,                               /* radiusOfCurve[0] + conf[7:1] */
      0x00,                               /* conf[0] + nsnnwn[6:0] */
      0x80,                               /* bitmap(1) + lenDet[7:1] */
      0xD5,                               /* lenDet[0] + content[7:1] */
      0x80,                               /* content[0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8-byte alignment padding for J2735_READ_BITS */
  };

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  bool has_ext = J2735_PATH_PREDICTION_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_ext, "Extension should be present");

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  int16_t radius = J2735_PATH_PREDICTION_GET_RADIUS_OF_CURVE(payload);
  TEST_ASSERT_EQUAL_INT16_MESSAGE(500, radius, "radiusOfCurve should be 500");

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  uint8_t conf = J2735_PATH_PREDICTION_GET_CONFIDENCE(payload);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(50U, conf, "confidence should be 50");

  /* Exercise extension parsing: root(25) + nsnnwn(7) + bitmap(1) + len(8) + content(8) = 49 bits */
  uint32_t size_bits = 0U;
  int result = j2735_inline_path_prediction_size(payload, &size_bits);
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "size calculation should succeed");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(49U, size_bits, "size should be 49 bits with extension");
}

/**
 * @brief Test PathPrediction with negative radiusOfCurve (sign extension).
 *
 * @par ASN.1 Definition:
 * @code
 * PathPrediction ::= SEQUENCE {
 *   radiusOfCurve  RadiusOfCurvature,  -- 16 bits (signed, -32767..32767)
 *   confidence     Confidence,         --  8 bits (unsigned, 0..200)
 *   ...                                -- Extensible
 * }
 * @endcode
 *
 * @par Test Vector:
 * - extension:     ABSENT
 * - radiusOfCurve: -1000 (0xFC18 in two's complement)
 * - confidence:    150 (0x96)
 *
 * @par Wire Format (25 bits total):
 * | Offset (bits) | Width | Field         | Value            |
 * |---------------|-------|---------------|------------------|
 * | 0             | 1     | extension     | 0 (not extended) |
 * | 1             | 16    | radiusOfCurve | 1111110000011000 |
 * | 17            | 8     | confidence    | 10010110         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x7E | 01111110 | ext(0) + radiusOfCurve[15:9] |
 * | 1    | 0x0C | 00001100 | radiusOfCurve[8:1]           |
 * | 2    | 0x4B | 01001011 | radiusOfCurve[0] + conf[7:1] |
 * | 3    | 0x00 | 00000000 | conf[0] + padding            |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_path_prediction_signed_negative(void) {
  static const uint8_t payload[] = {
      0x7E,                                          /* ext(0) + radiusOfCurve[15:9] */
      0x0C,                                          /* radiusOfCurve[8:1] */
      0x4B,                                          /* radiusOfCurve[0] + conf[7:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  bool has_ext = J2735_PATH_PREDICTION_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_ext, "Extension should be absent");

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  int16_t radius = J2735_PATH_PREDICTION_GET_RADIUS_OF_CURVE(payload);
  TEST_ASSERT_EQUAL_INT16_MESSAGE(-1000, radius, "radiusOfCurve should be -1000");

  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy macro uses packed-cast */
  uint8_t conf = J2735_PATH_PREDICTION_GET_CONFIDENCE(payload);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(150U, conf, "confidence should be 150");
}

void run_testsuite_path_prediction(void) {
  RUN_TEST(test_path_prediction_no_extension);
  RUN_TEST(test_path_prediction_with_extension);
  RUN_TEST(test_path_prediction_signed_negative);
}
