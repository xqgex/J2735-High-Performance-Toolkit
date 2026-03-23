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
 * @brief J2735 UserSizeAndBehaviour Definition and Access Macros.
 *
 * @verbatim
 * UserSizeAndBehaviour ::= BIT STRING {
 *     unavailable (0),
 *     smallStature (1),
 *     largeStature (2),
 *     erraticMoving (3),
 *     slowMoving (4)
 * } (SIZE (5, ...))
 * @endverbatim
 *
 * Extensible BIT STRING with root size 5 and known extension size 5.
 *
 * @par Wire Format (non-extended, 6 bits total):
 * @verbatim
 * ┌───────┬──────────────────────────────────────────────────────┐
 * │ Bit 0 │ Bits 1-5                                             │
 * ├───────┼──────────────────────────────────────────────────────┤
 * │ Ext=0 │ flags[0..4] (5 bits)                                 │
 * └───────┴──────────────────────────────────────────────────────┘
 * @endverbatim
 *
 * @par Wire Format (extended, 13 bits total):
 * @verbatim
 * ┌───────┬────────────────────┬─────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7           │ Bits 8-12                       │
 * ├───────┼────────────────────┼─────────────────────────────────┤
 * │ Ext=1 │ nsnnwn=5 (7 bits)  │ flags[0..4] (5 bits)            │
 * └───────┴────────────────────┴─────────────────────────────────┘
 * @endverbatim
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 13 bits ≤ 56-bit READ_BITS limit.
 * We read all 13 bits in ONE call, then use bit arithmetic to extract:
 *   - Extension bit at position 12 (MSB of 13-bit value)
 *   - Flags at positions 0-4 (extended) or shifted for non-extended
 *
 * 13-bit read layout (left-justified from bit 0):
 *   Non-extended: [Ext=0][F0..F4][7 garbage bits]
 *                  bit12  11..7     6..0
 *   Extended:     [Ext=1][nsnnwn:7][F0..F4]
 *                  bit12  11..5    4..0
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_USERSIZEANDBEHAVIOUR_H
#define J2735_INTERNAL_DE_USERSIZEANDBEHAVIOUR_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of UserSizeAndBehaviour in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_BITS_USER_SIZE_AND_BEHAVIOUR 5U

/**
 * @internal
 * @brief Extended size of UserSizeAndBehaviour in bits.
 */
#define J2735_INTERNAL_EXT_SIZE_USER_SIZE_AND_BEHAVIOUR 5U

/**
 * @internal
 * @brief Maximum wire encoding size of UserSizeAndBehaviour in bits.
 *
 * Used for single-read optimization: read all possible bits at once.
 * Calculated as: 1 (ext bit) + 7 (nsnnwn) + 5U (max payload).
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR 13U

_Static_assert(J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR ==
                   (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_NSNNWN_SMALL_BITS +
                    J2735_INTERNAL_EXT_SIZE_USER_SIZE_AND_BEHAVIOUR),
               "MAX_WIRE_BITS must equal ext_marker + nsnnwn + ext_size");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only.                                                                      */
/*  Use the public J2735_USER_SIZE_AND_BEHAVIOUR_GET_*() accessors instead.                       */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_UNAVAILABLE    0U
#define J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_SMALL_STATURE  1U
#define J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_LARGE_STATURE  2U
#define J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_ERRATIC_MOVING 3U
#define J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_SLOW_MOVING    4U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 13 bits from buffer for UserSizeAndBehaviour.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for UserSizeAndBehaviour.
 * All other macros operate purely on the 13-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded UserSizeAndBehaviour (const uint8_t*).
 * @pre @p buf must point to at least 2 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the UserSizeAndBehaviour encoding.
 * @return 13-bit value as uint64_t with wire bits left-justified:
 *         - Bit 12: Extension marker (0=root, 1=extended)
 *         - Bits 11..0: Payload (format depends on extension state)
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf)                                       \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Check extension bit from pre-read 13-bit raw value.
 *
 * The extension bit is at position 12 (MSB of the 13-bit read):
 * - 0 = Root form: 5 flag bits follow immediately after extension bit
 * - 1 = Extended form: 7-bit nsnnwn (value=5) followed by 5 flag bits
 *
 * @param[in] raw13 Value previously returned by J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR().
 * @return Non-zero (true) if extended form, zero (false) if root form.
 * @note Internal use only. Use J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION() for public API.
 */
#define J2735_INTERNAL_IS_EXTENSION_USER_SIZE_AND_BEHAVIOUR(raw13)                                 \
  (((raw13) >> (J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR - 1U)) != 0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 13-bit raw value.
 *
 * Bit layout depends on extension state:
 *
 * Non-extended (bit 12 = 0):
 *   raw13: [0][F0][F1]...[F4][garbage:7]
 *           12 11  10      7     6..0
 *   Extract: (raw13 >> 7) & 0x1F -> 5 bits
 *
 * Extended (bit 12 = 1):
 *   raw13: [1][nsnnwn:7][F0][F1]...[F4]
 *           12  11.. 5    4   3      0
 *   Extract: raw13 & 0x1F -> 5 bits
 *
 * @param[in] raw13 Value previously returned by J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR().
 * @return Right-aligned flag bits as uint8_t:
 *         - 5 significant bits (0x00-0x1F) if non-extended
 *         - 5 significant bits (0x00-0x1F) if extended
 * @note Uses 1ULL for bit shifts to prevent undefined behavior if size >= 32 bits.
 *       The compiler optimizes the truncation to uint8_t.
 * @note Internal use only. Use J2735_USER_SIZE_AND_BEHAVIOUR_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_USER_SIZE_AND_BEHAVIOUR(raw13)                                      \
  (J2735_INTERNAL_IS_EXTENSION_USER_SIZE_AND_BEHAVIOUR(raw13) ? /* Extended: low 5 bits */         \
       ((uint8_t)((raw13) & ((1ULL << J2735_INTERNAL_EXT_SIZE_USER_SIZE_AND_BEHAVIOUR) - 1ULL)))   \
                                                              : /* Non-ext: bits 11..7 = 5 bits */ \
       ((uint8_t)(((raw13) >> (J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR - 1U -         \
                               J2735_INTERNAL_ROOT_SIZE_BITS_USER_SIZE_AND_BEHAVIOUR)) &           \
                  ((1ULL << J2735_INTERNAL_ROOT_SIZE_BITS_USER_SIZE_AND_BEHAVIOUR) - 1ULL))))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 13-bit raw value.
 *
 * Computes the bit position within raw13 based on extension state:
 *
 * Non-extended: Flag[N] is at raw13 bit (11 - N)
 *   F0 at bit 11, F1 at bit 10, ..., F4 at bit 7
 *   Formula: (raw13 >> (11 - bit_pos)) & 1
 *
 * Extended: Flag[N] is at raw13 bit (4 - N)
 *   F0 at bit 4, F1 at bit 3, ..., F4 at bit 0
 *   Formula: (raw13 >> (4 - bit_pos)) & 1
 *
 * @param[in] raw13   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR().
 * @param[in] bit_pos ASN.1 bit position (0-4). Use J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @warning For non-extended messages, bit_pos >= 5 reads undefined garbage bits.
 *          Caller should verify HAS_EXTENSION before accessing extension-only flags.
 * @note Internal use only. Use J2735_USER_SIZE_AND_BEHAVIOUR_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_USER_SIZE_AND_BEHAVIOUR(raw13, bit_pos)                             \
  ((uint8_t)(J2735_INTERNAL_IS_EXTENSION_USER_SIZE_AND_BEHAVIOUR(raw13)                            \
                 ? /* Extended: F0 at bit 4 */                                                     \
                 (((raw13) >>                                                                      \
                   ((J2735_INTERNAL_EXT_SIZE_USER_SIZE_AND_BEHAVIOUR - 1U) - (bit_pos))) &         \
                  1U)                                                                              \
                 : /* Non-ext: F0 at bit 11 */                                                     \
                 (((raw13) >>                                                                      \
                   ((J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR - 2U) - (bit_pos))) &    \
                  1U)))

/* ============================================================================================== */
/*  PUBLIC API: UserSizeAndBehaviour Accessors                                                    */
/* ============================================================================================== */
/**
 * @brief Check if UserSizeAndBehaviour has an extension.
 *
 * Extended form includes 5 flags (bits 0-4).
 * Root form has only 5 flags (bits 0-4).
 *
 * @param[in] buf Pointer to the start of the UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid UserSizeAndBehaviour encoding with +7 byte padding.
 * @return Non-zero (true) if extended (5 flags), zero (false) if root (5 flags).
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION(buf)                                           \
  J2735_INTERNAL_IS_EXTENSION_USER_SIZE_AND_BEHAVIOUR(                                             \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf))

/**
 * @brief Get wire size of UserSizeAndBehaviour in bits.
 *
 * Use this to advance a bit cursor past the UserSizeAndBehaviour field.
 *
 * @param[in] buf Pointer to the start of the UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid UserSizeAndBehaviour encoding with +7 byte padding.
 * @return 13U if extended form, 6U if root form.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_SIZE(buf)                                                    \
  (J2735_INTERNAL_IS_EXTENSION_USER_SIZE_AND_BEHAVIOUR(                                            \
       J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf))                                       \
       ? J2735_INTERNAL_MAX_WIRE_BITS_USER_SIZE_AND_BEHAVIOUR                                      \
       : (J2735_INTERNAL_EXTENSION_MARKER_BITS +                                                   \
          J2735_INTERNAL_ROOT_SIZE_BITS_USER_SIZE_AND_BEHAVIOUR))

/**
 * @brief Get all UserSizeAndBehaviour as a single uint8_t value.
 *
 * Returns all flag bits packed into a right-aligned integer.
 * The number of significant bits depends on extension state:
 * - Root form: bits 0-4 valid (5 bits), bit 5 is zero
 * - Extended form: bits 0-4 valid (5 bits)
 *
 * @param[in] buf Pointer to the start of the UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid UserSizeAndBehaviour encoding with +7 byte padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 * @note Use J2735_USER_SIZE_AND_BEHAVIOUR_HAS_EXTENSION() to determine if bit 5 is meaningful.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_GET(buf)                                                     \
  J2735_INTERNAL_GET_ALL_USER_SIZE_AND_BEHAVIOUR(                                                  \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf))

/**
 * @brief Get unavailable flag (ASN.1 bit 0).
 * @param[in] buf Pointer to UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_GET_UNAVAILABLE(buf)                                         \
  J2735_INTERNAL_GET_ONE_USER_SIZE_AND_BEHAVIOUR(                                                  \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf),                                        \
      J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_UNAVAILABLE)

/**
 * @brief Get smallStature flag (ASN.1 bit 1).
 * @param[in] buf Pointer to UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_GET_SMALL_STATURE(buf)                                       \
  J2735_INTERNAL_GET_ONE_USER_SIZE_AND_BEHAVIOUR(                                                  \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf),                                        \
      J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_SMALL_STATURE)

/**
 * @brief Get largeStature flag (ASN.1 bit 2).
 * @param[in] buf Pointer to UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_GET_LARGE_STATURE(buf)                                       \
  J2735_INTERNAL_GET_ONE_USER_SIZE_AND_BEHAVIOUR(                                                  \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf),                                        \
      J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_LARGE_STATURE)

/**
 * @brief Get erraticMoving flag (ASN.1 bit 3).
 * @param[in] buf Pointer to UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_GET_ERRATIC_MOVING(buf)                                      \
  J2735_INTERNAL_GET_ONE_USER_SIZE_AND_BEHAVIOUR(                                                  \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf),                                        \
      J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_ERRATIC_MOVING)

/**
 * @brief Get slowMoving flag (ASN.1 bit 4).
 * @param[in] buf Pointer to UserSizeAndBehaviour UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_USER_SIZE_AND_BEHAVIOUR_GET_SLOW_MOVING(buf)                                         \
  J2735_INTERNAL_GET_ONE_USER_SIZE_AND_BEHAVIOUR(                                                  \
      J2735_INTERNAL_RAW_READ_USER_SIZE_AND_BEHAVIOUR(buf),                                        \
      J2735_INTERNAL_BIT_USER_SIZE_AND_BEHAVIOUR_SLOW_MOVING)

#endif /* J2735_INTERNAL_DE_USERSIZEANDBEHAVIOUR_H */
