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
 * @brief J2735 TransitStatus Definition and Access Macros.
 *
 * TransitStatus ::= BIT STRING {
 *     none (0),
 *     anADAuse (1),
 *     aBikeLoad (2),
 *     doorOpen (3),
 *     occM (4),
 *     occL (5)
 * } (SIZE (6))
 *
 * Fixed BIT STRING with size 6.
 *
 * Wire Format (6 bits total):
 * ┌──────────────────────────────────────────────────────────────┐
 * │ Bits 0-5                                                     │
 * ├──────────────────────────────────────────────────────────────┤
 * │ flags[0..5] (6 bits)                                         │
 * └──────────────────────────────────────────────────────────────┘
 *
 * Optimization: Single-Read Strategy
 * ──────────────────────────────────────────────────────────────────────────────────────────
 * Max wire size = 6 bits ≤ 56-bit READ_BITS limit.
 * We read all 6 bits in ONE call, then use bit arithmetic to extract:
 *   - Flags at positions 0-5
 *
 * 6-bit read layout (left-justified from bit 0):
 *   [F0..F5] (6 flag bits, no extension marker)
 */
#ifndef J2735_INTERNAL_DE_TRANSITSTATUS_H
#define J2735_INTERNAL_DE_TRANSITSTATUS_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of TransitStatus in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_TRANSIT_STATUS 6U

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only. Use the public J2735_TRANSIT_STATUS_GET_*() accessors instead.       */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_TRANSIT_STATUS_NONE        0U
#define J2735_INTERNAL_BIT_TRANSIT_STATUS_AN_ADA_USE  1U
#define J2735_INTERNAL_BIT_TRANSIT_STATUS_A_BIKE_LOAD 2U
#define J2735_INTERNAL_BIT_TRANSIT_STATUS_DOOR_OPEN   3U
#define J2735_INTERNAL_BIT_TRANSIT_STATUS_OCC_M       4U
#define J2735_INTERNAL_BIT_TRANSIT_STATUS_OCC_L       5U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 6 bits from buffer for TransitStatus.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for TransitStatus.
 * All other macros operate purely on the 6-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded TransitStatus (const uint8_t*).
 * @pre @p buf must point to at least 1 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the TransitStatus encoding.
 * @return 6-bit value as uint64_t with wire bits left-justified.
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf)                                                \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_ROOT_SIZE_TRANSIT_STATUS)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extension check for non-extensible TransitStatus.
 *
 * This type is not extensible, so always returns false (0).
 *
 * @param[in] raw6 Value previously returned by J2735_INTERNAL_RAW_READ_TRANSIT_STATUS().
 * @return Always 0 (not extended).
 * @note Internal use only.
 */
#define J2735_INTERNAL_IS_EXTENSION_TRANSIT_STATUS(raw6) (0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 6-bit raw value.
 *
 * @param[in] raw6 Value previously returned by J2735_INTERNAL_RAW_READ_TRANSIT_STATUS().
 * @return Right-aligned flag bits as uint8_t:
 *         - 6 significant bits (0x0000-0x003F)
 * @note Internal use only. Use J2735_TRANSIT_STATUS_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_TRANSIT_STATUS(raw6) ((uint8_t)((raw6) & 0x3FU))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 6-bit raw value.
 *
 * @param[in] raw6    Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_TRANSIT_STATUS().
 * @param[in] bit_pos ASN.1 bit position (0-5). Use J2735_INTERNAL_BIT_TRANSIT_STATUS_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @note Internal use only. Use J2735_TRANSIT_STATUS_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(raw6, bit_pos)                                       \
  ((uint8_t)(((raw6) >> (5U - (bit_pos))) & 1U))

/* ============================================================================================== */
/*  PUBLIC API: TransitStatus Accessors                                                           */
/* ============================================================================================== */
/**
 * @brief Check if TransitStatus is in extended form.
 *
 * TransitStatus is a fixed-size BIT STRING with 6 bits.
 * It does not have an extension marker, so this always returns 0 (false).
 *
 * @param[in] buf Pointer to the start of the TransitStatus UPER encoding (const uint8_t*).
 * @return Always 0 (false) - this type is not extensible.
 */
#define J2735_TRANSIT_STATUS_IS_EXTENDED(buf) ((void)(buf), 0)

/**
 * @brief Get wire size of TransitStatus in bits.
 *
 * TransitStatus is a fixed-size BIT STRING, always 6 bits on wire.
 *
 * @param[in] buf Pointer to the start of the TransitStatus UPER encoding (const uint8_t*).
 * @return Always 6U.
 */
#define J2735_TRANSIT_STATUS_SIZE(buf) ((void)(buf), J2735_INTERNAL_ROOT_SIZE_TRANSIT_STATUS)

/**
 * @brief Get all TransitStatus as a single uint8_t value.
 *
 * Returns all 6 flag bits packed into a right-aligned integer.
 *
 * @param[in] buf Pointer to the start of the TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid TransitStatus encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 */
#define J2735_TRANSIT_STATUS_GET(buf)                                                              \
  J2735_INTERNAL_GET_ALL_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf))

/**
 * @brief Get none flag (ASN.1 bit 0).
 * @param[in] buf Pointer to TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRANSIT_STATUS_GET_NONE(buf)                                                         \
  J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf),               \
                                        J2735_INTERNAL_BIT_TRANSIT_STATUS_NONE)

/**
 * @brief Get anADAuse flag (ASN.1 bit 1).
 * @param[in] buf Pointer to TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRANSIT_STATUS_GET_AN_ADA_USE(buf)                                                   \
  J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf),               \
                                        J2735_INTERNAL_BIT_TRANSIT_STATUS_AN_ADA_USE)

/**
 * @brief Get aBikeLoad flag (ASN.1 bit 2).
 * @param[in] buf Pointer to TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRANSIT_STATUS_GET_A_BIKE_LOAD(buf)                                                  \
  J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf),               \
                                        J2735_INTERNAL_BIT_TRANSIT_STATUS_A_BIKE_LOAD)

/**
 * @brief Get doorOpen flag (ASN.1 bit 3).
 * @param[in] buf Pointer to TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRANSIT_STATUS_GET_DOOR_OPEN(buf)                                                    \
  J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf),               \
                                        J2735_INTERNAL_BIT_TRANSIT_STATUS_DOOR_OPEN)

/**
 * @brief Get occM flag (ASN.1 bit 4).
 * @param[in] buf Pointer to TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRANSIT_STATUS_GET_OCC_M(buf)                                                        \
  J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf),               \
                                        J2735_INTERNAL_BIT_TRANSIT_STATUS_OCC_M)

/**
 * @brief Get occL flag (ASN.1 bit 5).
 * @param[in] buf Pointer to TransitStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRANSIT_STATUS_GET_OCC_L(buf)                                                        \
  J2735_INTERNAL_GET_ONE_TRANSIT_STATUS(J2735_INTERNAL_RAW_READ_TRANSIT_STATUS(buf),               \
                                        J2735_INTERNAL_BIT_TRANSIT_STATUS_OCC_L)

#endif /* J2735_INTERNAL_DE_TRANSITSTATUS_H */
