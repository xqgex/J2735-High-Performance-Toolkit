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
 * @brief J2735 TrafficLightOperationStatus Definition and Access Macros.
 *
 * @verbatim
 * TrafficLightOperationStatus ::= BIT STRING {
 *     manual (0),
 *     flashing (1),
 *     off (2),
 *     actuated (3),
 *     transition (4),
 *     priority (5),
 *     phase (6),
 *     reserved (7)
 * } (SIZE (8, ...))
 * @endverbatim
 *
 * Extensible BIT STRING with root size 8 and known extension size 8.
 *
 * @par Wire Format (non-extended, 9 bits total):
 * @verbatim
 * ┌───────┬──────────────────────────────────────────────────────┐
 * │ Bit 0 │ Bits 1-8                                             │
 * ├───────┼──────────────────────────────────────────────────────┤
 * │ Ext=0 │ flags[0..7] (8 bits)                                 │
 * └───────┴──────────────────────────────────────────────────────┘
 * @endverbatim
 *
 * @par Wire Format (extended, 16 bits total):
 * @verbatim
 * ┌───────┬────────────────────┬─────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7           │ Bits 8-15                       │
 * ├───────┼────────────────────┼─────────────────────────────────┤
 * │ Ext=1 │ nsnnwn=8 (7 bits)  │ flags[0..7] (8 bits)            │
 * └───────┴────────────────────┴─────────────────────────────────┘
 * @endverbatim
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 16 bits ≤ 56-bit READ_BITS limit.
 * We read all 16 bits in ONE call, then use bit arithmetic to extract:
 *   - Extension bit at position 15 (MSB of 16-bit value)
 *   - Flags at positions 0-7 (extended) or shifted for non-extended
 *
 * 16-bit read layout (left-justified from bit 0):
 *   Non-extended: [Ext=0][F0..F7][7 garbage bits]
 *                  bit15  14..7     6..0
 *   Extended:     [Ext=1][nsnnwn:7][F0..F7]
 *                  bit15  14..8    7..0
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_TRAFFICLIGHTOPERATIONSTATUS_H
#define J2735_INTERNAL_DE_TRAFFICLIGHTOPERATIONSTATUS_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of TrafficLightOperationStatus in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS 8U

/**
 * @internal
 * @brief Extended size of TrafficLightOperationStatus in bits.
 */
#define J2735_INTERNAL_EXT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS 8U

/**
 * @internal
 * @brief Maximum wire encoding size of TrafficLightOperationStatus in bits.
 *
 * Used for single-read optimization: read all possible bits at once.
 * Calculated as: 1 (ext bit) + 7 (nsnnwn) + 8U (max payload).
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS 16U

_Static_assert(J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS ==
                   (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_NSNNWN_SMALL_BITS +
                    J2735_INTERNAL_EXT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS),
               "MAX_WIRE_BITS must equal ext_marker + nsnnwn + ext_size");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only.                                                                      */
/*  Use the public J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_*() accessors instead.                */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_MANUAL     0U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_FLASHING   1U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_OFF        2U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_ACTUATED   3U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_TRANSITION 4U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_PRIORITY   5U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_PHASE      6U
#define J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_RESERVED   7U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 16 bits from buffer for TrafficLightOperationStatus.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for TrafficLightOperationStatus.
 * All other macros operate purely on the 16-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded TrafficLightOperationStatus (const uint8_t*).
 * @pre @p buf must point to at least 2 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the TrafficLightOperationStatus encoding.
 * @return 16-bit value as uint64_t with wire bits left-justified:
 *         - Bit 15: Extension marker (0=root, 1=extended)
 *         - Bits 14..0: Payload (format depends on extension state)
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf)                                \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Check extension bit from pre-read 16-bit raw value.
 *
 * The extension bit is at position 15 (MSB of the 16-bit read):
 * - 0 = Root form: 8 flag bits follow immediately after extension bit
 * - 1 = Extended form: 7-bit nsnnwn (value=8) followed by 8 flag bits
 *
 * @param[in] raw16 Value previously returned by
 * J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS().
 * @return Non-zero (true) if extended form, zero (false) if root form.
 * @note Internal use only. Use J2735_TRAFFIC_LIGHT_OPERATION_STATUS_IS_EXTENDED() for public API.
 */
#define J2735_INTERNAL_IS_EXTENSION_TRAFFIC_LIGHT_OPERATION_STATUS(raw16)                          \
  (((raw16) >> (J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS - 1U)) != 0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 16-bit raw value.
 *
 * Bit layout depends on extension state:
 *
 * Non-extended (bit 15 = 0):
 *   raw16: [0][F0][F1]...[F7][garbage:7]
 *           15 14  13      7     6..0
 *   Extract: (raw16 >> 7) & 0xFF -> 8 bits
 *
 * Extended (bit 15 = 1):
 *   raw16: [1][nsnnwn:7][F0][F1]...[F7]
 *           15  14.. 8    7   6      0
 *   Extract: raw16 & 0xFF -> 8 bits
 *
 * @param[in] raw16 Value previously returned by
 * J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS().
 * @return Right-aligned flag bits as uint8_t:
 *         - 8 significant bits (0x00-0xFF) if non-extended
 *         - 8 significant bits (0x00-0xFF) if extended
 * @note Uses 1ULL for bit shifts to prevent undefined behavior if size >= 32 bits.
 *       The compiler optimizes the truncation to uint8_t.
 * @note Internal use only. Use J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_TRAFFIC_LIGHT_OPERATION_STATUS(raw16)                               \
  (J2735_INTERNAL_IS_EXTENSION_TRAFFIC_LIGHT_OPERATION_STATUS(raw16)                               \
       ? /* Extended: low 8 bits */                                                                \
       ((uint8_t)((raw16) &                                                                        \
                  ((1ULL << J2735_INTERNAL_EXT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS) - 1ULL)))      \
       : /* Non-ext: bits 14..7 = 8 bits */                                                        \
       ((uint8_t)(((raw16) >> (J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS - 1U -  \
                               J2735_INTERNAL_ROOT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS)) &         \
                  ((1ULL << J2735_INTERNAL_ROOT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS) - 1ULL))))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 16-bit raw value.
 *
 * Computes the bit position within raw16 based on extension state:
 *
 * Non-extended: Flag[N] is at raw16 bit (14 - N)
 *   F0 at bit 14, F1 at bit 13, ..., F7 at bit 7
 *   Formula: (raw16 >> (14 - bit_pos)) & 1
 *
 * Extended: Flag[N] is at raw16 bit (7 - N)
 *   F0 at bit 7, F1 at bit 6, ..., F7 at bit 0
 *   Formula: (raw16 >> (7 - bit_pos)) & 1
 *
 * @param[in] raw16   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS().
 * @param[in] bit_pos ASN.1 bit position (0-7). Use
 * J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_* constants.
 * @return 0 or 1 as uint8_t.
 * @warning For non-extended messages, bit_pos >= 8 reads undefined garbage bits.
 *          Caller should verify IS_EXTENDED before accessing extension-only flags.
 * @note Internal use only. Use J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_*() accessors for public
 * API.
 */
#define J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(raw16, bit_pos)                      \
  ((uint8_t)(J2735_INTERNAL_IS_EXTENSION_TRAFFIC_LIGHT_OPERATION_STATUS(raw16)                     \
                 ? /* Extended: F0 at bit 7 */                                                     \
                 (((raw16) >>                                                                      \
                   ((J2735_INTERNAL_EXT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS - 1U) - (bit_pos))) &  \
                  1U)                                                                              \
                 : /* Non-ext: F0 at bit 14 */                                                     \
                 (((raw16) >>                                                                      \
                   ((J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS - 2U) -           \
                    (bit_pos))) &                                                                  \
                  1U)))

/* ============================================================================================== */
/*  PUBLIC API: TrafficLightOperationStatus Accessors                                             */
/* ============================================================================================== */
/**
 * @brief Check if TrafficLightOperationStatus is in extended form.
 *
 * Extended form includes 8 flags (bits 0-7).
 * Root form has only 8 flags (bits 0-7).
 *
 * @param[in] buf Pointer to the start of the TrafficLightOperationStatus UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid TrafficLightOperationStatus encoding with +7 byte padding.
 * @return Non-zero (true) if extended (8 flags), zero (false) if root (8 flags).
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_IS_EXTENDED(buf)                                      \
  J2735_INTERNAL_IS_EXTENSION_TRAFFIC_LIGHT_OPERATION_STATUS(                                      \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf))

/**
 * @brief Get wire size of TrafficLightOperationStatus in bits.
 *
 * Use this to advance a bit cursor past the TrafficLightOperationStatus field.
 *
 * @param[in] buf Pointer to the start of the TrafficLightOperationStatus UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid TrafficLightOperationStatus encoding with +7 byte padding.
 * @return 16U if extended form, 9U if root form.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_SIZE(buf)                                             \
  (J2735_INTERNAL_IS_EXTENSION_TRAFFIC_LIGHT_OPERATION_STATUS(                                     \
       J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf))                                \
       ? J2735_INTERNAL_MAX_WIRE_BITS_TRAFFIC_LIGHT_OPERATION_STATUS                               \
       : (J2735_INTERNAL_EXTENSION_MARKER_BITS +                                                   \
          J2735_INTERNAL_ROOT_SIZE_TRAFFIC_LIGHT_OPERATION_STATUS))

/**
 * @brief Get all TrafficLightOperationStatus as a single uint8_t value.
 *
 * Returns all flag bits packed into a right-aligned integer.
 * The number of significant bits depends on extension state:
 * - Root form: bits 0-7 valid (8 bits), bit 8 is zero
 * - Extended form: bits 0-7 valid (8 bits)
 *
 * @param[in] buf Pointer to the start of the TrafficLightOperationStatus UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid TrafficLightOperationStatus encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 * @note Use J2735_TRAFFIC_LIGHT_OPERATION_STATUS_IS_EXTENDED() to determine if bit 8 is meaningful.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET(buf)                                              \
  J2735_INTERNAL_GET_ALL_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf))

/**
 * @brief Get manual flag (ASN.1 bit 0).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_MANUAL(buf)                                       \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_MANUAL)

/**
 * @brief Get flashing flag (ASN.1 bit 1).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_FLASHING(buf)                                     \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_FLASHING)

/**
 * @brief Get off flag (ASN.1 bit 2).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_OFF(buf)                                          \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_OFF)

/**
 * @brief Get actuated flag (ASN.1 bit 3).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_ACTUATED(buf)                                     \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_ACTUATED)

/**
 * @brief Get transition flag (ASN.1 bit 4).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_TRANSITION(buf)                                   \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_TRANSITION)

/**
 * @brief Get priority flag (ASN.1 bit 5).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_PRIORITY(buf)                                     \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_PRIORITY)

/**
 * @brief Get phase flag (ASN.1 bit 6).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_PHASE(buf)                                        \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_PHASE)

/**
 * @brief Get reserved flag (ASN.1 bit 7).
 * @param[in] buf Pointer to TrafficLightOperationStatus UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_TRAFFIC_LIGHT_OPERATION_STATUS_GET_RESERVED(buf)                                     \
  J2735_INTERNAL_GET_ONE_TRAFFIC_LIGHT_OPERATION_STATUS(                                           \
      J2735_INTERNAL_RAW_READ_TRAFFIC_LIGHT_OPERATION_STATUS(buf),                                 \
      J2735_INTERNAL_BIT_TRAFFIC_LIGHT_OPERATION_STATUS_RESERVED)

#endif /* J2735_INTERNAL_DE_TRAFFICLIGHTOPERATIONSTATUS_H */
