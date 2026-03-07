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
 * @brief J2735 LaneDirection Definition and Access Macros.
 *
 * LaneDirection ::= BIT STRING {
 *     ingressPath (0),
 *     egressPath (1)
 * } (SIZE (2))
 *
 * Fixed BIT STRING with size 2.
 *
 * Wire Format (2 bits total):
 * ┌──────────────────────────────────────────────────────────────┐
 * │ Bits 0-1                                                     │
 * ├──────────────────────────────────────────────────────────────┤
 * │ flags[0..1] (2 bits)                                         │
 * └──────────────────────────────────────────────────────────────┘
 *
 * Optimization: Single-Read Strategy
 * ──────────────────────────────────────────────────────────────────────────────────────────
 * Max wire size = 2 bits ≤ 56-bit READ_BITS limit.
 * We read all 2 bits in ONE call, then use bit arithmetic to extract:
 *   - Flags at positions 0-1
 *
 * 2-bit read layout (left-justified from bit 0):
 *   [F0..F1] (2 flag bits, no extension marker)
 */
#ifndef J2735_INTERNAL_DE_LANEDIRECTION_H
#define J2735_INTERNAL_DE_LANEDIRECTION_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of LaneDirection in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_LANE_DIRECTION 2U

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only. Use the public J2735_LANE_DIRECTION_GET_*() accessors instead.       */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_LANE_DIRECTION_INGRESS_PATH 0U
#define J2735_INTERNAL_BIT_LANE_DIRECTION_EGRESS_PATH  1U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 2 bits from buffer for LaneDirection.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for LaneDirection.
 * All other macros operate purely on the 2-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded LaneDirection (const uint8_t*).
 * @pre @p buf must point to at least 1 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the LaneDirection encoding.
 * @return 2-bit value as uint64_t with wire bits left-justified.
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_LANE_DIRECTION(buf)                                                \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_ROOT_SIZE_LANE_DIRECTION)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extension check for non-extensible LaneDirection.
 *
 * This type is not extensible, so always returns false (0).
 *
 * @param[in] raw2 Value previously returned by J2735_INTERNAL_RAW_READ_LANE_DIRECTION().
 * @return Always 0 (not extended).
 * @note Internal use only.
 */
#define J2735_INTERNAL_IS_EXTENSION_LANE_DIRECTION(raw2) (0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 2-bit raw value.
 *
 * @param[in] raw2 Value previously returned by J2735_INTERNAL_RAW_READ_LANE_DIRECTION().
 * @return Right-aligned flag bits as uint8_t:
 *         - 2 significant bits (0x0000-0x0003)
 * @note Internal use only. Use J2735_LANE_DIRECTION_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_LANE_DIRECTION(raw2) ((uint8_t)((raw2) & 0x3U))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 2-bit raw value.
 *
 * @param[in] raw2    Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_LANE_DIRECTION().
 * @param[in] bit_pos ASN.1 bit position (0-1). Use J2735_INTERNAL_BIT_LANE_DIRECTION_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @note Internal use only. Use J2735_LANE_DIRECTION_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_LANE_DIRECTION(raw2, bit_pos)                                       \
  ((uint8_t)(((raw2) >> (1U - (bit_pos))) & 1U))

/* ============================================================================================== */
/*  PUBLIC API: LaneDirection Accessors                                                           */
/* ============================================================================================== */
/**
 * @brief Check if LaneDirection is in extended form.
 *
 * LaneDirection is a fixed-size BIT STRING with 2 bits.
 * It does not have an extension marker, so this always returns 0 (false).
 *
 * @param[in] buf Pointer to the start of the LaneDirection UPER encoding (const uint8_t*).
 * @return Always 0 (false) - this type is not extensible.
 */
#define J2735_LANE_DIRECTION_IS_EXTENDED(buf) ((void)(buf), 0)

/**
 * @brief Get wire size of LaneDirection in bits.
 *
 * LaneDirection is a fixed-size BIT STRING, always 2 bits on wire.
 *
 * @param[in] buf Pointer to the start of the LaneDirection UPER encoding (const uint8_t*).
 * @return Always 2U.
 */
#define J2735_LANE_DIRECTION_SIZE(buf) ((void)(buf), J2735_INTERNAL_ROOT_SIZE_LANE_DIRECTION)

/**
 * @brief Get all LaneDirection as a single uint8_t value.
 *
 * Returns all 2 flag bits packed into a right-aligned integer.
 *
 * @param[in] buf Pointer to the start of the LaneDirection UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid LaneDirection encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 */
#define J2735_LANE_DIRECTION_GET(buf)                                                              \
  J2735_INTERNAL_GET_ALL_LANE_DIRECTION(J2735_INTERNAL_RAW_READ_LANE_DIRECTION(buf))

/**
 * @brief Get ingressPath flag (ASN.1 bit 0).
 * @param[in] buf Pointer to LaneDirection UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_DIRECTION_GET_INGRESS_PATH(buf)                                                 \
  J2735_INTERNAL_GET_ONE_LANE_DIRECTION(J2735_INTERNAL_RAW_READ_LANE_DIRECTION(buf),               \
                                        J2735_INTERNAL_BIT_LANE_DIRECTION_INGRESS_PATH)

/**
 * @brief Get egressPath flag (ASN.1 bit 1).
 * @param[in] buf Pointer to LaneDirection UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_LANE_DIRECTION_GET_EGRESS_PATH(buf)                                                  \
  J2735_INTERNAL_GET_ONE_LANE_DIRECTION(J2735_INTERNAL_RAW_READ_LANE_DIRECTION(buf),               \
                                        J2735_INTERNAL_BIT_LANE_DIRECTION_EGRESS_PATH)

#endif /* J2735_INTERNAL_DE_LANEDIRECTION_H */
