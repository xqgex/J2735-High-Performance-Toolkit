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
 * @brief J2735 PersonalDeviceUsageState Definition and Access Macros.
 *
 * @code
 * PersonalDeviceUsageState ::= BIT STRING {
 *     unavailable (0),
 *     other (1),
 *     idle (2),
 *     listeningToAudio (3),
 *     typing (4),
 *     calling (5),
 *     playingGames (6),
 *     reading (7),
 *     viewing (8)
 * } (SIZE (9, ...))
 * @endcode
 *
 * Extensible BIT STRING with root size 9 and known extension size 9.
 *
 * @par Wire Format (non-extended, 10 bits total):
 * @code
 * ┌───────┬──────────────────────────────────────────────────────┐
 * │ Bit 0 │ Bits 1-9                                             │
 * ├───────┼──────────────────────────────────────────────────────┤
 * │ Ext=0 │ flags[0..8] (9 bits)                                 │
 * └───────┴──────────────────────────────────────────────────────┘
 * @endcode
 *
 * @par Wire Format (extended, 17 bits total):
 * @code
 * ┌───────┬────────────────────┬─────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7           │ Bits 8-16                       │
 * ├───────┼────────────────────┼─────────────────────────────────┤
 * │ Ext=1 │ nsnnwn=9 (7 bits)  │ flags[0..8] (9 bits)            │
 * └───────┴────────────────────┴─────────────────────────────────┘
 * @endcode
 *
 * @par Optimization: Single-Read Strategy
 * @code
 * Max wire size = 17 bits ≤ 56-bit READ_BITS limit.
 * We read all 17 bits in ONE call, then use bit arithmetic to extract:
 *   - Extension bit at position 16 (MSB of 17-bit value)
 *   - Flags at positions 0-8 (extended) or shifted for non-extended
 *
 * 17-bit read layout (left-justified from bit 0):
 *   Non-extended: [Ext=0][F0..F8][7 garbage bits]
 *                  bit16  15..7     6..0
 *   Extended:     [Ext=1][nsnnwn:7][F0..F8]
 *                  bit16  15..9    8..0
 * @endcode
 */
#ifndef J2735_INTERNAL_DE_PERSONALDEVICEUSAGESTATE_H
#define J2735_INTERNAL_DE_PERSONALDEVICEUSAGESTATE_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of PersonalDeviceUsageState in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_PERSONAL_DEVICE_USAGE_STATE 9U

/**
 * @internal
 * @brief Extended size of PersonalDeviceUsageState in bits.
 */
#define J2735_INTERNAL_EXT_SIZE_PERSONAL_DEVICE_USAGE_STATE 9U

/**
 * @internal
 * @brief Maximum wire encoding size of PersonalDeviceUsageState in bits.
 *
 * Used for single-read optimization: read all possible bits at once.
 * Calculated as: 1 (ext bit) + 7 (nsnnwn) + 9U (max payload).
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE 17U

_Static_assert(J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE ==
                   (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_NSNNWN_SMALL_BITS +
                    J2735_INTERNAL_EXT_SIZE_PERSONAL_DEVICE_USAGE_STATE),
               "MAX_WIRE_BITS must equal ext_marker + nsnnwn + ext_size");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only. Use the public J2735_PERSONAL_DEVICE_USAGE_STATE_GET_*() accessors
 * instead.  */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_UNAVAILABLE        0U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_OTHER              1U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_IDLE               2U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_LISTENING_TO_AUDIO 3U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_TYPING             4U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_CALLING            5U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_PLAYING_GAMES      6U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_READING            7U
#define J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_VIEWING            8U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 17 bits from buffer for PersonalDeviceUsageState.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for PersonalDeviceUsageState.
 * All other macros operate purely on the 17-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded PersonalDeviceUsageState (const uint8_t*).
 * @pre @p buf must point to at least 3 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the PersonalDeviceUsageState encoding.
 * @return 17-bit value as uint64_t with wire bits left-justified:
 *         - Bit 16: Extension marker (0=root, 1=extended)
 *         - Bits 15..0: Payload (format depends on extension state)
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf)                                   \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Check extension bit from pre-read 17-bit raw value.
 *
 * The extension bit is at position 16 (MSB of the 17-bit read):
 * - 0 = Root form: 9 flag bits follow immediately after extension bit
 * - 1 = Extended form: 7-bit nsnnwn (value=9) followed by 9 flag bits
 *
 * @param[in] raw17 Value previously returned by
 * J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE().
 * @return Non-zero (true) if extended form, zero (false) if root form.
 * @note Internal use only. Use J2735_PERSONAL_DEVICE_USAGE_STATE_IS_EXTENDED() for public API.
 */
#define J2735_INTERNAL_IS_EXTENSION_PERSONAL_DEVICE_USAGE_STATE(raw17)                             \
  (((raw17) >> (J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE - 1U)) != 0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 17-bit raw value.
 *
 * Bit layout depends on extension state:
 *
 * Non-extended (bit 16 = 0):
 *   raw17: [0][F0][F1]...[F8][garbage:7]
 *           16 15  14      7     6..0
 *   Extract: (raw17 >> 7) & 0x1FF -> 9 bits
 *
 * Extended (bit 16 = 1):
 *   raw17: [1][nsnnwn:7][F0][F1]...[F8]
 *           16  15.. 9    8   7      0
 *   Extract: raw17 & 0x1FF -> 9 bits
 *
 * @param[in] raw17 Value previously returned by
 * J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE().
 * @return Right-aligned flag bits as uint16_t:
 *         - 9 significant bits (0x0000-0x01FF) if non-extended
 *         - 9 significant bits (0x0000-0x01FF) if extended
 * @note Uses 1ULL for bit shifts to prevent undefined behavior if size >= 32 bits.
 *       The compiler optimizes the truncation to uint16_t.
 * @note Internal use only. Use J2735_PERSONAL_DEVICE_USAGE_STATE_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_PERSONAL_DEVICE_USAGE_STATE(raw17)                                  \
  (J2735_INTERNAL_IS_EXTENSION_PERSONAL_DEVICE_USAGE_STATE(raw17)                                  \
       ? /* Extended: low 9 bits */                                                                \
       ((uint16_t)((raw17) &                                                                       \
                   ((1ULL << J2735_INTERNAL_EXT_SIZE_PERSONAL_DEVICE_USAGE_STATE) - 1ULL)))        \
       : /* Non-ext: bits 15..7 = 9 bits */                                                        \
       ((uint16_t)(((raw17) >> (J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE - 1U -    \
                                J2735_INTERNAL_ROOT_SIZE_PERSONAL_DEVICE_USAGE_STATE)) &           \
                   ((1ULL << J2735_INTERNAL_ROOT_SIZE_PERSONAL_DEVICE_USAGE_STATE) - 1ULL))))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 17-bit raw value.
 *
 * Computes the bit position within raw17 based on extension state:
 *
 * Non-extended: Flag[N] is at raw17 bit (15 - N)
 *   F0 at bit 15, F1 at bit 14, ..., F8 at bit 7
 *   Formula: (raw17 >> (15 - bit_pos)) & 1
 *
 * Extended: Flag[N] is at raw17 bit (8 - N)
 *   F0 at bit 8, F1 at bit 7, ..., F8 at bit 0
 *   Formula: (raw17 >> (8 - bit_pos)) & 1
 *
 * @param[in] raw17   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE().
 * @param[in] bit_pos ASN.1 bit position (0-8). Use J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @warning For non-extended messages, bit_pos >= 9 reads undefined garbage bits.
 *          Caller should verify IS_EXTENDED before accessing extension-only flags.
 * @note Internal use only. Use J2735_PERSONAL_DEVICE_USAGE_STATE_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(raw17, bit_pos)                         \
  ((uint8_t)(J2735_INTERNAL_IS_EXTENSION_PERSONAL_DEVICE_USAGE_STATE(raw17)                        \
                 ? /* Extended: F0 at bit 8 */                                                     \
                 (((raw17) >>                                                                      \
                   ((J2735_INTERNAL_EXT_SIZE_PERSONAL_DEVICE_USAGE_STATE - 1U) - (bit_pos))) &     \
                  1U)                                                                              \
                 : /* Non-ext: F0 at bit 15 */                                                     \
                 (((raw17) >> ((J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE - 2U) -   \
                               (bit_pos))) &                                                       \
                  1U)))

/* ============================================================================================== */
/*  PUBLIC API: PersonalDeviceUsageState Accessors                                                */
/* ============================================================================================== */
/**
 * @brief Check if PersonalDeviceUsageState is in extended form.
 *
 * Extended form includes 9 flags (bits 0-8).
 * Root form has only 9 flags (bits 0-8).
 *
 * @param[in] buf Pointer to the start of the PersonalDeviceUsageState UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid PersonalDeviceUsageState encoding with +7 byte padding.
 * @return Non-zero (true) if extended (9 flags), zero (false) if root (9 flags).
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_IS_EXTENDED(buf)                                         \
  J2735_INTERNAL_IS_EXTENSION_PERSONAL_DEVICE_USAGE_STATE(                                         \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf))

/**
 * @brief Get wire size of PersonalDeviceUsageState in bits.
 *
 * Use this to advance a bit cursor past the PersonalDeviceUsageState field.
 *
 * @param[in] buf Pointer to the start of the PersonalDeviceUsageState UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid PersonalDeviceUsageState encoding with +7 byte padding.
 * @return 17U if extended form, 10U if root form.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_SIZE(buf)                                                \
  (J2735_INTERNAL_IS_EXTENSION_PERSONAL_DEVICE_USAGE_STATE(                                        \
       J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf))                                   \
       ? J2735_INTERNAL_MAX_WIRE_BITS_PERSONAL_DEVICE_USAGE_STATE                                  \
       : (J2735_INTERNAL_EXTENSION_MARKER_BITS +                                                   \
          J2735_INTERNAL_ROOT_SIZE_PERSONAL_DEVICE_USAGE_STATE))

/**
 * @brief Get all PersonalDeviceUsageState as a single uint16_t value.
 *
 * Returns all flag bits packed into a right-aligned integer.
 * The number of significant bits depends on extension state:
 * - Root form: bits 0-8 valid (9 bits), bit 9 is zero
 * - Extended form: bits 0-8 valid (9 bits)
 *
 * @param[in] buf Pointer to the start of the PersonalDeviceUsageState UPER encoding (const
 * uint8_t*).
 * @pre @p buf must point to valid PersonalDeviceUsageState encoding with +7 byte padding.
 * @return Right-aligned flag value (uint16_t). Bit 0 of result = first named bit.
 * @note Use J2735_PERSONAL_DEVICE_USAGE_STATE_IS_EXTENDED() to determine if bit 9 is meaningful.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET(buf)                                                 \
  J2735_INTERNAL_GET_ALL_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf))

/**
 * @brief Get unavailable flag (ASN.1 bit 0).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_UNAVAILABLE(buf)                                     \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_UNAVAILABLE)

/**
 * @brief Get other flag (ASN.1 bit 1).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_OTHER(buf)                                           \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_OTHER)

/**
 * @brief Get idle flag (ASN.1 bit 2).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_IDLE(buf)                                            \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_IDLE)

/**
 * @brief Get listeningToAudio flag (ASN.1 bit 3).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_LISTENING_TO_AUDIO(buf)                              \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_LISTENING_TO_AUDIO)

/**
 * @brief Get typing flag (ASN.1 bit 4).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_TYPING(buf)                                          \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_TYPING)

/**
 * @brief Get calling flag (ASN.1 bit 5).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_CALLING(buf)                                         \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_CALLING)

/**
 * @brief Get playingGames flag (ASN.1 bit 6).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_PLAYING_GAMES(buf)                                   \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_PLAYING_GAMES)

/**
 * @brief Get reading flag (ASN.1 bit 7).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_READING(buf)                                         \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_READING)

/**
 * @brief Get viewing flag (ASN.1 bit 8).
 * @param[in] buf Pointer to PersonalDeviceUsageState UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PERSONAL_DEVICE_USAGE_STATE_GET_VIEWING(buf)                                         \
  J2735_INTERNAL_GET_ONE_PERSONAL_DEVICE_USAGE_STATE(                                              \
      J2735_INTERNAL_RAW_READ_PERSONAL_DEVICE_USAGE_STATE(buf),                                    \
      J2735_INTERNAL_BIT_PERSONAL_DEVICE_USAGE_STATE_VIEWING)

#endif /* J2735_INTERNAL_DE_PERSONALDEVICEUSAGESTATE_H */
