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
 * BSMcoreData ::= SEQUENCE {
 *     msgCnt        MsgCount,             --  7 bits (J2735_BW_MSG_COUNT)
 *     id            TemporaryID,          -- 32 bits (J2735_BW_TEMPORARY_ID)
 *     secMark       DSecond,              -- 16 bits (J2735_BW_DS_ECOND)
 *     lat           Latitude,             -- 31 bits (J2735_BW_LATITUDE) [signed]
 *     long          Longitude,            -- 32 bits (J2735_BW_LONGITUDE) [signed]
 *     elev          Elevation,            -- 16 bits (J2735_BW_ELEVATION) [signed]
 *     accuracy      PositionalAccuracy,   -- 32 bits (J2735_BW_POSITIONAL_ACCURACY)
 *     transmission  TransmissionState,    --  3 bits (J2735_BW_TRANSMISSION_STATE)
 *     speed         Speed,                -- 13 bits (J2735_BW_SPEED)
 *     heading       Heading,              -- 15 bits (J2735_BW_HEADING)
 *     angle         SteeringWheelAngle,   --  8 bits (J2735_BW_STEERING_WHEEL_ANGLE) [signed]
 *     accelSet      AccelerationSet4Way,  -- 48 bits (J2735_BW_ACCELERATION_SET_4_WAY)
 *     brakes        BrakeSystemStatus,    -- 15 bits (J2735_BW_BRAKE_SYSTEM_STATUS)
 *     size          VehicleSize           -- 22 bits (J2735_BW_VEHICLE_SIZE)
 * }
 *
 * Wire Format (fixed 290 bits, no optional fields, not extensible):
 * | Bits 0-6  | Bits 7-38 | Bits 39-54 | Bits 55-85 | ... | Bits 268-289 |
 * |-----------|-----------|------------|------------|-----|--------------|
 * | msgCnt(7) | id(32)    | secMark(16)| lat(31)    | ... | size(22)     |
 *
 * @todo Update the Doxygen to indicate [in] and [out] parameters
 */
#ifndef J2735_INTERNAL_DF_BSMCOREDATA_H
#define J2735_INTERNAL_DF_BSMCOREDATA_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* Internal - Structure metadata */
#define J2735_PREFIX_BITS_BSM_CORE_DATA                                                            \
  (0U + J2735_INTERNAL_PREAMBLE_BITS(0U)) /* 0 ext + 0 opt = 0 bits (non-extensible, all required) */

/* Internal - Root component size (for calculating where extensions start) */

/* Internal - Optional field indices (bitmap index, not bit offset) */

/* Internal - Widths */

/* Internal - Offsets */
#define J2735_OFF_BSM_CORE_DATA_MSG_CNT(buf) J2735_PREFIX_BITS_BSM_CORE_DATA /*   0 */
#define J2735_OFF_BSM_CORE_DATA_ID(buf)                                                            \
  (J2735_OFF_BSM_CORE_DATA_MSG_CNT(buf) + J2735_BW_MSG_COUNT) /*   7 */
#define J2735_OFF_BSM_CORE_DATA_SEC_MARK(buf)                                                      \
  (J2735_OFF_BSM_CORE_DATA_ID(buf) + J2735_BW_TEMPORARY_ID) /*  39 */
#define J2735_OFF_BSM_CORE_DATA_LAT(buf)                                                           \
  (J2735_OFF_BSM_CORE_DATA_SEC_MARK(buf) + J2735_BW_DS_ECOND) /*  55 */
#define J2735_OFF_BSM_CORE_DATA_LONG(buf)                                                          \
  (J2735_OFF_BSM_CORE_DATA_LAT(buf) + J2735_BW_LATITUDE) /*  86 */
#define J2735_OFF_BSM_CORE_DATA_ELEV(buf)                                                          \
  (J2735_OFF_BSM_CORE_DATA_LONG(buf) + J2735_BW_LONGITUDE) /* 118 */
#define J2735_OFF_BSM_CORE_DATA_ACCURACY(buf)                                                      \
  (J2735_OFF_BSM_CORE_DATA_ELEV(buf) + J2735_BW_ELEVATION) /* 134 */
#define J2735_OFF_BSM_CORE_DATA_TRANSMISSION(buf)                                                  \
  (J2735_OFF_BSM_CORE_DATA_ACCURACY(buf) + J2735_BW_POSITIONAL_ACCURACY) /* 166 */
#define J2735_OFF_BSM_CORE_DATA_SPEED(buf)                                                         \
  (J2735_OFF_BSM_CORE_DATA_TRANSMISSION(buf) + J2735_BW_TRANSMISSION_STATE) /* 169 */
#define J2735_OFF_BSM_CORE_DATA_HEADING(buf)                                                       \
  (J2735_OFF_BSM_CORE_DATA_SPEED(buf) + J2735_BW_SPEED) /* 182 */
#define J2735_OFF_BSM_CORE_DATA_ANGLE(buf)                                                         \
  (J2735_OFF_BSM_CORE_DATA_HEADING(buf) + J2735_BW_HEADING) /* 197 */
#define J2735_OFF_BSM_CORE_DATA_ACCEL_SET(buf)                                                     \
  (J2735_OFF_BSM_CORE_DATA_ANGLE(buf) + J2735_BW_STEERING_WHEEL_ANGLE) /* 205 */
#define J2735_OFF_BSM_CORE_DATA_BRAKES(buf)                                                        \
  (J2735_OFF_BSM_CORE_DATA_ACCEL_SET(buf) + J2735_BW_ACCELERATION_SET_4_WAY) /* 253 */
#define J2735_OFF_BSM_CORE_DATA_SIZE(buf)                                                          \
  (J2735_OFF_BSM_CORE_DATA_BRAKES(buf) + J2735_BW_BRAKE_SYSTEM_STATUS) /* 268 */

/* Has-checkers */

/* Getters */
/**
 * @brief Get 'msgCnt' (MsgCount, unsigned 7 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return MsgCount value (uint8_t, range 0..127).
 */
#define J2735_BSM_CORE_DATA_GET_MSG_CNT(buf)                                                       \
  ((uint8_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_MSG_CNT(buf), J2735_BW_MSG_COUNT))
/**
 * @brief Get 'id' (TemporaryID, unsigned 32 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return TemporaryID value (uint32_t, range 0..4294967295).
 */
#define J2735_BSM_CORE_DATA_GET_ID(buf)                                                            \
  ((uint32_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_ID(buf), J2735_BW_TEMPORARY_ID))
/**
 * @brief Get 'secMark' (DSecond, unsigned 16 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return DSecond value (uint16_t, range 0..65535).
 */
#define J2735_BSM_CORE_DATA_GET_SEC_MARK(buf)                                                      \
  ((uint16_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_SEC_MARK(buf), J2735_BW_DS_ECOND))
/**
 * @brief Get 'lat' (Latitude, signed 31 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return Latitude value (int32_t, range -900000000..900000001).
 */
#define J2735_BSM_CORE_DATA_GET_LAT(buf)                                                           \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_LAT(buf), J2735_BW_LATITUDE),                 \
      J2735_BW_LATITUDE, int32_t)
/**
 * @brief Get 'long' (Longitude, signed 32 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return Longitude value (int32_t, range -1799999999..1800000001).
 */
#define J2735_BSM_CORE_DATA_GET_LONG(buf)                                                          \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_LONG(buf), J2735_BW_LONGITUDE),               \
      J2735_BW_LONGITUDE, int32_t)
/**
 * @brief Get 'elev' (Elevation, signed 16 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return Elevation value (int16_t, range -4096..61439).
 */
#define J2735_BSM_CORE_DATA_GET_ELEV(buf)                                                          \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_ELEV(buf), J2735_BW_ELEVATION),               \
      J2735_BW_ELEVATION, int16_t)
/**
 * @brief Get 'accuracy' (PositionalAccuracy, unsigned 32 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return PositionalAccuracy value (uint32_t, range 0..4294967295).
 */
#define J2735_BSM_CORE_DATA_GET_ACCURACY(buf)                                                      \
  ((uint32_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_ACCURACY(buf),                         \
                             J2735_BW_POSITIONAL_ACCURACY))
/**
 * @brief Get 'transmission' (TransmissionState, unsigned 3 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return TransmissionState value (uint8_t, range 0..7).
 */
#define J2735_BSM_CORE_DATA_GET_TRANSMISSION(buf)                                                  \
  ((uint8_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_TRANSMISSION(buf),                      \
                            J2735_BW_TRANSMISSION_STATE))
/**
 * @brief Get 'speed' (Speed, unsigned 13 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return Speed value (uint16_t, range 0..8191).
 */
#define J2735_BSM_CORE_DATA_GET_SPEED(buf)                                                         \
  ((uint16_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_SPEED(buf), J2735_BW_SPEED))
/**
 * @brief Get 'heading' (Heading, unsigned 15 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return Heading value (uint16_t, range 0..28800).
 */
#define J2735_BSM_CORE_DATA_GET_HEADING(buf)                                                       \
  ((uint16_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_HEADING(buf), J2735_BW_HEADING))
/**
 * @brief Get 'angle' (SteeringWheelAngle, signed 8 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return SteeringWheelAngle value (int8_t, range -126..127).
 */
#define J2735_BSM_CORE_DATA_GET_ANGLE(buf)                                                         \
  J2735_INTERNAL_SIGN_EXTEND(                                                                      \
      J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_ANGLE(buf), J2735_BW_STEERING_WHEEL_ANGLE),   \
      J2735_BW_STEERING_WHEEL_ANGLE, int8_t)
/**
 * @brief Get 'accelSet' (AccelerationSet4Way, unsigned 48 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return AccelerationSet4Way value (uint64_t, range 0..281474976710655).
 */
#define J2735_BSM_CORE_DATA_GET_ACCEL_SET(buf)                                                     \
  ((uint64_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_ACCEL_SET(buf),                        \
                             J2735_BW_ACCELERATION_SET_4_WAY))
/**
 * @brief Get 'brakes' (BrakeSystemStatus, unsigned 15 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return BrakeSystemStatus value (uint16_t, range 0..32767).
 */
#define J2735_BSM_CORE_DATA_GET_BRAKES(buf)                                                        \
  ((uint16_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_BRAKES(buf),                           \
                             J2735_BW_BRAKE_SYSTEM_STATUS))
/**
 * @brief Get 'size' (VehicleSize, unsigned 22 bits).
 * @param buf Pointer to the BSMcoreData encoding.
 * @return VehicleSize value (uint32_t, range 0..4194303).
 */
#define J2735_BSM_CORE_DATA_GET_SIZE(buf)                                                          \
  ((uint32_t)J2735_READ_BITS((buf), J2735_OFF_BSM_CORE_DATA_SIZE(buf), J2735_BW_VEHICLE_SIZE))

/* Inline Functions */

#endif /* J2735_INTERNAL_DF_BSMCOREDATA_H */
