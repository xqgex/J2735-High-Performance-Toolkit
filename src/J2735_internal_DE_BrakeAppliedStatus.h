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
 * @brief J2735 BrakeAppliedStatus Definition and Access Macros.
 *
 * @verbatim
 * BrakeAppliedStatus ::= BIT STRING {
 *     unavailable (0),
 *     leftFront (1),
 *     leftRear (2),
 *     rightFront (3),
 *     rightRear (4)
 * } (SIZE (5))
 * @endverbatim
 *
 * Fixed BIT STRING with size 5.
 *
 * @par Wire Format (5 bits total):
 * @verbatim
 * ┌──────────────────────────────────────────────────────────────┐
 * │ Bits 0-4                                                     │
 * ├──────────────────────────────────────────────────────────────┤
 * │ flags[0..4] (5 bits)                                         │
 * └──────────────────────────────────────────────────────────────┘
 * @endverbatim
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 5 bits ≤ 56-bit READ_BITS limit.
 * We read all 5 bits in ONE call, then use bit arithmetic to extract:
 *   - Flags at positions 0-4
 *
 * 5-bit read layout (left-justified from bit 0):
 *   [F0..F4] (5 flag bits, no extension marker)
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_BRAKEAPPLIEDSTATUS_H
#define J2735_INTERNAL_DE_BRAKEAPPLIEDSTATUS_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only.                                                                      */
/*  Use the public J2735_BRAKE_APPLIED_STATUS_GET_*() accessors instead.                          */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_UNAVAILABLE 0U
#define J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_LEFT_FRONT  1U
#define J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_LEFT_REAR   2U
#define J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_RIGHT_FRONT 3U
#define J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_RIGHT_REAR  4U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 5 bits from buffer for BrakeAppliedStatus.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for BrakeAppliedStatus.
 * All other macros operate purely on the 5-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded BrakeAppliedStatus (const uint8_t*).
 * @pre @p buf must point to at least 1 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the BrakeAppliedStatus encoding.
 * @return 5-bit value as uint64_t with wire bits left-justified.
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf)                                          \
  J2735_READ_BITS((buf), 0U, J2735_BW_BRAKE_APPLIED_STATUS)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extension check for non-extensible BrakeAppliedStatus.
 *
 * This type is not extensible, so always returns false (0).
 *
 * @param[in] raw5 Value previously returned by J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS().
 * @return Always 0 (not extended).
 * @note Internal use only.
 */
#define J2735_INTERNAL_IS_EXTENSION_BRAKE_APPLIED_STATUS(raw5) (0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 5-bit raw value.
 *
 * @param[in] raw5 Value previously returned by J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS().
 * @return Right-aligned flag bits as uint8_t:
 *         - 5 significant bits (0x00-0x1F)
 * @note Internal use only. Use J2735_BRAKE_APPLIED_STATUS_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_BRAKE_APPLIED_STATUS(raw5) ((uint8_t)((raw5) & 0x1FU))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 5-bit raw value.
 *
 * @param[in] raw5    Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS().
 * @param[in] bit_pos ASN.1 bit position (0-4). Use J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @note Internal use only. Use J2735_BRAKE_APPLIED_STATUS_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_BRAKE_APPLIED_STATUS(raw5, bit_pos)                                 \
  ((uint8_t)(((raw5) >> (4U - (bit_pos))) & 1U))

/* ============================================================================================== */
/*  PUBLIC API: BrakeAppliedStatus Accessors                                                      */
/* ============================================================================================== */
/**
 * @brief Check if BrakeAppliedStatus is in extended form.
 *
 * BrakeAppliedStatus is a fixed-size BIT STRING with 5 bits.
 * It does not have an extension marker, so this always returns 0 (false).
 *
 * @param[in] buf Pointer to the start of the BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @return Always 0 (false) - this type is not extensible.
 */
#define J2735_BRAKE_APPLIED_STATUS_IS_EXTENDED(buf) ((void)(buf), 0)

/**
 * @brief Get wire size of BrakeAppliedStatus in bits.
 *
 * BrakeAppliedStatus is a fixed-size BIT STRING, always 5 bits on wire.
 *
 * @param[in] buf Pointer to the start of the BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @return Always 5U.
 */
#define J2735_BRAKE_APPLIED_STATUS_SIZE(buf) ((void)(buf), J2735_BW_BRAKE_APPLIED_STATUS)

/**
 * @brief Get all BrakeAppliedStatus as a single uint8_t value.
 *
 * Returns all 5 flag bits packed into a right-aligned integer.
 *
 * @param[in] buf Pointer to the start of the BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid BrakeAppliedStatus encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 */
#define J2735_BRAKE_APPLIED_STATUS_GET(buf)                                                        \
  J2735_INTERNAL_GET_ALL_BRAKE_APPLIED_STATUS(J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf))

/**
 * @brief Get unavailable flag (ASN.1 bit 0).
 * @param[in] buf Pointer to BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_BRAKE_APPLIED_STATUS_GET_UNAVAILABLE(buf)                                            \
  J2735_INTERNAL_GET_ONE_BRAKE_APPLIED_STATUS(J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf),   \
                                              J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_UNAVAILABLE)

/**
 * @brief Get leftFront flag (ASN.1 bit 1).
 * @param[in] buf Pointer to BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_BRAKE_APPLIED_STATUS_GET_LEFT_FRONT(buf)                                             \
  J2735_INTERNAL_GET_ONE_BRAKE_APPLIED_STATUS(J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf),   \
                                              J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_LEFT_FRONT)

/**
 * @brief Get leftRear flag (ASN.1 bit 2).
 * @param[in] buf Pointer to BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_BRAKE_APPLIED_STATUS_GET_LEFT_REAR(buf)                                              \
  J2735_INTERNAL_GET_ONE_BRAKE_APPLIED_STATUS(J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf),   \
                                              J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_LEFT_REAR)

/**
 * @brief Get rightFront flag (ASN.1 bit 3).
 * @param[in] buf Pointer to BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_BRAKE_APPLIED_STATUS_GET_RIGHT_FRONT(buf)                                            \
  J2735_INTERNAL_GET_ONE_BRAKE_APPLIED_STATUS(J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf),   \
                                              J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_RIGHT_FRONT)

/**
 * @brief Get rightRear flag (ASN.1 bit 4).
 * @param[in] buf Pointer to BrakeAppliedStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_BRAKE_APPLIED_STATUS_GET_RIGHT_REAR(buf)                                             \
  J2735_INTERNAL_GET_ONE_BRAKE_APPLIED_STATUS(J2735_INTERNAL_RAW_READ_BRAKE_APPLIED_STATUS(buf),   \
                                              J2735_INTERNAL_BIT_BRAKE_APPLIED_STATUS_RIGHT_REAR)

#endif /* J2735_INTERNAL_DE_BRAKEAPPLIEDSTATUS_H */
