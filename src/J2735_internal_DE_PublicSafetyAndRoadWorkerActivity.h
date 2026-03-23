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
 * @brief J2735 PublicSafetyAndRoadWorkerActivity Definition and Access Macros.
 *
 * @verbatim
 * PublicSafetyAndRoadWorkerActivity ::= BIT STRING {
 *     unavailable (0),
 *     workingOnRoad (1),
 *     settingUpClosures (2),
 *     respondingToEvents (3),
 *     directingTraffic (4),
 *     otherActivities (5)
 * } (SIZE (6, ...))
 * @endverbatim
 *
 * Extensible BIT STRING with root size 6 and known extension size 6.
 *
 * @par Wire Format (non-extended, 7 bits total):
 * @verbatim
 * ┌───────┬──────────────────────────────────────────────────────┐
 * │ Bit 0 │ Bits 1-6                                             │
 * ├───────┼──────────────────────────────────────────────────────┤
 * │ Ext=0 │ flags[0..5] (6 bits)                                 │
 * └───────┴──────────────────────────────────────────────────────┘
 * @endverbatim
 *
 * @par Wire Format (extended, 14 bits total):
 * @verbatim
 * ┌───────┬────────────────────┬─────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7           │ Bits 8-13                       │
 * ├───────┼────────────────────┼─────────────────────────────────┤
 * │ Ext=1 │ nsnnwn=6 (7 bits)  │ flags[0..5] (6 bits)            │
 * └───────┴────────────────────┴─────────────────────────────────┘
 * @endverbatim
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 14 bits ≤ 56-bit READ_BITS limit.
 * We read all 14 bits in ONE call, then use bit arithmetic to extract:
 *   - Extension bit at position 13 (MSB of 14-bit value)
 *   - Flags at positions 0-5 (extended) or shifted for non-extended
 *
 * 14-bit read layout (left-justified from bit 0):
 *   Non-extended: [Ext=0][F0..F5][7 garbage bits]
 *                  bit13  12..7     6..0
 *   Extended:     [Ext=1][nsnnwn:7][F0..F5]
 *                  bit13  12..6    5..0
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_PUBLICSAFETYANDROADWORKERACTIVITY_H
#define J2735_INTERNAL_DE_PUBLICSAFETYANDROADWORKERACTIVITY_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of PublicSafetyAndRoadWorkerActivity in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY 6U

/**
 * @internal
 * @brief Extended size of PublicSafetyAndRoadWorkerActivity in bits.
 */
#define J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY 6U

/**
 * @internal
 * @brief Maximum wire encoding size of PublicSafetyAndRoadWorkerActivity in bits.
 *
 * Used for single-read optimization: read all possible bits at once.
 * Calculated as: 1 (ext bit) + 7 (nsnnwn) + 6U (max payload).
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY 14U

_Static_assert(J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY ==
                   (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_NSNNWN_SMALL_BITS +
                    J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY),
               "MAX_WIRE_BITS must equal ext_marker + nsnnwn + ext_size");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only.                                                                      */
/*  Use the public J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_*() accessors instead.        */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_UNAVAILABLE          0U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_WORKING_ON_ROAD      1U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SETTING_UP_CLOSURES  2U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_RESPONDING_TO_EVENTS 3U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_DIRECTING_TRAFFIC    4U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_OTHER_ACTIVITIES     5U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 14 bits from buffer for PublicSafetyAndRoadWorkerActivity.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for
 * PublicSafetyAndRoadWorkerActivity. All other macros operate purely on the 14-bit value returned
 * by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded PublicSafetyAndRoadWorkerActivity (const uint8_t*).
 * @pre @p buf must point to at least 2 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the PublicSafetyAndRoadWorkerActivity encoding.
 * @return 14-bit value as uint64_t with wire bits left-justified:
 *         - Bit 13: Extension marker (0=root, 1=extended)
 *         - Bits 12..0: Payload (format depends on extension state)
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf)                        \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Check extension bit from pre-read 14-bit raw value.
 *
 * The extension bit is at position 13 (MSB of the 14-bit read):
 * - 0 = Root form: 6 flag bits follow immediately after extension bit
 * - 1 = Extended form: 7-bit nsnnwn (value=6) followed by 6 flag bits
 *
 * @param[in] raw14 Value previously returned by
 * J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY().
 * @return Non-zero (true) if extended form, zero (false) if root form.
 * @note Internal use only. Use J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION() for
 * public API.
 */
#define J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(raw14)                  \
  (((raw14) >> (J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY - 1U)) != 0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 14-bit raw value.
 *
 * Bit layout depends on extension state:
 *
 * Non-extended (bit 13 = 0):
 *   raw14: [0][F0][F1]...[F5][garbage:7]
 *           13 12  11      7     6..0
 *   Extract: (raw14 >> 7) & 0x3F -> 6 bits
 *
 * Extended (bit 13 = 1):
 *   raw14: [1][nsnnwn:7][F0][F1]...[F5]
 *           13  12.. 6    5   4      0
 *   Extract: raw14 & 0x3F -> 6 bits
 *
 * @param[in] raw14 Value previously returned by
 * J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY().
 * @return Right-aligned flag bits as uint8_t:
 *         - 6 significant bits (0x00-0x3F) if non-extended
 *         - 6 significant bits (0x00-0x3F) if extended
 * @note Uses 1ULL for bit shifts to prevent undefined behavior if size >= 32 bits.
 *       The compiler optimizes the truncation to uint8_t.
 * @note Internal use only. Use J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(raw14)                       \
  (J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(raw14)                       \
       ? /* Extended: low 6 bits */                                                                \
       ((uint8_t)((raw14) &                                                                        \
                  ((1ULL << J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY) -      \
                   1ULL)))                                                                         \
       : /* Non-ext: bits 12..7 = 6 bits */                                                        \
       ((uint8_t)(((raw14) >>                                                                      \
                   (J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY - 1U -     \
                    J2735_INTERNAL_ROOT_SIZE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY)) &       \
                  ((1ULL                                                                           \
                    << J2735_INTERNAL_ROOT_SIZE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY) -     \
                   1ULL))))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 14-bit raw value.
 *
 * Computes the bit position within raw14 based on extension state:
 *
 * Non-extended: Flag[N] is at raw14 bit (12 - N)
 *   F0 at bit 12, F1 at bit 11, ..., F5 at bit 7
 *   Formula: (raw14 >> (12 - bit_pos)) & 1
 *
 * Extended: Flag[N] is at raw14 bit (5 - N)
 *   F0 at bit 5, F1 at bit 4, ..., F5 at bit 0
 *   Formula: (raw14 >> (5 - bit_pos)) & 1
 *
 * @param[in] raw14   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY().
 * @param[in] bit_pos ASN.1 bit position (0-5). Use
 * J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_* constants.
 * @return 0 or 1 as uint8_t.
 * @warning For non-extended messages, bit_pos >= 6 reads undefined garbage bits.
 *          Caller should verify HAS_EXTENSION before accessing extension-only flags.
 * @note Internal use only. Use J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_*() accessors for
 * public API.
 */
#define J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(raw14, bit_pos)              \
  ((uint8_t)(J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(raw14)             \
                 ? /* Extended: F0 at bit 5 */                                                     \
                 (((raw14) >>                                                                      \
                   ((J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY - 1U) -        \
                    (bit_pos))) &                                                                  \
                  1U)                                                                              \
                 : /* Non-ext: F0 at bit 12 */                                                     \
                 (((raw14) >>                                                                      \
                   ((J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY - 2U) -   \
                    (bit_pos))) &                                                                  \
                  1U)))

/* ============================================================================================== */
/*  PUBLIC API: PublicSafetyAndRoadWorkerActivity Accessors                                       */
/* ============================================================================================== */
/**
 * @brief Check if PublicSafetyAndRoadWorkerActivity has an extension.
 *
 * Extended form includes 6 flags (bits 0-5).
 * Root form has only 6 flags (bits 0-5).
 *
 * @param[in] buf Pointer to the start of the PublicSafetyAndRoadWorkerActivity UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid PublicSafetyAndRoadWorkerActivity encoding with +7 byte padding.
 * @return Non-zero (true) if extended (6 flags), zero (false) if root (6 flags).
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION(buf)                            \
  J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                              \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf))

/**
 * @brief Get wire size of PublicSafetyAndRoadWorkerActivity in bits.
 *
 * Use this to advance a bit cursor past the PublicSafetyAndRoadWorkerActivity field.
 *
 * @param[in] buf Pointer to the start of the PublicSafetyAndRoadWorkerActivity UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid PublicSafetyAndRoadWorkerActivity encoding with +7 byte padding.
 * @return 14U if extended form, 7U if root form.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SIZE(buf)                                     \
  (J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                             \
       J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf))                        \
       ? J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY                       \
       : (J2735_INTERNAL_EXTENSION_MARKER_BITS +                                                   \
          J2735_INTERNAL_ROOT_SIZE_BITS_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY))

/**
 * @brief Get all PublicSafetyAndRoadWorkerActivity as a single uint8_t value.
 *
 * Returns all flag bits packed into a right-aligned integer.
 * The number of significant bits depends on extension state:
 * - Root form: bits 0-5 valid (6 bits), bit 6 is zero
 * - Extended form: bits 0-5 valid (6 bits)
 *
 * @param[in] buf Pointer to the start of the PublicSafetyAndRoadWorkerActivity UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid PublicSafetyAndRoadWorkerActivity encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 * @note Use J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_HAS_EXTENSION() to determine if bit 6 is
 * meaningful.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET(buf)                                      \
  J2735_INTERNAL_GET_ALL_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf))

/**
 * @brief Get unavailable flag (ASN.1 bit 0).
 * @param[in] buf Pointer to PublicSafetyAndRoadWorkerActivity UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_UNAVAILABLE(buf)                          \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf),                         \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_UNAVAILABLE)

/**
 * @brief Get workingOnRoad flag (ASN.1 bit 1).
 * @param[in] buf Pointer to PublicSafetyAndRoadWorkerActivity UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_WORKING_ON_ROAD(buf)                      \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf),                         \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_WORKING_ON_ROAD)

/**
 * @brief Get settingUpClosures flag (ASN.1 bit 2).
 * @param[in] buf Pointer to PublicSafetyAndRoadWorkerActivity UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_SETTING_UP_CLOSURES(buf)                  \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf),                         \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_SETTING_UP_CLOSURES)

/**
 * @brief Get respondingToEvents flag (ASN.1 bit 3).
 * @param[in] buf Pointer to PublicSafetyAndRoadWorkerActivity UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_RESPONDING_TO_EVENTS(buf)                 \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf),                         \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_RESPONDING_TO_EVENTS)

/**
 * @brief Get directingTraffic flag (ASN.1 bit 4).
 * @param[in] buf Pointer to PublicSafetyAndRoadWorkerActivity UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_DIRECTING_TRAFFIC(buf)                    \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf),                         \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_DIRECTING_TRAFFIC)

/**
 * @brief Get otherActivities flag (ASN.1 bit 5).
 * @param[in] buf Pointer to PublicSafetyAndRoadWorkerActivity UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_GET_OTHER_ACTIVITIES(buf)                     \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(                                   \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY(buf),                         \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY_OTHER_ACTIVITIES)

#endif /* J2735_INTERNAL_DE_PUBLICSAFETYANDROADWORKERACTIVITY_H */
