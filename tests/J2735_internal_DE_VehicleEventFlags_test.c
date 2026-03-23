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
 * @brief Tests for VehicleEventFlags extensible BIT STRING.
 *
 * @par ASN.1 Type Under Test:
 * @code
 * VehicleEventFlags ::= BIT STRING {
 *     eventHazardLights (0),
 *     eventStopLineViolation (1),
 *     eventABSactivated (2),
 *     eventTractionControlLoss (3),
 *     eventStabilityControlactivated (4),
 *     eventHazardousMaterials (5),
 *     eventReserved1 (6),
 *     eventHardBraking (7),
 *     eventLightsChanged (8),
 *     eventWipersChanged (9),
 *     eventFlatTire (10),
 *     eventDisabledVehicle (11),
 *     eventAirBagDeployment (12),
 *     eventJackKnife (13)         -- Extension addition
 * } (SIZE (13, ..., 14))
 * @endcode
 *
 * @par Wire Format Summary:
 * - Non-extended form (14 bits): [ext=0][13 flag bits][2-bit padding]
 * - Extended form (22 bits): [ext=1][nsnnwn=7 bits][14 flag bits][2-bit padding]
 *
 * @par Bit Numbering Convention:
 * - ASN.1 bit 0 = leftmost/MSB of BIT STRING content (eventHazardLights)
 * - ASN.1 bit 12 = rightmost of root (eventAirBagDeployment)
 * - ASN.1 bit 13 = extension addition (eventJackKnife, extended form only)
 *
 * @par Byte Encoding Notation:
 * - flags[12:6] = flag bits 12 down to 6 (7 bits, Verilog-style bit-slice)
 * - flags[5:0] = flag bits 5 down to 0 (6 bits)
 * - Note: Bit indices are from the 13-bit field where bit 0 = MSB (hazardLights)
 */

#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_internal_DE_VehicleEventFlags.h"
#include "J2735_internal_DE_VehicleEventFlags_test.h"

/* cppcheck-suppress-begin misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Test VehicleEventFlags with non-extended (root) form.
 *
 * @par Test Vector:
 * - Extended: NO (root form)
 * - Flags: 0x1234 masked to 13 bits = 0x1234 & 0x1FFF = 0x1234 (bits 0-12 set)
 *
 * @par Wire Format (14 bits total):
 * | Offset (bits) | Width | Field        | Value          |
 * |---------------|-------|--------------|----------------|
 * | 0             | 1     | ext_bit      | 0              |
 * | 1             | 13    | flags        | 1001000110100  |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x48 | 01001000 | ext(0) + flags[12:6]       |
 * | 1    | 0xD0 | 11010000 | flags[5:0] + padding(2)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended(void) {
  /* Flags = 0x1234 = 0001 0010 0011 0100 (but we only use 13 bits)
   * 13-bit value = 0x1234 & 0x1FFF = 0x1234
   * Binary: 1 0010 0011 0100 = 1001000110100
   *
   * Wire: [ext=0][13 bits: 1001000110100]
   * = 0 1001 0001 1010 0xxx = 0x48 0xD0 (with padding)
   */
  static const uint8_t payload[] = {
      0x48,                                          /* ext(0) + flags[12:6] = 1001000 */
      0xD0,                                          /* flags[5:0] + padding = 11010000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Check extension bit */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_extension, "Extension bit should be 0 for non-extended form");

  /* Check flags value using unified getter */
  uint16_t const flags = J2735_VEHICLE_EVENT_FLAGS_GET(payload);
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x1234U, flags,
                                  "Flags value should match expected 0x1234 for non-extended form");

  /* Check size using dynamic size macro */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(14U, J2735_VEHICLE_EVENT_FLAGS_SIZE(payload),
                                   "Size should be 14 for non-extended form");
}

/**
 * @brief Test VehicleEventFlags with extended form.
 *
 * @par Test Vector:
 * - Extended: YES
 * - nsnnwn value: 14 (small form: 0 + 6 bits = 0b0001110 = 14)
 * - Flags: 0x3FFF (all 14 bits set)
 *
 * @par Wire Format (22 bits total):
 * | Offset (bits) | Width | Field        | Value                    |
 * |---------------|-------|--------------|--------------------------|
 * | 0             | 1     | ext_bit      | 1                        |
 * | 1             | 7     | nsnnwn       | 0001110 (small form: 14) |
 * | 8             | 14    | flags        | 11111111111111           |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)     |
 * | 1    | 0xFF | 11111111 | flags[13:6]                  |
 * | 2    | 0xFC | 11111100 | flags[5:0] + padding(2)      |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_extended(void) {
  /* nsnnwn small form for 14: 0 + 001110 = 0b0001110 = 0x0E
   * Wire: [ext=1][0001110][11 1111 1111 1111]
   * = 1 0001110 11111111 111111xx
   * = 0x8E 0xFF 0xFC
   */
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn = 10001110 */
      0xFF,                                          /* flags[13:6] */
      0xFC,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Check extension bit */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_extension, "Extension bit should be 1 for extended form");

  /* Check flags value using unified getter */
  uint16_t const flags = J2735_VEHICLE_EVENT_FLAGS_GET(payload);
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x3FFFU, flags,
                                  "Flags value should match expected 0x3FFF for extended form");

  /* Check size using dynamic size macro */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(22U, J2735_VEHICLE_EVENT_FLAGS_SIZE(payload),
                                   "Size should be 22 for extended form");
}

/**
 * @brief Test individual flag accessors (bits 0-4) with non-extended form.
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Extended: NO (root form)
 * - Flags: 0x1234 = binary 1 0010 0011 0100 (13 bits)
 * - Derivation: Each bit position maps to ASN.1 named bit
 *   - bit 0  (0x1000) = 1 → eventHazardLights = ON
 *   - bit 1  (0x0800) = 0 → eventStopLineViolation = OFF
 *   - bit 2  (0x0400) = 0 → eventABSactivated = OFF
 *   - bit 3  (0x0200) = 1 → eventTractionControlLoss = ON
 *   - bit 4  (0x0100) = 0 → eventStabilityControlactivated = OFF
 *
 * @par Wire Format (14 bits total):
 * | Offset (bits) | Width | Field    | Value         |
 * |---------------|-------|----------|---------------|
 * | 0             | 1     | ext_bit  | 0             |
 * | 1             | 13    | flags    | 1001000110100 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                 |
 * |------|------|----------|------------------------|
 * | 0    | 0x48 | 01001000 | ext(0) + flags[12:6]   |
 * | 1    | 0xD0 | 11010000 | flags[5:0] + pad(2)    |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_individual_non_extended_flags_0_to_4(void) {
  static const uint8_t payload[] = {
      0x48,                                          /* ext(0) + flags[12:6] */
      0xD0,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify individual flags match expected pattern for 0x1234 */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "bit 1: eventStopLineViolation should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_ABS_ACTIVATED(payload),
                                  "bit 2: eventABSactivated should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_TRACTION_CONTROL_LOSS(payload),
      "bit 3: eventTractionControlLoss should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STABILITY_CONTROLACTIVATED(payload),
      "bit 4: eventStabilityControlactivated should be OFF");
}

/**
 * @brief Test individual flag accessors (bits 5-9) with non-extended form.
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x1234 = binary 1 0010 0011 0100 (13 bits)
 * - Derivation:
 *   - bit 5  (0x0080) = 0 → eventHazardousMaterials = OFF
 *   - bit 6  (0x0040) = 0 → eventReserved1 = OFF
 *   - bit 7  (0x0020) = 1 → eventHardBraking = ON
 *   - bit 8  (0x0010) = 1 → eventLightsChanged = ON
 *   - bit 9  (0x0008) = 0 → eventWipersChanged = OFF
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                 |
 * |------|------|----------|------------------------|
 * | 0    | 0x48 | 01001000 | ext(0) + flags[12:6]   |
 * | 1    | 0xD0 | 11010000 | flags[5:0] + pad(2)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_individual_non_extended_flags_5_to_9(void) {
  static const uint8_t payload[] = {
      0x48,                                          /* ext(0) + flags[12:6] */
      0xD0,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify individual flags match expected pattern for 0x1234 */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARDOUS_MATERIALS(payload),
                                  "bit 5: eventHazardousMaterials should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_RESERVED_1(payload),
                                  "bit 6: eventReserved1 should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "bit 7: eventHardBraking should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_LIGHTS_CHANGED(payload),
                                  "bit 8: eventLightsChanged should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_WIPERS_CHANGED(payload),
                                  "bit 9: eventWipersChanged should be OFF");
}

/**
 * @brief Test individual flag accessors (bits 10-13) with non-extended form.
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x1234 = binary 1 0010 0011 0100 (13 bits)
 * - Derivation:
 *   - bit 10 (0x0004) = 1 → eventFlatTire = ON
 *   - bit 11 (0x0002) = 0 → eventDisabledVehicle = OFF
 *   - bit 12 (0x0001) = 0 → eventAirBagDeployment = OFF
 *   - bit 13 (N/A)    = 0 → eventJackKnife = OFF (non-extended form, bit does not exist)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                 |
 * |------|------|----------|------------------------|
 * | 0    | 0x48 | 01001000 | ext(0) + flags[12:6]   |
 * | 1    | 0xD0 | 11010000 | flags[5:0] + pad(2)    |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_individual_non_extended_flags_10_to_13(void) {
  static const uint8_t payload[] = {
      0x48,                                          /* ext(0) + flags[12:6] */
      0xD0,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify individual flags match expected pattern for 0x1234 */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_FLAT_TIRE(payload),
                                  "bit 10: eventFlatTire should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_DISABLED_VEHICLE(payload),
                                  "bit 11: eventDisabledVehicle should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "bit 12: eventAirBagDeployment should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(payload),
                                  "bit 13: eventJackKnife should be OFF (non-extended)");
}

/**
 * @brief Test individual flag accessors (bits 0-4) with extended form, all ON.
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Extended: YES
 * - nsnnwn: 14 (small form: 0 + 6 bits = 0b0001110)
 * - Flags: 0x3FFF = binary 11 1111 1111 1111 (14 bits, all ON)
 * - Derivation: All 14 flags should be ON
 *   - bit 0 = 1 → eventHazardLights = ON
 *   - bit 1 = 1 → eventStopLineViolation = ON
 *   - bit 2 = 1 → eventABSactivated = ON
 *   - bit 3 = 1 → eventTractionControlLoss = ON
 *   - bit 4 = 1 → eventStabilityControlactivated = ON
 *
 * @par Wire Format (22 bits total):
 * | Offset (bits) | Width | Field    | Value                  |
 * |---------------|-------|----------|------------------------|
 * | 0             | 1     | ext_bit  | 1                      |
 * | 1             | 7     | nsnnwn   | 0001110 (=14)          |
 * | 8             | 14    | flags    | 11111111111111         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)    |
 * | 1    | 0xFF | 11111111 | flags[13:6]                 |
 * | 2    | 0xFC | 11111100 | flags[5:0] + pad(2)         |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_individual_extended_flags_0_to_4(void) {
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn = 10001110 */
      0xFF,                                          /* flags[13:6] */
      0xFC,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "bit 1: eventStopLineViolation should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_ABS_ACTIVATED(payload),
                                  "bit 2: eventABSactivated should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_TRACTION_CONTROL_LOSS(payload),
      "bit 3: eventTractionControlLoss should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STABILITY_CONTROLACTIVATED(payload),
      "bit 4: eventStabilityControlactivated should be ON");
}

/**
 * @brief Test individual flag accessors (bits 5-9) with extended form, all ON.
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x3FFF (all 14 bits ON)
 * - Derivation:
 *   - bit 5 = 1 → eventHazardousMaterials = ON
 *   - bit 6 = 1 → eventReserved1 = ON
 *   - bit 7 = 1 → eventHardBraking = ON
 *   - bit 8 = 1 → eventLightsChanged = ON
 *   - bit 9 = 1 → eventWipersChanged = ON
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)    |
 * | 1    | 0xFF | 11111111 | flags[13:6]                 |
 * | 2    | 0xFC | 11111100 | flags[5:0] + pad(2)         |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_individual_extended_flags_5_to_9(void) {
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn = 10001110 */
      0xFF,                                          /* flags[13:6] */
      0xFC,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARDOUS_MATERIALS(payload),
                                  "bit 5: eventHazardousMaterials should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_RESERVED_1(payload),
                                  "bit 6: eventReserved1 should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "bit 7: eventHardBraking should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_LIGHTS_CHANGED(payload),
                                  "bit 8: eventLightsChanged should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_WIPERS_CHANGED(payload),
                                  "bit 9: eventWipersChanged should be ON");
}

/**
 * @brief Test individual flag accessors (bits 10-13) with extended form, all ON.
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x3FFF (all 14 bits ON)
 * - Derivation:
 *   - bit 10 = 1 → eventFlatTire = ON
 *   - bit 11 = 1 → eventDisabledVehicle = ON
 *   - bit 12 = 1 → eventAirBagDeployment = ON
 *   - bit 13 = 1 → eventJackKnife = ON (extended-only flag)
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                      |
 * |------|------|----------|-----------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)    |
 * | 1    | 0xFF | 11111111 | flags[13:6]                 |
 * | 2    | 0xFC | 11111100 | flags[5:0] + pad(2)         |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_individual_extended_flags_10_to_13(void) {
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn = 10001110 */
      0xFF,                                          /* flags[13:6] */
      0xFC,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_FLAT_TIRE(payload),
                                  "bit 10: eventFlatTire should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_DISABLED_VEHICLE(payload),
                                  "bit 11: eventDisabledVehicle should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "bit 12: eventAirBagDeployment should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(payload),
                                  "bit 13: eventJackKnife should be ON (extended)");
}

/**
 * @brief Test J2735_VEHICLE_EVENT_FLAGS_SIZE for non-extended form.
 *
 * Non-extended wire format: [ext=0][13 flag bits] = 14 bits total.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_size_non_extended(void) {
  /* Same payload as test_vehicle_event_flags_non_extended: flags = 0x1234 */
  static const uint8_t payload[] = {
      0x48,                                          /* ext(0) + flags[12:6] */
      0xD0,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Non-extended form: 1 bit (ext) + 13 bits (flags) = 14 bits */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(14U, J2735_VEHICLE_EVENT_FLAGS_SIZE(payload),
                                   "Non-extended form should consume 14 bits");
}

/**
 * @brief Test J2735_VEHICLE_EVENT_FLAGS_SIZE for extended form.
 *
 * Extended wire format: [ext=1][nsnnwn=7 bits][14 flag bits] = 22 bits total.
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_size_extended(void) {
  /* Same payload as test_vehicle_event_flags_extended: flags = 0x3FFF */
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn = 10001110 */
      0xFF,                                          /* flags[13:6] */
      0xFC,                                          /* flags[5:0] + padding */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Extended form: 1 bit (ext) + 7 bits (nsnnwn) + 14 bits (flags) = 22 bits */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(22U, J2735_VEHICLE_EVENT_FLAGS_SIZE(payload),
                                   "Extended form should consume 22 bits");
}

/**
 * @brief Test VehicleEventFlags with all zeros (non-extended).
 *
 * Edge case: All 13 flag bits are zero.
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field    | Value         |
 * |--------|-------|----------|---------------|
 * | 0      | 1     | ext_bit  | 0             |
 * | 1      | 13    | flags    | 0000000000000 |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                 |
 * |------|------|----------|------------------------|
 * | 0    | 0x00 | 00000000 | ext(0) + flags[12:5]   |
 * | 1    | 0x00 | 00000000 | flags[4:0] + padding   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_all_zeros_non_extended(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* ext(0) + flags[12:5] = 00000000 */
      0x00,                                          /* flags[4:0] + padding = 00000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify extension bit */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_extension, "Extension bit should be 0");

  /* Verify flags value is zero */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0000U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "All flags should be zero");

  /* Verify all individual flags are OFF */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "eventHazardLights should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "eventHardBraking should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "eventAirBagDeployment should be OFF");
}

/**
 * @brief Test VehicleEventFlags extended form with only jackKnife flag set.
 *
 * Edge case: Only the extension-only flag (bit 13) is set.
 * This validates that extended form is required for this flag.
 *
 * @par Test Vector:
 * - Extended: YES
 * - nsnnwn: 14 (small form)
 * - Flags: 0x0001 (only bit 13 = jackKnife)
 *
 * @par Wire Format (22 bits):
 * | Offset | Width | Field    | Value                  |
 * |--------|-------|----------|------------------------|
 * | 0      | 1     | ext_bit  | 1                      |
 * | 1      | 7     | nsnnwn   | 0001110 (14)           |
 * | 8      | 14    | flags    | 00000000000001         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)   |
 * | 1    | 0x00 | 00000000 | flags[13:6]                |
 * | 2    | 0x04 | 00000100 | flags[5:0] + padding       |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_extended_single_jackknife(void) {
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn(0001110) */
      0x00,                                          /* flags[13:6] = 00000000 */
      0x04,                                          /* flags[5:0] + padding = 000001|00 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify extension bit */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_extension, "Extension bit should be 1");

  /* Verify flags value */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0001U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Only jackKnife bit (bit 13) should be set");

  /* Verify only jackKnife is ON, all others OFF */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "eventHazardLights should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "eventHardBraking should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "eventAirBagDeployment should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(payload),
                                  "eventJackKnife should be ON");
}

/**
 * @brief Test non-extended form with all 13 root flags ON (max value 0x1FFF).
 *
 * Boundary test: Maximum valid value for non-extended form.
 * Validates that all 13 bits can be set without triggering extension.
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field   | Value           |
 * |--------|-------|---------|-----------------|
 * | 0      | 1     | ext_bit | 0               |
 * | 1      | 13    | flags   | 1111111111111   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                   |
 * |------|------|----------|--------------------------|
 * | 0    | 0x7F | 01111111 | ext(0) + flags[12:6]=1111111 |
 * | 1    | 0xFC | 11111100 | flags[5:0]=111111 + pad(2)  |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_all_root_flags_on_metadata(void) {
  static const uint8_t payload[] = {
      0x7F,                                          /* ext(0) + flags[12:6] = 01111111 */
      0xFC,                                          /* flags[5:0] + padding = 11111100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify non-extended */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_extension, "Extension bit should be 0");

  /* Verify max 13-bit value */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x1FFFU, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "All 13 root flags should be ON (0x1FFF)");

  /* Verify size */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(14U, J2735_VEHICLE_EVENT_FLAGS_SIZE(payload),
                                   "Non-extended form should be 14 bits");
}

/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_all_root_flags_on_bits(void) {
  static const uint8_t payload[] = {
      0x7F,                                          /* ext(0) + flags[12:6] = 01111111 */
      0xFC,                                          /* flags[5:0] + padding = 11111100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify all root flags are ON */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "eventStopLineViolation should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_ABS_ACTIVATED(payload),
                                  "eventABSactivated should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "eventAirBagDeployment should be ON");
  /* Bit 13 should be OFF (non-extended form) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(payload),
                                  "eventJackKnife should be OFF (non-extended)");
}

/**
 * @brief Test extended form with all flags OFF (only extension overhead).
 *
 * Edge case: Extended form but zero flag value.
 * Validates that extended form parsing works even with no flags set.
 *
 * @par Wire Format (22 bits):
 * | Offset | Width | Field   | Value                  |
 * |--------|-------|---------|------------------------|
 * | 0      | 1     | ext_bit | 1                      |
 * | 1      | 7     | nsnnwn  | 0001110 (14)           |
 * | 8      | 14    | flags   | 00000000000000         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                     |
 * |------|------|----------|----------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)   |
 * | 1    | 0x00 | 00000000 | flags[13:6]                |
 * | 2    | 0x00 | 00000000 | flags[5:0] + padding       |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_extended_all_zeros(void) {
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn(0001110) */
      0x00,                                          /* flags[13:6] = 00000000 */
      0x00,                                          /* flags[5:0] + padding = 00000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify extended */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_extension, "Extension bit should be 1");

  /* Verify zero flags */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0000U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "All 14 flags should be zero");

  /* Verify size */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(22U, J2735_VEHICLE_EVENT_FLAGS_SIZE(payload),
                                   "Extended form should be 22 bits");

  /* Verify all individual flags are OFF */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "eventHazardLights should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(payload),
                                  "eventJackKnife should be OFF");
}

/**
 * @brief Test non-extended with alternating bits pattern 0x1555.
 *
 * Pattern test: Alternating 1/0 starting with 1 (0x1555 = 1010101010101).
 * Catches bit alignment and nibble boundary issues.
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field   | Value           |
 * |--------|-------|---------|-----------------|
 * | 0      | 1     | ext_bit | 0               |
 * | 1      | 13    | flags   | 1010101010101   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0) + flags[12:6]=1010101 |
 * | 1    | 0x54 | 01010100 | flags[5:0]=010101 + pad(2)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x1555_metadata(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* ext(0) + flags[12:6] = 01010101 */
      0x54,                                          /* flags[5:0] + padding = 01010100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify non-extended */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_extension, "Extension bit should be 0");

  /* Verify alternating pattern */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x1555U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Flags should be alternating pattern 0x1555");
}

/**
 * @brief Test alternating 0x1555 pattern (bits 0-4).
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 * This part tests bits 0-4 of the 0x1555 alternating pattern.
 *
 * @par Test Vector:
 * - Flags: 0x1555 = binary 1 0101 0101 0101 (13 bits)
 * - Pattern: Alternating 1/0 starting with 1 (even positions ON)
 * - Derivation:
 *   - bit 0  = 1 → eventHazardLights = ON
 *   - bit 1  = 0 → eventStopLineViolation = OFF
 *   - bit 2  = 1 → eventABSactivated = ON
 *   - bit 3  = 0 → eventTractionControlLoss = OFF
 *   - bit 4  = 1 → eventStabilityControlactivated = ON
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0) + flags[12:6]=1010101 |
 * | 1    | 0x54 | 01010100 | flags[5:0]=010101 + pad(2)   |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x1555_0_to_4(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* ext(0) + flags[12:6] = 01010101 */
      0x54,                                          /* flags[5:0] + padding = 01010100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify alternating flags: ON at even positions (0,2,4,6,8,10,12) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "bit 1: eventStopLineViolation should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_ABS_ACTIVATED(payload),
                                  "bit 2: eventABSactivated should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_TRACTION_CONTROL_LOSS(payload),
      "bit 3: eventTractionControlLoss should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STABILITY_CONTROLACTIVATED(payload),
      "bit 4: eventStabilityControlactivated should be ON");
}

/**
 * @brief Test alternating 0x1555 pattern (bits 5-9).
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x1555 = binary 1 0101 0101 0101 (13 bits)
 * - Derivation:
 *   - bit 5  = 0 → eventHazardousMaterials = OFF
 *   - bit 6  = 1 → eventReserved1 = ON
 *   - bit 7  = 0 → eventHardBraking = OFF
 *   - bit 8  = 1 → eventLightsChanged = ON
 *   - bit 9  = 0 → eventWipersChanged = OFF
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0) + flags[12:6]=1010101 |
 * | 1    | 0x54 | 01010100 | flags[5:0]=010101 + pad(2)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x1555_5_to_9(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* ext(0) + flags[12:6] = 01010101 */
      0x54,                                          /* flags[5:0] + padding = 01010100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify alternating flags: ON at even positions (0,2,4,6,8,10,12) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARDOUS_MATERIALS(payload),
                                  "bit 5: eventHazardousMaterials should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_RESERVED_1(payload),
                                  "bit 6: eventReserved1 should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "bit 7: eventHardBraking should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_LIGHTS_CHANGED(payload),
                                  "bit 8: eventLightsChanged should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_WIPERS_CHANGED(payload),
                                  "bit 9: eventWipersChanged should be OFF");
}

/**
 * @brief Test alternating 0x1555 pattern (bits 10-12).
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x1555 = binary 1 0101 0101 0101 (13 bits)
 * - Derivation:
 *   - bit 10 = 1 → eventFlatTire = ON
 *   - bit 11 = 0 → eventDisabledVehicle = OFF
 *   - bit 12 = 1 → eventAirBagDeployment = ON
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x55 | 01010101 | ext(0) + flags[12:6]=1010101 |
 * | 1    | 0x54 | 01010100 | flags[5:0]=010101 + pad(2)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x1555_10_to_12(void) {
  static const uint8_t payload[] = {
      0x55,                                          /* ext(0) + flags[12:6] = 01010101 */
      0x54,                                          /* flags[5:0] + padding = 01010100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify alternating flags: ON at even positions (0,2,4,6,8,10,12) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_FLAT_TIRE(payload),
                                  "bit 10: eventFlatTire should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_DISABLED_VEHICLE(payload),
                                  "bit 11: eventDisabledVehicle should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "bit 12: eventAirBagDeployment should be ON");
}

/**
 * @brief Test non-extended with inverse alternating pattern 0x0AAA.
 *
 * Pattern test: Alternating 0/1 starting with 0 (0x0AAA = 0101010101010).
 * Catches bit alignment and nibble boundary issues (opposite of 0x1555).
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field   | Value           |
 * |--------|-------|---------|-----------------|
 * | 0      | 1     | ext_bit | 0               |
 * | 1      | 13    | flags   | 0101010101010   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x2A | 00101010 | ext(0) + flags[12:6]=0101010 |
 * | 1    | 0xA8 | 10101000 | flags[5:0]=101010 + pad(2)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_metadata(void) {
  static const uint8_t payload[] = {
      0x2A,                                          /* ext(0) + flags[12:6] = 00101010 */
      0xA8,                                          /* flags[5:0] + padding = 10101000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify non-extended */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_FALSE_MESSAGE(has_extension, "Extension bit should be 0");

  /* Verify alternating pattern */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0AAAU, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Flags should be alternating pattern 0x0AAA");
}

/**
 * @brief Test alternating 0x0AAA pattern (bits 0-4).
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 * This part tests bits 0-4 of the 0x0AAA alternating pattern (inverse of 0x1555).
 *
 * @par Test Vector:
 * - Flags: 0x0AAA = binary 0 1010 1010 1010 (13 bits)
 * - Pattern: Alternating 0/1 starting with 0 (odd positions ON)
 * - Derivation:
 *   - bit 0  = 0 → eventHazardLights = OFF
 *   - bit 1  = 1 → eventStopLineViolation = ON
 *   - bit 2  = 0 → eventABSactivated = OFF
 *   - bit 3  = 1 → eventTractionControlLoss = ON
 *   - bit 4  = 0 → eventStabilityControlactivated = OFF
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x2A | 00101010 | ext(0) + flags[12:6]=0101010 |
 * | 1    | 0xA8 | 10101000 | flags[5:0]=101010 + pad(2)   |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_0_to_4(void) {
  static const uint8_t payload[] = {
      0x2A,                                          /* ext(0) + flags[12:6] = 00101010 */
      0xA8,                                          /* flags[5:0] + padding = 10101000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify alternating flags: ON at odd positions (1,3,5,7,9,11) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "bit 1: eventStopLineViolation should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_ABS_ACTIVATED(payload),
                                  "bit 2: eventABSactivated should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_TRACTION_CONTROL_LOSS(payload),
      "bit 3: eventTractionControlLoss should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STABILITY_CONTROLACTIVATED(payload),
      "bit 4: eventStabilityControlactivated should be OFF");
}

/**
 * @brief Test alternating 0x0AAA pattern (bits 5-9).
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x0AAA = binary 0 1010 1010 1010 (13 bits)
 * - Derivation:
 *   - bit 5  = 1 → eventHazardousMaterials = ON
 *   - bit 6  = 0 → eventReserved1 = OFF
 *   - bit 7  = 1 → eventHardBraking = ON
 *   - bit 8  = 0 → eventLightsChanged = OFF
 *   - bit 9  = 1 → eventWipersChanged = ON
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x2A | 00101010 | ext(0) + flags[12:6]=0101010 |
 * | 1    | 0xA8 | 10101000 | flags[5:0]=101010 + pad(2)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_5_to_9(void) {
  static const uint8_t payload[] = {
      0x2A,                                          /* ext(0) + flags[12:6] = 00101010 */
      0xA8,                                          /* flags[5:0] + padding = 10101000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify alternating flags: ON at odd positions (1,3,5,7,9,11) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARDOUS_MATERIALS(payload),
                                  "bit 5: eventHazardousMaterials should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_RESERVED_1(payload),
                                  "bit 6: eventReserved1 should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "bit 7: eventHardBraking should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_LIGHTS_CHANGED(payload),
                                  "bit 8: eventLightsChanged should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_WIPERS_CHANGED(payload),
                                  "bit 9: eventWipersChanged should be ON");
}

/**
 * @brief Test alternating 0x0AAA pattern (bits 10-12).
 *
 * Split into multiple functions to reduce cyclomatic complexity per MISRA.
 *
 * @par Test Vector:
 * - Flags: 0x0AAA = binary 0 1010 1010 1010 (13 bits)
 * - Derivation:
 *   - bit 10 = 0 → eventFlatTire = OFF
 *   - bit 11 = 1 → eventDisabledVehicle = ON
 *   - bit 12 = 0 → eventAirBagDeployment = OFF
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x2A | 00101010 | ext(0) + flags[12:6]=0101010 |
 * | 1    | 0xA8 | 10101000 | flags[5:0]=101010 + pad(2)   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_10_to_12(void) {
  static const uint8_t payload[] = {
      0x2A,                                          /* ext(0) + flags[12:6] = 00101010 */
      0xA8,                                          /* flags[5:0] + padding = 10101000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  /* Verify alternating flags: ON at odd positions (1,3,5,7,9,11) */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_FLAT_TIRE(payload),
                                  "bit 10: eventFlatTire should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_DISABLED_VEHICLE(payload),
                                  "bit 11: eventDisabledVehicle should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "bit 12: eventAirBagDeployment should be OFF");
}

/**
 * @brief Test non-extended with only bit 0 (hazardLights) set.
 *
 * Single-bit isolation test for the first/highest bit.
 * Value = 0x1000 (only bit 0 = hazardLights ON).
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field   | Value           |
 * |--------|-------|---------|-----------------|
 * | 0      | 1     | ext_bit | 0               |
 * | 1      | 13    | flags   | 1000000000000   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x40 | 01000000 | ext(0) + flags[0:6]=1000000  |
 * | 1    | 0x00 | 00000000 | flags[7:12]=000000 + pad(2)  |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_single_bit_0_hazard_lights(void) {
  static const uint8_t payload[] = {
      0x40,                                          /* ext(0) + flags[12:5] = 01000000 */
      0x00,                                          /* flags[4:0] + padding = 00000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x1000U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Only bit 0 should be set (0x1000)");

  /* Verify only hazardLights is ON */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "bit 1: eventStopLineViolation should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "bit 12: eventAirBagDeployment should be OFF");
}

/**
 * @brief Test non-extended with only bit 12 (airBagDeployment) set.
 *
 * Single-bit isolation test for the last root bit.
 * Value = 0x0001 (only bit 12 = airBagDeployment ON).
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field   | Value           |
 * |--------|-------|---------|-----------------|
 * | 0      | 1     | ext_bit | 0               |
 * | 1      | 13    | flags   | 0000000000001   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x00 | 00000000 | ext(0) + flags[0:6]=0000000  |
 * | 1    | 0x04 | 00000100 | flags[7:12]=000001 + pad(2)  |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_single_bit_12_airbag(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* ext(0) + flags[12:6] = 00000000 */
      0x04,                                          /* flags[5:0] + padding = 00000100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0001U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Only bit 12 should be set (0x0001)");

  /* Verify only airBagDeployment is ON */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_DISABLED_VEHICLE(payload),
                                  "bit 11: eventDisabledVehicle should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(payload),
                                  "bit 12: eventAirBagDeployment should be ON");
}

/**
 * @brief Test non-extended with only bit 7 (hardBraking) set.
 *
 * Single-bit isolation test for a middle bit at byte boundary.
 * Value = 0x0020 (only bit 7 = hardBraking ON).
 *
 * @par Wire Format (14 bits):
 * | Offset | Width | Field   | Value           |
 * |--------|-------|---------|-----------------|
 * | 0      | 1     | ext_bit | 0               |
 * | 1      | 13    | flags   | 0000000100000   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x00 | 00000000 | ext(0) + flags[0:6]=0000000  |
 * | 1    | 0x80 | 10000000 | flags[7:12]=100000 + pad(2)  |
 *
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_single_bit_7_hard_braking(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* ext(0) + flags[12:6] = 00000000 */
      0x80,                                          /* flags[5:0] + padding = 10000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x0020U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Only bit 7 should be set (0x0020)");

  /* Verify only hardBraking is ON */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_RESERVED_1(payload),
                                  "bit 6: eventReserved1 should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(payload),
                                  "bit 7: eventHardBraking should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_LIGHTS_CHANGED(payload),
                                  "bit 8: eventLightsChanged should be OFF");
}

/**
 * @brief Test extended with only bit 0 (hazardLights) set.
 *
 * Extended form with single root flag - validates extended parsing
 * doesn't corrupt root flag positions.
 * Value = 0x2000 (only bit 0 = hazardLights ON in 14-bit form).
 *
 * @par Wire Format (22 bits):
 * | Offset | Width | Field   | Value                  |
 * |--------|-------|---------|------------------------|
 * | 0      | 1     | ext_bit | 1                      |
 * | 1      | 7     | nsnnwn  | 0001110 (14)           |
 * | 8      | 14    | flags   | 10000000000000         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                       |
 * |------|------|----------|------------------------------|
 * | 0    | 0x8E | 10001110 | ext(1) + nsnnwn(0001110)     |
 * | 1    | 0x80 | 10000000 | flags[13:6] = 10000000       |
 * | 2    | 0x00 | 00000000 | flags[5:0] + padding         |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_extended_single_hazard_lights(void) {
  static const uint8_t payload[] = {
      0x8E,                                          /* ext(1) + nsnnwn(0001110) */
      0x80,                                          /* flags[13:6] = 10000000 */
      0x00,                                          /* flags[5:0] + padding = 00000000 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };

  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(payload);
  TEST_ASSERT_TRUE_MESSAGE(has_extension, "Extension bit should be 1");

  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x2000U, J2735_VEHICLE_EVENT_FLAGS_GET(payload),
                                  "Only bit 0 should be set in 14-bit form (0x2000)");

  /* Verify only hazardLights is ON */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(payload),
                                  "bit 0: eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(payload),
                                  "bit 1: eventStopLineViolation should be OFF");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(payload),
                                  "bit 13: eventJackKnife should be OFF");
}

/**
 * @brief Test VehicleEventFlags with deliberately misaligned buffer pointer.
 *
 * @par Test Vector:
 * - Extended: NO (root form)
 * - Flags: 0x1FFF (all 13 root bits ON)
 *
 * @par Wire Format (14 bits total):
 * | Offset (bits) | Width | Field   | Value           |
 * |---------------|-------|---------|-----------------|
 * | 0             | 1     | ext_bit | 0               |
 * | 1             | 13    | flags   | 1111111111111   |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                         |
 * |------|------|----------|--------------------------------|
 * | 0    | 0x7F | 01111111 | ext(0) + flags[12:6]=1111111   |
 * | 1    | 0xFC | 11111100 | flags[5:0]=111111 + pad(2)     |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_vehicle_event_flags_misaligned_access(void) {
  static const uint8_t payload[] = {
      0x00,                                          /* junk byte for misalignment */
      0x7F,                                          /* ext(0) + flags[12:6] = 01111111 */
      0xFC,                                          /* flags[5:0] + padding = 11111100 */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  const uint8_t *unaligned_ptr = &payload[1];

  /* Verify non-extended */
  bool has_extension = J2735_VEHICLE_EVENT_FLAGS_HAS_EXTENSION(unaligned_ptr);
  TEST_ASSERT_FALSE_MESSAGE(has_extension, "Misaligned: extension bit should be 0");

  /* Verify all 13 root flags ON */
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0x1FFFU, J2735_VEHICLE_EVENT_FLAGS_GET(unaligned_ptr),
                                  "Misaligned: all 13 root flags should be ON (0x1FFF)");

  /* Verify size */
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(14U, J2735_VEHICLE_EVENT_FLAGS_SIZE(unaligned_ptr),
                                   "Misaligned: non-extended form should be 14 bits");

  /* Verify individual flag accessors on unaligned data */
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U,
                                  J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(unaligned_ptr),
                                  "Misaligned: eventHazardLights should be ON");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1U, J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(unaligned_ptr),
      "Misaligned: eventAirBagDeployment should be ON");
}

/* cppcheck-suppress-end misra-c2012-11.3 ; J2735_READ_BITS requires pointer cast */

/**
 * @brief Run all VehicleEventFlags tests.
 */
void run_testsuite_vehicle_event_flags(void) {
  RUN_TEST(test_vehicle_event_flags_non_extended);
  RUN_TEST(test_vehicle_event_flags_extended);
  /**
   * Individual flag accessor tests
   *
   * - Individual extended flags 0 to 4: Check bits 0-4
   * - Individual extended flags 5 to 9: Check bits 5-9
   * - Individual extended flags 10 to 13: Check bits 10-13
   * - Individual non extended flags 0 to 4: Check bits 0-4
   * - Individual non extended flags 5 to 9: Check bits 5-9
   * - Individual non extended flags 10 to 13: Check bits 10-13
   */
  RUN_TEST(test_vehicle_event_flags_individual_extended_flags_0_to_4);
  RUN_TEST(test_vehicle_event_flags_individual_extended_flags_5_to_9);
  RUN_TEST(test_vehicle_event_flags_individual_extended_flags_10_to_13);
  RUN_TEST(test_vehicle_event_flags_individual_non_extended_flags_0_to_4);
  RUN_TEST(test_vehicle_event_flags_individual_non_extended_flags_5_to_9);
  RUN_TEST(test_vehicle_event_flags_individual_non_extended_flags_10_to_13);
  /**
   * SIZE macro tests
   *
   * - Size non extended: 14 bits, check root form size
   * - Size extended: 22 bits, check extended form size
   */
  RUN_TEST(test_vehicle_event_flags_size_non_extended);
  RUN_TEST(test_vehicle_event_flags_size_extended);
  /**
   * Edge cases tests
   *
   * - All zeros non extended: Check all 13 flags OFF
   * - Extended single jackknife: Check only extension-only flag set
   * - Non extended all root flags on: : Check max value 0x1FFF
   * - Extended all zeros: : Check extended form with no flags
   */
  RUN_TEST(test_vehicle_event_flags_all_zeros_non_extended);
  RUN_TEST(test_vehicle_event_flags_extended_single_jackknife);
  RUN_TEST(test_vehicle_event_flags_non_extended_all_root_flags_on_metadata);
  RUN_TEST(test_vehicle_event_flags_non_extended_all_root_flags_on_bits);
  RUN_TEST(test_vehicle_event_flags_extended_all_zeros);
  /**
   * Alternating bit pattern tests (catch alignment bugs)
   *
   * - Alternating 0x1555: 1010101010101, check even bits ON
   * - Alternating 0x0AAA: 0101010101010, check odd bits ON
   */
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x1555_metadata);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x1555_0_to_4);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x1555_5_to_9);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x1555_10_to_12);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x0AAA_metadata);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x0AAA_0_to_4);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x0AAA_5_to_9);
  RUN_TEST(test_vehicle_event_flags_non_extended_alternating_0x0AAA_10_to_12);
  /**
   * Single-bit isolation tests
   *
   * - Single bit 0 hazard lights: bit 0 (MSB), check first flag isolation
   * - Single bit 12 airbag: bit 12 (LSB of root), check last root flag
   * - Single bit 7 hard braking: bit 7 (boundary), check middle/byte boundary
   * - Extended single hazard lights: bit 0 in 14-bit, check extended form root flag
   */
  RUN_TEST(test_vehicle_event_flags_single_bit_0_hazard_lights);
  RUN_TEST(test_vehicle_event_flags_single_bit_12_airbag);
  RUN_TEST(test_vehicle_event_flags_single_bit_7_hard_braking);
  RUN_TEST(test_vehicle_event_flags_extended_single_hazard_lights);
  /**
   * Misaligned buffer pointer test
   *
   * - Misaligned access: Forces &payload[1] to verify alignment safety
   */
  RUN_TEST(test_vehicle_event_flags_misaligned_access);
}
