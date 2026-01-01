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
 * @brief J2735 In-Place Parsing Toolkit, Zero-Copy, Bit-Level Arithmetic Macros for J2735 UPER
 *        Access.
 * @note Optimized to use Packed-Casting (No memcpy call).
 * @section implementation_details
 * - Uses __attribute__((packed)) / #pragma pack(1) to define a safe unaligned type.
 * - Casts raw byte pointers to this type to force the compiler to emit safe
 *   unaligned load instructions (e.g. MOV/LDR).
 * - Performs Byte-Swapping (BSWAP) to handle Big-Endian UPER data on Little-Endian hosts.
 */
#ifndef J2735_TOOLKIT_H
#define J2735_TOOLKIT_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* Start Packing Region (Applies to all structs below) */
J2735_PACK_START

/**
 * @struct J2735_BSMcoreData_t
 * @brief Container for BSMcoreData (37 bytes / 290 bits).
 *
 * @par BSMcoreData Wire Format (UPER):
 * @code
 * | Offset | Bits 7-0                                                                             |
 * |:-------|:-------------------------------------------------------------------------------------|
 * | +0     | MsgCount(7) | TemporaryID (Bit 31)                                                   |
 * | +1     | TemporaryID (Bits 30-23)                                                             |
 * | +2     | TemporaryID (Bits 22-15)                                                             |
 * | +3     | TemporaryID (Bits 14-7)                                                              |
 * | +4     | TemporaryID (Bits 6-0) | DSecond (Bit 15)                                            |
 * | +5     | DSecond (Bits 14-7)                                                                  |
 * | +6     | DSecond (Bits 6-0) | Latitude (Bit 30)                                               |
 * | +7     | Latitude (Bits 29-22)                                                                |
 * | +8     | Latitude (Bits 21-14)                                                                |
 * | +9     | Latitude (Bits 13-6)                                                                 |
 * | +10    | Latitude (Bits 5-0) | Longitude (Bits 31-30)                                         |
 * | +11    | Longitude (Bits 29-22)                                                               |
 * | +12    | Longitude (Bits 21-14)                                                               |
 * | +13    | Longitude (Bits 13-6)                                                                |
 * | +14    | Longitude (Bits 5-0) | Elevation (Bits 15-14)                                        |
 * | +15    | Elevation (Bits 13-6)                                                                |
 * | +16    | Elevation (Bits 5-0) | PositionalAccuracy (Bits 31-30)                               |
 * | +17    | PositionalAccuracy (Bits 29-22)                                                      |
 * | +18    | PositionalAccuracy (Bits 21-14)                                                      |
 * | +19    | PositionalAccuracy (Bits 13-6)                                                       |
 * | +20    | PositionalAccuracy (Bits 5-0) | TransmissionState (Bits 2-1)                         |
 * | +21    | TransmissionState (Bit 0) | Speed (Bits 12-6)                                        |
 * | +22    | Speed (Bits 5-0) | Heading (Bits 14-13)                                              |
 * | +23    | Heading (Bits 12-5)                                                                  |
 * | +24    | Heading (Bits 4-0) | SteeringWheelAngle (Bits 7-5)                                   |
 * | +25    | SteeringWheelAngle (Bits 4-0) | AccelerationSet4Way (Bits 47-45)                     |
 * | +26    | AccelerationSet4Way (Bits 44-37)                                                     |
 * | +27    | AccelerationSet4Way (Bits 36-29)                                                     |
 * | +28    | AccelerationSet4Way (Bits 28-21)                                                     |
 * | +29    | AccelerationSet4Way (Bits 20-13)                                                     |
 * | +30    | AccelerationSet4Way (Bits 12-5)                                                      |
 * | +31    | AccelerationSet4Way (Bits 4-0) | BrakeSystemStatus (Bits 14-12)                      |
 * | +32    | BrakeSystemStatus (Bits 11-4)                                                        |
 * | +33    | BrakeSystemStatus (Bits 3-0) | VehicleSize (Bits 21-18)                              |
 * | +34    | VehicleSize (Bits 17-10)                                                             |
 * | +35    | VehicleSize (Bits 9-2)                                                               |
 * | +36    | VehicleSize (Bits 1-0)                                                               |
 * @endcode
 */
struct J2735_BSMcoreData {
  uint8_t raw_bytes[J2735_SIZE_BSM_CORE_DATA];
};
/* cppcheck-suppress misra-c2012-2.3 ; Placeholder struct for BSM message wrapper (WIP) */
typedef struct J2735_BSMcoreData J2735_BSMcoreData_t;

/**
 * @struct J2735_SPAT_Header_t
 * @par SPAT Header Wire Format:
 * @code
 * | Octet 0                                  | Octet 1                        |
 * |:-----------------------------------------|:-------------------------------|
 * | name_present(1) | intersections_pres(1)  | moy_pres(1) | ts_pres(1) | ... |
 * @endcode
 *
 * @todo Implement SPAT (Signal Phase and Timing) message parsing.
 *       Currently unused - reserved for future development.
 */
/* cppcheck-suppress misra-c2012-2.3 ; Placeholder for future SPAT implementation */
typedef struct {
  uint8_t raw_bytes[J2735_SIZE_SPAT_HEADER];
} J2735_SPAT_Header_t;

/**
 * @todo Add MAP and TIM headers with visual Doxygen diagrams
 */
/**
 * @todo Add bit-field structs for LaneAttributes (16-bit) and IntersectionStatus (16-bit)
 */

/* End Packing Region */
J2735_PACK_END

#endif /* J2735_TOOLKIT_H */
