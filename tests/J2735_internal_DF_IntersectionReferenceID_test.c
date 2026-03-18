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
 * @brief Tests for IntersectionReferenceID non-extensible SEQUENCE.
 *
 * The data frame IntersectionReferenceID is a simple case with an optional fields.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DF_IntersectionReferenceID.h"
#include "J2735_internal_DF_IntersectionReferenceID_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test IntersectionReferenceID with OPTIONAL field ABSENT.
 *
 * @par ASN.1 Definition:
 * @code
 * IntersectionReferenceID ::= SEQUENCE {
 *   region  RoadRegulatorID OPTIONAL,  -- 16 bits (unsigned)
 *   id      IntersectionID             -- 16 bits (unsigned)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - region: ABSENT
 * - id:     0x1234 (4660)
 *
 * @par Wire Format (17 bits total):
 * | Offset (bits) | Width | Field    | Value            |
 * |---------------|-------|----------|------------------|
 * | 0             | 1     | preamble | 0 (region absent)|
 * | 1             | 16    | id       | 0001001000110100 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x09 | 00001001 | preamble(0) + id[15:9]|
 * | 1    | 0x1A | 00011010 | id[8:1]               |
 * | 2    | 0x00 | 00000000 | id[0] + padding       |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_intersection_reference_id_optional_field_absent(void) {
  static const uint8_t payload[] = {
      0x09,                                          /* preamble(0) + id[15:9] */
      0x1A,                                          /* id[8:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool region_present = J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(payload);
  TEST_ASSERT_FALSE_MESSAGE(region_present, "Region field should be absent");

  uint16_t intersection_id = J2735_INTERSECTION_REFERENCE_ID_GET_ID(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x1234U, intersection_id, "id should be 0x1234");
}

/**
 * @brief Test IntersectionReferenceID with OPTIONAL field PRESENT.
 *
 * @par ASN.1 Definition:
 * @code
 * IntersectionReferenceID ::= SEQUENCE {
 *   region  RoadRegulatorID OPTIONAL,  -- 16 bits (unsigned)
 *   id      IntersectionID             -- 16 bits (unsigned)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - region: PRESENT, 0x00FF (255)
 * - id:     0x1234 (4660)
 *
 * @par Wire Format (33 bits total):
 * | Offset (bits) | Width | Field    | Value             |
 * |---------------|-------|----------|-------------------|
 * | 0             | 1     | preamble | 1 (region present)|
 * | 1             | 16    | region   | 0000000011111111  |
 * | 17            | 16    | id       | 0001001000110100  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x80 | 10000000 | preamble(1) + region[15:9]|
 * | 1    | 0x7F | 01111111 | region[8:2] + region[1]   |
 * | 2    | 0x89 | 10001001 | region[0] + id[15:9]      |
 * | 3    | 0x1A | 00011010 | id[8:1]                   |
 * | 4    | 0x00 | 00000000 | id[0] + padding           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_intersection_reference_id_optional_field_present(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* preamble(1) + region[15:9] */
      0x7F,                                          /* region[8:2] + region[1] */
      0x89,                                          /* region[0] + id[15:9] */
      0x1A,                                          /* id[8:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool region_present = J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(payload);
  TEST_ASSERT_TRUE_MESSAGE(region_present, "Region field should be present");

  uint16_t region = J2735_INTERSECTION_REFERENCE_ID_GET_REGION(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x00FFU, region, "region should be 0x00FF");

  uint16_t intersection_id = J2735_INTERSECTION_REFERENCE_ID_GET_ID(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x1234U, intersection_id, "id should be 0x1234");
}

/**
 * @brief Test IntersectionReferenceID with boundary minimum values (all zeros).
 *
 * @par ASN.1 Definition:
 * @code
 * IntersectionReferenceID ::= SEQUENCE {
 *   region  RoadRegulatorID OPTIONAL,  -- 16 bits (unsigned, 0..65535)
 *   id      IntersectionID             -- 16 bits (unsigned, 0..65535)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - region: PRESENT, 0 (minimum)
 * - id:     0 (minimum)
 *
 * @par Wire Format (33 bits total):
 * | Offset (bits) | Width | Field    | Value             |
 * |---------------|-------|----------|-------------------|
 * | 0             | 1     | preamble | 1 (region present)|
 * | 1             | 16    | region   | 0000000000000000  |
 * | 17            | 16    | id       | 0000000000000000  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0x80 | 10000000 | preamble(1) + region[15:9]|
 * | 1    | 0x00 | 00000000 | region[8:1]               |
 * | 2    | 0x00 | 00000000 | region[0] + id[15:9]      |
 * | 3    | 0x00 | 00000000 | id[8:1]                   |
 * | 4    | 0x00 | 00000000 | id[0] + padding           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_intersection_reference_id_boundary_min(void) {
  static const uint8_t payload[] = {
      0x80,                                          /* preamble(1) + region[15:9] = 0 */
      0x00,                                          /* region[8:1] */
      0x00,                                          /* region[0] + id[15:9] */
      0x00,                                          /* id[8:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool region_present = J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(payload);
  TEST_ASSERT_TRUE_MESSAGE(region_present, "Region field should be present");

  uint16_t region = J2735_INTERSECTION_REFERENCE_ID_GET_REGION(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0U, region, "region should be 0 (minimum)");

  uint16_t intersection_id = J2735_INTERSECTION_REFERENCE_ID_GET_ID(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0U, intersection_id, "id should be 0 (minimum)");
}

/**
 * @brief Test IntersectionReferenceID with boundary maximum values (all ones).
 *
 * @par ASN.1 Definition:
 * @code
 * IntersectionReferenceID ::= SEQUENCE {
 *   region  RoadRegulatorID OPTIONAL,  -- 16 bits (unsigned, 0..65535)
 *   id      IntersectionID             -- 16 bits (unsigned, 0..65535)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - region: PRESENT, 65535 (0xFFFF, maximum)
 * - id:     65535 (0xFFFF, maximum)
 *
 * @par Wire Format (33 bits total):
 * | Offset (bits) | Width | Field    | Value             |
 * |---------------|-------|----------|-------------------|
 * | 0             | 1     | preamble | 1 (region present)|
 * | 1             | 16    | region   | 1111111111111111  |
 * | 17            | 16    | id       | 1111111111111111  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                    |
 * |------|------|----------|---------------------------|
 * | 0    | 0xFF | 11111111 | preamble(1) + region[15:9]|
 * | 1    | 0xFF | 11111111 | region[8:1]               |
 * | 2    | 0xFF | 11111111 | region[0] + id[15:9]      |
 * | 3    | 0xFF | 11111111 | id[8:1]                   |
 * | 4    | 0x80 | 10000000 | id[0] + padding           |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_intersection_reference_id_boundary_max(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* preamble(1) + region[15:9] = 0x7F */
      0xFF,                                          /* region[8:1] */
      0xFF,                                          /* region[0] + id[15:9] */
      0xFF,                                          /* id[8:1] */
      0x80,                                          /* id[0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool region_present = J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(payload);
  TEST_ASSERT_TRUE_MESSAGE(region_present, "Region field should be present");

  uint16_t region = J2735_INTERSECTION_REFERENCE_ID_GET_REGION(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xFFFFU, region, "region should be 0xFFFF (maximum)");

  uint16_t intersection_id = J2735_INTERSECTION_REFERENCE_ID_GET_ID(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xFFFFU, intersection_id, "id should be 0xFFFF (maximum)");
}

/**
 * @brief Test IntersectionReferenceID with id at maximum when region absent.
 *
 * @par ASN.1 Definition:
 * @code
 * IntersectionReferenceID ::= SEQUENCE {
 *   region  RoadRegulatorID OPTIONAL,  -- 16 bits (unsigned, 0..65535)
 *   id      IntersectionID             -- 16 bits (unsigned, 0..65535)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - region: ABSENT
 * - id:     65535 (0xFFFF, maximum)
 *
 * @par Wire Format (17 bits total):
 * | Offset (bits) | Width | Field    | Value            |
 * |---------------|-------|----------|------------------|
 * | 0             | 1     | preamble | 0 (region absent)|
 * | 1             | 16    | id       | 1111111111111111 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x7F | 01111111 | preamble(0) + id[15:9]|
 * | 1    | 0xFF | 11111111 | id[8:1]               |
 * | 2    | 0x80 | 10000000 | id[0] + padding       |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_intersection_reference_id_absent_region_max_id(void) {
  static const uint8_t payload[] = {
      0x7F,                                          /* preamble(0) + id[15:9] */
      0xFF,                                          /* id[8:1] */
      0x80,                                          /* id[0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool region_present = J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(payload);
  TEST_ASSERT_FALSE_MESSAGE(region_present, "Region field should be absent");

  uint16_t intersection_id = J2735_INTERSECTION_REFERENCE_ID_GET_ID(payload);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xFFFFU, intersection_id, "id should be 0xFFFF (maximum)");
}

/**
 * @brief Test IntersectionReferenceID with misaligned buffer access.
 *
 * Since this is an embedded library, we must verify correct operation when
 * the buffer is not aligned to a natural boundary. This tests the packed-cast
 * optimization used by J2735_READ_BITS.
 *
 * Uses the same test vector as test_intersection_reference_id_optional_field_present
 * but with a 1-byte offset to force misalignment.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_intersection_reference_id_misaligned_access(void) {
  static const uint8_t payload[] = {
      0xFF,                                          /* padding byte to force misalignment */
      0x80,                                          /* preamble(1) + region[15:9] */
      0x7F,                                          /* region[8:2] + region[1] */
      0x89,                                          /* region[0] + id[15:9] */
      0x1A,                                          /* id[8:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Offset pointer by 1 byte to force misalignment */
  const uint8_t *unaligned_ptr = &payload[1];

  bool region_present = J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(unaligned_ptr);
  TEST_ASSERT_TRUE_MESSAGE(region_present, "Region should be present (misaligned)");

  uint16_t region = J2735_INTERSECTION_REFERENCE_ID_GET_REGION(unaligned_ptr);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x00FFU, region, "region should be 0x00FF (misaligned)");

  uint16_t intersection_id = J2735_INTERSECTION_REFERENCE_ID_GET_ID(unaligned_ptr);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x1234U, intersection_id, "id should be 0x1234 (misaligned)");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

void run_testsuite_intersection_reference_id(void) {
  /* Happy path tests */
  RUN_TEST(test_intersection_reference_id_optional_field_absent);
  RUN_TEST(test_intersection_reference_id_optional_field_present);

  /* Boundary value tests */
  RUN_TEST(test_intersection_reference_id_boundary_min);
  RUN_TEST(test_intersection_reference_id_boundary_max);
  RUN_TEST(test_intersection_reference_id_absent_region_max_id);

  /* Misalignment tests */
  RUN_TEST(test_intersection_reference_id_misaligned_access);
}
