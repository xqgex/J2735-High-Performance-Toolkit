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
 * @brief J2735 AllowedManeuvers Definition and Access Macros.
 *
 * @verbatim
 * AllowedManeuvers ::= BIT STRING {
 *     maneuverStraightAllowed (0),
 *     maneuverLeftAllowed (1),
 *     maneuverRightAllowed (2),
 *     maneuverUTurnAllowed (3),
 *     maneuverLeftTurnOnRedAllowed (4),
 *     maneuverRightTurnOnRedAllowed (5),
 *     maneuverLaneChangeAllowed (6),
 *     maneuverNoStoppingAllowed (7),
 *     yieldAllwaysRequired (8),
 *     goWithHalt (9),
 *     caution (10),
 *     reserved1 (11)
 * } (SIZE (12))
 * @endverbatim
 *
 * Fixed BIT STRING with size 12.
 *
 * @par Wire Format (12 bits total):
 * @verbatim
 * ┌──────────────────────────────────────────────────────────────┐
 * │ Bits 0-11                                                    │
 * ├──────────────────────────────────────────────────────────────┤
 * │ flags[0..11] (12 bits)                                       │
 * └──────────────────────────────────────────────────────────────┘
 * @endverbatim
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 12 bits ≤ 56-bit READ_BITS limit.
 * We read all 12 bits in ONE call, then use bit arithmetic to extract:
 *   - Flags at positions 0-11
 *
 * 12-bit read layout (left-justified from bit 0):
 *   [F0..F11] (12 flag bits, no extension marker)
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_ALLOWEDMANEUVERS_H
#define J2735_INTERNAL_DE_ALLOWEDMANEUVERS_H

#include "J2735_internal_common.h"

_Static_assert(J2735_BW_ALLOWED_MANEUVERS <= 56U,
               "BIT STRING must fit in a single 56-bit J2735_READ_BITS call");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only.                                                                      */
/*  Use the public J2735_ALLOWED_MANEUVERS_GET_*() accessors instead.                             */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_STRAIGHT_ALLOWED          0U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_LEFT_ALLOWED              1U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_RIGHT_ALLOWED             2U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_UT_URN_ALLOWED            3U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_LEFT_TURN_ON_RED_ALLOWED  4U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_RIGHT_TURN_ON_RED_ALLOWED 5U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_LANE_CHANGE_ALLOWED       6U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_NO_STOPPING_ALLOWED       7U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_YIELD_ALLWAYS_REQUIRED             8U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_GO_WITH_HALT                       9U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_CAUTION                            10U
#define J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_RESERVED_1                         11U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 12 bits from buffer for AllowedManeuvers.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for AllowedManeuvers.
 * All other macros operate purely on the 12-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded AllowedManeuvers (const uint8_t*).
 * @pre @p buf must point to at least 2 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the AllowedManeuvers encoding.
 * @return 12-bit value as uint64_t with wire bits left-justified.
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf)                                             \
  J2735_READ_BITS((buf), 0U, J2735_BW_ALLOWED_MANEUVERS)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extension check for non-extensible AllowedManeuvers.
 *
 * This type is not extensible, so always returns false (0).
 *
 * @param[in] raw12 Value previously returned by J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS().
 * @return Always 0 (not extended).
 * @note Internal use only.
 */
#define J2735_INTERNAL_IS_EXTENSION_ALLOWED_MANEUVERS(raw12) (0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 12-bit raw value.
 *
 * @param[in] raw12 Value previously returned by J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS().
 * @return Right-aligned flag bits as uint16_t:
 *         - 12 significant bits (0x0000-0x0FFF)
 * @note Internal use only. Use J2735_ALLOWED_MANEUVERS_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_ALLOWED_MANEUVERS(raw12) ((uint16_t)((raw12) & 0xFFFU))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 12-bit raw value.
 *
 * @param[in] raw12   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS().
 * @param[in] bit_pos ASN.1 bit position (0-11). Use J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @note Internal use only. Use J2735_ALLOWED_MANEUVERS_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(raw12, bit_pos)                                   \
  ((uint8_t)(((raw12) >> (11U - (bit_pos))) & 1U))

/* ============================================================================================== */
/*  PUBLIC API: AllowedManeuvers Accessors                                                        */
/* ============================================================================================== */
/**
 * @brief Check if AllowedManeuvers is in extended form.
 *
 * AllowedManeuvers is a fixed-size BIT STRING with 12 bits.
 * It does not have an extension marker, so this always returns 0 (false).
 *
 * @param[in] buf Pointer to the start of the AllowedManeuvers UPER encoding (const uint8_t*).
 * @return Always 0 (false) - this type is not extensible.
 */
#define J2735_ALLOWED_MANEUVERS_IS_EXTENDED(buf) ((void)(buf), 0)

/**
 * @brief Get wire size of AllowedManeuvers in bits.
 *
 * AllowedManeuvers is a fixed-size BIT STRING, always 12 bits on wire.
 *
 * @param[in] buf Pointer to the start of the AllowedManeuvers UPER encoding (const uint8_t*).
 * @return Always 12U.
 */
#define J2735_ALLOWED_MANEUVERS_SIZE(buf) ((void)(buf), J2735_BW_ALLOWED_MANEUVERS)

/**
 * @brief Get all AllowedManeuvers as a single uint16_t value.
 *
 * Returns all 12 flag bits packed into a right-aligned integer.
 *
 * @param[in] buf Pointer to the start of the AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid AllowedManeuvers encoding with +7 byte padding.
 * @return Right-aligned flag value (uint16_t). Bit 0 of result = first named bit.
 */
#define J2735_ALLOWED_MANEUVERS_GET(buf)                                                           \
  J2735_INTERNAL_GET_ALL_ALLOWED_MANEUVERS(J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf))

/**
 * @brief Get maneuverStraightAllowed flag (ASN.1 bit 0).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_STRAIGHT_ALLOWED(buf)                                 \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_STRAIGHT_ALLOWED)

/**
 * @brief Get maneuverLeftAllowed flag (ASN.1 bit 1).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LEFT_ALLOWED(buf)                                     \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_LEFT_ALLOWED)

/**
 * @brief Get maneuverRightAllowed flag (ASN.1 bit 2).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_RIGHT_ALLOWED(buf)                                    \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_RIGHT_ALLOWED)

/**
 * @brief Get maneuverUTurnAllowed flag (ASN.1 bit 3).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_UT_URN_ALLOWED(buf)                                   \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_UT_URN_ALLOWED)

/**
 * @brief Get maneuverLeftTurnOnRedAllowed flag (ASN.1 bit 4).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LEFT_TURN_ON_RED_ALLOWED(buf)                         \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_LEFT_TURN_ON_RED_ALLOWED)

/**
 * @brief Get maneuverRightTurnOnRedAllowed flag (ASN.1 bit 5).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_RIGHT_TURN_ON_RED_ALLOWED(buf)                        \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_RIGHT_TURN_ON_RED_ALLOWED)

/**
 * @brief Get maneuverLaneChangeAllowed flag (ASN.1 bit 6).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_LANE_CHANGE_ALLOWED(buf)                              \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_LANE_CHANGE_ALLOWED)

/**
 * @brief Get maneuverNoStoppingAllowed flag (ASN.1 bit 7).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_MANEUVER_NO_STOPPING_ALLOWED(buf)                              \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_MANEUVER_NO_STOPPING_ALLOWED)

/**
 * @brief Get yieldAllwaysRequired flag (ASN.1 bit 8).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_YIELD_ALLWAYS_REQUIRED(buf)                                    \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(                                                        \
      J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),                                              \
      J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_YIELD_ALLWAYS_REQUIRED)

/**
 * @brief Get goWithHalt flag (ASN.1 bit 9).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_GO_WITH_HALT(buf)                                              \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),         \
                                           J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_GO_WITH_HALT)

/**
 * @brief Get caution flag (ASN.1 bit 10).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_CAUTION(buf)                                                   \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),         \
                                           J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_CAUTION)

/**
 * @brief Get reserved1 flag (ASN.1 bit 11).
 * @param[in] buf Pointer to AllowedManeuvers UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_ALLOWED_MANEUVERS_GET_RESERVED_1(buf)                                                \
  J2735_INTERNAL_GET_ONE_ALLOWED_MANEUVERS(J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS(buf),         \
                                           J2735_INTERNAL_BIT_ALLOWED_MANEUVERS_RESERVED_1)

#endif /* J2735_INTERNAL_DE_ALLOWEDMANEUVERS_H */
