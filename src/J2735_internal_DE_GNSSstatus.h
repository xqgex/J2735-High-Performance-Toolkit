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
 * @brief J2735 GNSSstatus Definition and Access Macros.
 *
 * @code
 * GNSSstatus ::= BIT STRING {
 *     unavailable (0),
 *     isHealthy (1),
 *     isMonitored (2),
 *     baseStationType (3),
 *     aPDOPofUnder5 (4),
 *     inViewOfUnder5 (5),
 *     localCorrectionsPresent (6),
 *     networkCorrectionsPresent (7)
 * } (SIZE (8))
 * @endcode
 *
 * Fixed BIT STRING with size 8.
 *
 * @par Wire Format (8 bits total):
 * @code
 * ┌──────────────────────────────────────────────────────────────┐
 * │ Bits 0-7                                                     │
 * ├──────────────────────────────────────────────────────────────┤
 * │ flags[0..7] (8 bits)                                         │
 * └──────────────────────────────────────────────────────────────┘
 * @endcode
 *
 * @par Optimization: Single-Read Strategy
 * @code
 * Max wire size = 8 bits ≤ 56-bit READ_BITS limit.
 * We read all 8 bits in ONE call, then use bit arithmetic to extract:
 *   - Flags at positions 0-7
 *
 * 8-bit read layout (left-justified from bit 0):
 *   [F0..F7] (8 flag bits, no extension marker)
 * @endcode
 */
#ifndef J2735_INTERNAL_DE_GNSSSTATUS_H
#define J2735_INTERNAL_DE_GNSSSTATUS_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only. Use the public J2735_GNSS_STATUS_GET_*() accessors instead.          */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_GNSS_STATUS_UNAVAILABLE                 0U
#define J2735_INTERNAL_BIT_GNSS_STATUS_IS_HEALTHY                  1U
#define J2735_INTERNAL_BIT_GNSS_STATUS_IS_MONITORED                2U
#define J2735_INTERNAL_BIT_GNSS_STATUS_BASE_STATION_TYPE           3U
#define J2735_INTERNAL_BIT_GNSS_STATUS_A_PDOP_OF_UNDER_5           4U
#define J2735_INTERNAL_BIT_GNSS_STATUS_IN_VIEW_OF_UNDER_5          5U
#define J2735_INTERNAL_BIT_GNSS_STATUS_LOCAL_CORRECTIONS_PRESENT   6U
#define J2735_INTERNAL_BIT_GNSS_STATUS_NETWORK_CORRECTIONS_PRESENT 7U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 8 bits from buffer for GNSSstatus.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for GNSSstatus.
 * All other macros operate purely on the 8-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded GNSSstatus (const uint8_t*).
 * @pre @p buf must point to at least 1 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the GNSSstatus encoding.
 * @return 8-bit value as uint64_t with wire bits left-justified.
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf) J2735_READ_BITS((buf), 0U, J2735_BW_GNSS_STATUS)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extension check for non-extensible GNSSstatus.
 *
 * This type is not extensible, so always returns false (0).
 *
 * @param[in] raw8 Value previously returned by J2735_INTERNAL_RAW_READ_GNSS_STATUS().
 * @return Always 0 (not extended).
 * @note Internal use only.
 */
#define J2735_INTERNAL_IS_EXTENSION_GNSS_STATUS(raw8) (0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 8-bit raw value.
 *
 * @param[in] raw8 Value previously returned by J2735_INTERNAL_RAW_READ_GNSS_STATUS().
 * @return Right-aligned flag bits as uint8_t:
 *         - 8 significant bits (0x0000-0x00FF)
 * @note Internal use only. Use J2735_GNSS_STATUS_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_GNSS_STATUS(raw8) ((uint8_t)((raw8) & 0xFFU))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 8-bit raw value.
 *
 * @param[in] raw8    Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_GNSS_STATUS().
 * @param[in] bit_pos ASN.1 bit position (0-7). Use J2735_INTERNAL_BIT_GNSS_STATUS_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @note Internal use only. Use J2735_GNSS_STATUS_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_GNSS_STATUS(raw8, bit_pos)                                          \
  ((uint8_t)(((raw8) >> (7U - (bit_pos))) & 1U))

/* ============================================================================================== */
/*  PUBLIC API: GNSSstatus Accessors                                                              */
/* ============================================================================================== */
/**
 * @brief Check if GNSSstatus is in extended form.
 *
 * GNSSstatus is a fixed-size BIT STRING with 8 bits.
 * It does not have an extension marker, so this always returns 0 (false).
 *
 * @param[in] buf Pointer to the start of the GNSSstatus UPER encoding (const uint8_t*).
 * @return Always 0 (false) - this type is not extensible.
 */
#define J2735_GNSS_STATUS_IS_EXTENDED(buf) ((void)(buf), 0)

/**
 * @brief Get wire size of GNSSstatus in bits.
 *
 * GNSSstatus is a fixed-size BIT STRING, always 8 bits on wire.
 *
 * @param[in] buf Pointer to the start of the GNSSstatus UPER encoding (const uint8_t*).
 * @return Always 8U.
 */
#define J2735_GNSS_STATUS_SIZE(buf) ((void)(buf), J2735_BW_GNSS_STATUS)

/**
 * @brief Get all GNSSstatus as a single uint8_t value.
 *
 * Returns all 8 flag bits packed into a right-aligned integer.
 *
 * @param[in] buf Pointer to the start of the GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid GNSSstatus encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 */
#define J2735_GNSS_STATUS_GET(buf)                                                                 \
  J2735_INTERNAL_GET_ALL_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf))

/**
 * @brief Get unavailable flag (ASN.1 bit 0).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_UNAVAILABLE(buf)                                                     \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_UNAVAILABLE)

/**
 * @brief Get isHealthy flag (ASN.1 bit 1).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_IS_HEALTHY(buf)                                                      \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_IS_HEALTHY)

/**
 * @brief Get isMonitored flag (ASN.1 bit 2).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_IS_MONITORED(buf)                                                    \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_IS_MONITORED)

/**
 * @brief Get baseStationType flag (ASN.1 bit 3).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_BASE_STATION_TYPE(buf)                                               \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_BASE_STATION_TYPE)

/**
 * @brief Get aPDOPofUnder5 flag (ASN.1 bit 4).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_A_PDOP_OF_UNDER_5(buf)                                               \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_A_PDOP_OF_UNDER_5)

/**
 * @brief Get inViewOfUnder5 flag (ASN.1 bit 5).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_IN_VIEW_OF_UNDER_5(buf)                                              \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_IN_VIEW_OF_UNDER_5)

/**
 * @brief Get localCorrectionsPresent flag (ASN.1 bit 6).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_LOCAL_CORRECTIONS_PRESENT(buf)                                       \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_LOCAL_CORRECTIONS_PRESENT)

/**
 * @brief Get networkCorrectionsPresent flag (ASN.1 bit 7).
 * @param[in] buf Pointer to GNSSstatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_GNSS_STATUS_GET_NETWORK_CORRECTIONS_PRESENT(buf)                                     \
  J2735_INTERNAL_GET_ONE_GNSS_STATUS(J2735_INTERNAL_RAW_READ_GNSS_STATUS(buf),                     \
                                     J2735_INTERNAL_BIT_GNSS_STATUS_NETWORK_CORRECTIONS_PRESENT)

#endif /* J2735_INTERNAL_DE_GNSSSTATUS_H */
