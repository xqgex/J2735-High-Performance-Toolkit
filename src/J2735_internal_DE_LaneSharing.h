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
 * @brief J2735 LaneSharing Definition and Access Macros.
 *
 * @verbatim
 * LaneSharing ::= BIT STRING {
 *     overlappingLaneDescriptionProvided (0),
 *     multipleLanesTreatedAsOneLane (1),
 *     otherNonMotorizedTrafficTypes (2),
 *     individualMotorizedVehicleTraffic (3),
 *     busVehicleTraffic (4),
 *     taxiVehicleTraffic (5),
 *     pedestriansTraffic (6),
 *     cyclistVehicleTraffic (7),
 *     trackedVehicleTraffic (8),
 *     reserved (9)
 * } (SIZE (10))
 * @endverbatim
 *
 * Fixed BIT STRING with size 10.
 *
 * @par Wire Format (10 bits total):
 * @verbatim
 * ┌──────────────────────────────────────────────────────────────┐
 * │ Bits 0-9                                                     │
 * ├──────────────────────────────────────────────────────────────┤
 * │ flags[0..9] (10 bits)                                        │
 * └──────────────────────────────────────────────────────────────┘
 * @endverbatim
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 10 bits ≤ 56-bit READ_BITS limit.
 * We read all 10 bits in ONE call, then use bit arithmetic to extract:
 *   - Flags at positions 0-9
 *
 * 10-bit read layout (left-justified from bit 0):
 *   [F0..F9] (10 flag bits, no extension marker)
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_LANESHARING_H
#define J2735_INTERNAL_DE_LANESHARING_H

#include "J2735_internal_common.h"

_Static_assert(J2735_BW_LANE_SHARING <= 56U,
               "BIT STRING must fit in a single 56-bit J2735_READ_BITS call");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only.                                                                      */
/*  Use the public J2735_LANE_SHARING_GET_*() accessors instead.                                  */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_LANE_SHARING_OVERLAPPING_LANE_DESCRIPTION_PROVIDED 0U
#define J2735_INTERNAL_BIT_LANE_SHARING_MULTIPLE_LANES_TREATED_AS_ONE_LANE    1U
#define J2735_INTERNAL_BIT_LANE_SHARING_OTHER_NON_MOTORIZED_TRAFFIC_TYPES     2U
#define J2735_INTERNAL_BIT_LANE_SHARING_INDIVIDUAL_MOTORIZED_VEHICLE_TRAFFIC  3U
#define J2735_INTERNAL_BIT_LANE_SHARING_BUS_VEHICLE_TRAFFIC                   4U
#define J2735_INTERNAL_BIT_LANE_SHARING_TAXI_VEHICLE_TRAFFIC                  5U
#define J2735_INTERNAL_BIT_LANE_SHARING_PEDESTRIANS_TRAFFIC                   6U
#define J2735_INTERNAL_BIT_LANE_SHARING_CYCLIST_VEHICLE_TRAFFIC               7U
#define J2735_INTERNAL_BIT_LANE_SHARING_TRACKED_VEHICLE_TRAFFIC               8U
#define J2735_INTERNAL_BIT_LANE_SHARING_RESERVED                              9U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 10 bits from buffer for LaneSharing.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for LaneSharing.
 * All other macros operate purely on the 10-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded LaneSharing (const uint8_t*).
 * @pre @p buf must point to at least 2 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the LaneSharing encoding.
 * @return 10-bit value as uint64_t with wire bits left-justified.
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_LANE_SHARING(buf) J2735_READ_BITS((buf), 0U, J2735_BW_LANE_SHARING)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extension check for non-extensible LaneSharing.
 *
 * This type is not extensible, so always returns false (0).
 *
 * @param[in] raw10 Value previously returned by J2735_INTERNAL_RAW_READ_LANE_SHARING().
 * @return Always 0 (not extended).
 * @note Internal use only.
 */
#define J2735_INTERNAL_IS_EXTENSION_LANE_SHARING(raw10) (0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 10-bit raw value.
 *
 * @param[in] raw10 Value previously returned by J2735_INTERNAL_RAW_READ_LANE_SHARING().
 * @return Right-aligned flag bits as uint16_t:
 *         - 10 significant bits (0x0000-0x03FF)
 * @note Internal use only. Use J2735_LANE_SHARING_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_LANE_SHARING(raw10) ((uint16_t)((raw10) & 0x3FFU))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 10-bit raw value.
 *
 * @param[in] raw10   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_LANE_SHARING().
 * @param[in] bit_pos ASN.1 bit position (0-9). Use J2735_INTERNAL_BIT_LANE_SHARING_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @note Internal use only. Use J2735_LANE_SHARING_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_LANE_SHARING(raw10, bit_pos)                                        \
  ((uint8_t)(((raw10) >> (9U - (bit_pos))) & 1U))

/* ============================================================================================== */
/*  PUBLIC API: LaneSharing Accessors                                                             */
/* ============================================================================================== */
/**
 * @brief Check if LaneSharing has an extension.
 *
 * LaneSharing is a fixed-size BIT STRING with 10 bits.
 * It does not have an extension marker, so this always returns 0 (false).
 *
 * @param[in] buf Pointer to the start of the LaneSharing UPER encoding (const uint8_t*).
 * @return Always 0 (false) - this type is not extensible.
 */
#define J2735_LANE_SHARING_HAS_EXTENSION(buf) ((void)(buf), 0)

/**
 * @brief Get wire size of LaneSharing in bits.
 *
 * LaneSharing is a fixed-size BIT STRING, always 10 bits on wire.
 *
 * @param[in] buf Pointer to the start of the LaneSharing UPER encoding (const uint8_t*).
 * @return Always 10U.
 */
#define J2735_LANE_SHARING_SIZE(buf) ((void)(buf), J2735_BW_LANE_SHARING)

/**
 * @brief Get all LaneSharing as a single uint16_t value.
 *
 * Returns all 10 flag bits packed into a right-aligned integer.
 *
 * @param[in] buf Pointer to the start of the LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid LaneSharing encoding with +7 byte padding.
 * @return Right-aligned flag value (uint16_t). Bit 0 of result = first named bit.
 */
#define J2735_LANE_SHARING_GET(buf)                                                                \
  J2735_INTERNAL_GET_ALL_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf))

/**
 * @brief Get overlappingLaneDescriptionProvided flag (ASN.1 bit 0).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_OVERLAPPING_LANE_DESCRIPTION_PROVIDED(buf)                          \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(                                                             \
      J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                                                   \
      J2735_INTERNAL_BIT_LANE_SHARING_OVERLAPPING_LANE_DESCRIPTION_PROVIDED)

/**
 * @brief Get multipleLanesTreatedAsOneLane flag (ASN.1 bit 1).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_MULTIPLE_LANES_TREATED_AS_ONE_LANE(buf)                             \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(                                                             \
      J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                                                   \
      J2735_INTERNAL_BIT_LANE_SHARING_MULTIPLE_LANES_TREATED_AS_ONE_LANE)

/**
 * @brief Get otherNonMotorizedTrafficTypes flag (ASN.1 bit 2).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_OTHER_NON_MOTORIZED_TRAFFIC_TYPES(buf)                              \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(                                                             \
      J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                                                   \
      J2735_INTERNAL_BIT_LANE_SHARING_OTHER_NON_MOTORIZED_TRAFFIC_TYPES)

/**
 * @brief Get individualMotorizedVehicleTraffic flag (ASN.1 bit 3).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_INDIVIDUAL_MOTORIZED_VEHICLE_TRAFFIC(buf)                           \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(                                                             \
      J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                                                   \
      J2735_INTERNAL_BIT_LANE_SHARING_INDIVIDUAL_MOTORIZED_VEHICLE_TRAFFIC)

/**
 * @brief Get busVehicleTraffic flag (ASN.1 bit 4).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_BUS_VEHICLE_TRAFFIC(buf)                                            \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                   \
                                      J2735_INTERNAL_BIT_LANE_SHARING_BUS_VEHICLE_TRAFFIC)

/**
 * @brief Get taxiVehicleTraffic flag (ASN.1 bit 5).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_TAXI_VEHICLE_TRAFFIC(buf)                                           \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                   \
                                      J2735_INTERNAL_BIT_LANE_SHARING_TAXI_VEHICLE_TRAFFIC)

/**
 * @brief Get pedestriansTraffic flag (ASN.1 bit 6).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_PEDESTRIANS_TRAFFIC(buf)                                            \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                   \
                                      J2735_INTERNAL_BIT_LANE_SHARING_PEDESTRIANS_TRAFFIC)

/**
 * @brief Get cyclistVehicleTraffic flag (ASN.1 bit 7).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_CYCLIST_VEHICLE_TRAFFIC(buf)                                        \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                   \
                                      J2735_INTERNAL_BIT_LANE_SHARING_CYCLIST_VEHICLE_TRAFFIC)

/**
 * @brief Get trackedVehicleTraffic flag (ASN.1 bit 8).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_TRACKED_VEHICLE_TRAFFIC(buf)                                        \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                   \
                                      J2735_INTERNAL_BIT_LANE_SHARING_TRACKED_VEHICLE_TRAFFIC)

/**
 * @brief Get reserved flag (ASN.1 bit 9).
 * @param[in] buf Pointer to LaneSharing UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_SHARING_GET_RESERVED(buf)                                                       \
  J2735_INTERNAL_GET_ONE_LANE_SHARING(J2735_INTERNAL_RAW_READ_LANE_SHARING(buf),                   \
                                      J2735_INTERNAL_BIT_LANE_SHARING_RESERVED)

#endif /* J2735_INTERNAL_DE_LANESHARING_H */
