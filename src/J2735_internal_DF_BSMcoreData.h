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
 * @brief J2735 BSMcoreData Definition and Access Macros.
 *
 * @par BSMcoreData Wire Format (UPER):
 * @verbatim
 * BSMcoreData ::= SEQUENCE {
 *     msgCnt        MsgCount,             --  7 bits (unsigned, 0..127)
 *     id            TemporaryID,          -- 32 bits
 *     secMark       DSecond,              -- 16 bits (unsigned, 0..65535)
 *     lat           Latitude,             -- 31 bits (signed, -900000000..900000001)
 *     long          Longitude,            -- 32 bits (signed, -1799999999..1800000001)
 *     elev          Elevation,            -- 16 bits (signed, -4096..61439)
 *     accuracy      PositionalAccuracy,   -- 32 bits
 *     transmission  TransmissionState,    --  3 bits
 *     speed         Speed,                -- 13 bits (unsigned, 0..8191)
 *     heading       Heading,              -- 15 bits (unsigned, 0..28800)
 *     angle         SteeringWheelAngle,   --  8 bits (signed, -126..127)
 *     accelSet      AccelerationSet4Way,  -- 48 bits
 *     brakes        BrakeSystemStatus,    -- 15 bits
 *     size          VehicleSize           -- 22 bits
 * }
 * @endverbatim
 *
 * @par Wire Format (290 bits):
 * @verbatim
 * ┌──────────────┬───────────────────────────────────────────────┐
 * │ Bits         │ Content                                       │
 * ├──────────────┼───────────────────────────────────────────────┤
 * │ 0-6          │ msgCnt (7)                                    │
 * │ 7-38         │ id (32)                                       │
 * │ 39-54        │ secMark (16)                                  │
 * │ 55-85        │ lat (31)                                      │
 * │ 86-117       │ long (32)                                     │
 * │ 118-133      │ elev (16)                                     │
 * │ 134-165      │ accuracy (32)                                 │
 * │ 166-168      │ transmission (3)                              │
 * │ 169-181      │ speed (13)                                    │
 * │ 182-196      │ heading (15)                                  │
 * │ 197-204      │ angle (8)                                     │
 * │ 205-252      │ accelSet (48)                                 │
 * │ 253-267      │ brakes (15)                                   │
 * │ 268-289      │ size (22)                                     │
 * └──────────────┴───────────────────────────────────────────────┘
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DF_BSMCOREDATA_H
#define J2735_INTERNAL_DF_BSMCOREDATA_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* ============================================================================================== */
/*  INTERNAL: Structure Metadata                                                                  */
/* ============================================================================================== */
/**
 * @internal
 * @brief Number of prefix bits before first field (extension bit + optional preamble).
 *
 * Calculation: 0 ext + 0 opt = 0 bits (non-extensible, all required)
 */
#define J2735_INTERNAL_PREFIX_BITS_BSM_CORE_DATA                                                   \
  (0U +                                                                                            \
   J2735_INTERNAL_PREAMBLE_BITS(0U)) /* 0 ext + 0 opt = 0 bits (non-extensible, all required) */

/* ============================================================================================== */
/*  INTERNAL: Field Offsets                                                                       */
/*  (Cumulative bit offset: prev_offset + prev_width)                                             */
/* ============================================================================================== */
/**
 * @internal
 * @brief Bit offset of field 'msgCnt' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_MSG_CNT(buf)                                              \
  J2735_INTERNAL_PREFIX_BITS_BSM_CORE_DATA /*   0 */

/**
 * @internal
 * @brief Bit offset of field 'id' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_ID(buf)                                                   \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_MSG_CNT(buf) + J2735_BW_MSG_COUNT) /*   7 */

/**
 * @internal
 * @brief Bit offset of field 'secMark' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_SEC_MARK(buf)                                             \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_ID(buf) + J2735_BW_TEMPORARY_ID) /*  39 */

/**
 * @internal
 * @brief Bit offset of field 'lat' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_LAT(buf)                                                  \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_SEC_MARK(buf) + J2735_BW_D_SECOND) /*  55 */

/**
 * @internal
 * @brief Bit offset of field 'long' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_LONG(buf)                                                 \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_LAT(buf) + J2735_BW_LATITUDE) /*  86 */

/**
 * @internal
 * @brief Bit offset of field 'elev' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_ELEV(buf)                                                 \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_LONG(buf) + J2735_BW_LONGITUDE) /* 118 */

/**
 * @internal
 * @brief Bit offset of field 'accuracy' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_ACCURACY(buf)                                             \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_ELEV(buf) + J2735_BW_ELEVATION) /* 134 */

/**
 * @internal
 * @brief Bit offset of field 'transmission' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_TRANSMISSION(buf)                                         \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_ACCURACY(buf) + J2735_BW_POSITIONAL_ACCURACY) /* 166 */

/**
 * @internal
 * @brief Bit offset of field 'speed' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_SPEED(buf)                                                \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_TRANSMISSION(buf) + J2735_BW_TRANSMISSION_STATE) /* 169 */

/**
 * @internal
 * @brief Bit offset of field 'heading' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_HEADING(buf)                                              \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_SPEED(buf) + J2735_BW_SPEED) /* 182 */

/**
 * @internal
 * @brief Bit offset of field 'angle' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_ANGLE(buf)                                                \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_HEADING(buf) + J2735_BW_HEADING) /* 197 */

/**
 * @internal
 * @brief Bit offset of field 'accelSet' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_ACCEL_SET(buf)                                            \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_ANGLE(buf) + J2735_BW_STEERING_WHEEL_ANGLE) /* 205 */

/**
 * @internal
 * @brief Bit offset of field 'brakes' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_BRAKES(buf)                                               \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_ACCEL_SET(buf) + J2735_BW_ACCELERATION_SET_4_WAY) /* 253 */

/**
 * @internal
 * @brief Bit offset of field 'size' within BSMcoreData.
 */
#define J2735_INTERNAL_OFF_BSM_CORE_DATA_SIZE(buf)                                                 \
  (J2735_INTERNAL_OFF_BSM_CORE_DATA_BRAKES(buf) + J2735_BW_BRAKE_SYSTEM_STATUS) /* 268 */

_Static_assert((J2735_INTERNAL_OFF_BSM_CORE_DATA_SIZE(0) + J2735_BW_VEHICLE_SIZE) ==
                   J2735_BW_BSM_CORE_DATA,
               "BSMcoreData offset chain must sum to J2735_BW_BSM_CORE_DATA");

/* ============================================================================================== */
/*  PUBLIC API: Field Getters                                                                     */
/* ============================================================================================== */
/**
 * @brief Get 'msgCnt' (MsgCount, unsigned 7 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return MsgCount value (uint8_t, range 0..127).
 */
#define J2735_BSM_CORE_DATA_GET_MSG_CNT(buf)                                                       \
  ((uint8_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_MSG_CNT(buf),                  \
                            J2735_BW_MSG_COUNT))

/**
 * @brief Get 'id' (TemporaryID, unsigned 32 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return TemporaryID value (uint32_t, range 0..4294967295).
 */
#define J2735_BSM_CORE_DATA_GET_ID(buf)                                                            \
  ((uint32_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_ID(buf),                      \
                             J2735_BW_TEMPORARY_ID))

/**
 * @brief Get 'secMark' (DSecond, unsigned 16 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return DSecond value (uint16_t, range 0..65535).
 */
#define J2735_BSM_CORE_DATA_GET_SEC_MARK(buf)                                                      \
  ((uint16_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_SEC_MARK(buf),                \
                             J2735_BW_D_SECOND))

/**
 * @brief Get 'lat' (Latitude, signed 31 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return Latitude value (int32_t, range -900000000..900000001).
 */
#define J2735_BSM_CORE_DATA_GET_LAT(buf)                                                           \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_LAT(buf), J2735_BW_LATITUDE),        \
      J2735_BW_LATITUDE, int32_t)

/**
 * @brief Get 'long' (Longitude, signed 32 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return Longitude value (int32_t, range -1799999999..1800000001).
 */
#define J2735_BSM_CORE_DATA_GET_LONG(buf)                                                          \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_LONG(buf), J2735_BW_LONGITUDE),      \
      J2735_BW_LONGITUDE, int32_t)

/**
 * @brief Get 'elev' (Elevation, signed 16 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return Elevation value (int16_t, range -4096..61439).
 */
#define J2735_BSM_CORE_DATA_GET_ELEV(buf)                                                          \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_ELEV(buf), J2735_BW_ELEVATION),      \
      J2735_BW_ELEVATION, int16_t)

/**
 * @brief Get 'accuracy' (PositionalAccuracy, unsigned 32 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return PositionalAccuracy value (uint32_t, range 0..4294967295).
 */
#define J2735_BSM_CORE_DATA_GET_ACCURACY(buf)                                                      \
  ((uint32_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_ACCURACY(buf),                \
                             J2735_BW_POSITIONAL_ACCURACY))

/**
 * @brief Get 'transmission' (TransmissionState, unsigned 3 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return TransmissionState value (uint8_t, range 0..7).
 */
#define J2735_BSM_CORE_DATA_GET_TRANSMISSION(buf)                                                  \
  ((uint8_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_TRANSMISSION(buf),             \
                            J2735_BW_TRANSMISSION_STATE))

/**
 * @brief Get 'speed' (Speed, unsigned 13 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return Speed value (uint16_t, range 0..8191).
 */
#define J2735_BSM_CORE_DATA_GET_SPEED(buf)                                                         \
  ((uint16_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_SPEED(buf), J2735_BW_SPEED))

/**
 * @brief Get 'heading' (Heading, unsigned 15 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return Heading value (uint16_t, range 0..28800).
 */
#define J2735_BSM_CORE_DATA_GET_HEADING(buf)                                                       \
  ((uint16_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_HEADING(buf),                 \
                             J2735_BW_HEADING))

/**
 * @brief Get 'angle' (SteeringWheelAngle, signed 8 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return SteeringWheelAngle value (int8_t, range -126..127).
 */
#define J2735_BSM_CORE_DATA_GET_ANGLE(buf)                                                         \
  J2735_INTERNAL_SIGN_EXTEND(J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_ANGLE(buf),   \
                                             J2735_BW_STEERING_WHEEL_ANGLE),                       \
                             J2735_BW_STEERING_WHEEL_ANGLE, int8_t)

/**
 * @brief Get 'accelSet' (AccelerationSet4Way, unsigned 48 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return AccelerationSet4Way value (uint64_t, range 0..281474976710655).
 */
#define J2735_BSM_CORE_DATA_GET_ACCEL_SET(buf)                                                     \
  ((uint64_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_ACCEL_SET(buf),               \
                             J2735_BW_ACCELERATION_SET_4_WAY))

/**
 * @brief Get 'brakes' (BrakeSystemStatus, unsigned 15 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return BrakeSystemStatus value (uint16_t, range 0..32767).
 */
#define J2735_BSM_CORE_DATA_GET_BRAKES(buf)                                                        \
  ((uint16_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_BRAKES(buf),                  \
                             J2735_BW_BRAKE_SYSTEM_STATUS))

/**
 * @brief Get 'size' (VehicleSize, unsigned 22 bits).
 * @param[in] buf Pointer to the BSMcoreData encoding.
 * @return VehicleSize value (uint32_t, range 0..4194303).
 */
#define J2735_BSM_CORE_DATA_GET_SIZE(buf)                                                          \
  ((uint32_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_BSM_CORE_DATA_SIZE(buf),                    \
                             J2735_BW_VEHICLE_SIZE))

#endif /* J2735_INTERNAL_DF_BSMCOREDATA_H */
