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
 * @brief J2735 PublicSafetyDirectingTrafficSubType Definition and Access Macros.
 *
 * PublicSafetyDirectingTrafficSubType ::= BIT STRING {
 *     unavailable (0),
 *     policeAndTrafficOfficers (1),
 *     trafficControlPersons (2),
 *     railroadCrossingGuards (3),
 *     civilDefenseNationalGuardMilitaryPolice (4),
 *     emergencyOrganizationPersonnel (5),
 *     highwayServiceVehiclePersonnel (6)
 * } (SIZE (7, ...))
 *
 * Extensible BIT STRING with root size 7 and known extension size 7.
 *
 * Wire Format (non-extended, 8 bits total):
 * ┌───────┬──────────────────────────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7                                             │
 * ├───────┼──────────────────────────────────────────────────────┤
 * │ Ext=0 │ flags[0..6] (7 bits)                                 │
 * └───────┴──────────────────────────────────────────────────────┘
 *
 * Wire Format (extended, 15 bits total):
 * ┌───────┬────────────────────┬─────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7           │ Bits 8-14                       │
 * ├───────┼────────────────────┼─────────────────────────────────┤
 * │ Ext=1 │ nsnnwn=7 (7 bits)  │ flags[0..6] (7 bits)            │
 * └───────┴────────────────────┴─────────────────────────────────┘
 *
 * Optimization: Single-Read Strategy
 * ──────────────────────────────────────────────────────────────────────────────────────────
 * Max wire size = 15 bits ≤ 56-bit READ_BITS limit.
 * We read all 15 bits in ONE call, then use bit arithmetic to extract:
 *   - Extension bit at position 14 (MSB of 15-bit value)
 *   - Flags at positions 0-6 (extended) or shifted for non-extended
 *
 * 15-bit read layout (left-justified from bit 0):
 *   Non-extended: [Ext=0][F0..F6][7 garbage bits]
 *                  bit14  13..7     6..0
 *   Extended:     [Ext=1][nsnnwn:7][F0..F6]
 *                  bit14  13..7    6..0
 */
#ifndef J2735_INTERNAL_DE_PUBLICSAFETYDIRECTINGTRAFFICSUBTYPE_H
#define J2735_INTERNAL_DE_PUBLICSAFETYDIRECTINGTRAFFICSUBTYPE_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of PublicSafetyDirectingTrafficSubType in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE 7U

/**
 * @internal
 * @brief Extended size of PublicSafetyDirectingTrafficSubType in bits.
 */
#define J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE 7U

/**
 * @internal
 * @brief Maximum wire encoding size of PublicSafetyDirectingTrafficSubType in bits.
 *
 * Used for single-read optimization: read all possible bits at once.
 * Calculated as: 1 (ext bit) + 7 (nsnnwn) + 7U (max payload).
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE 15U

_Static_assert(J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE ==
                   (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_NSNNWN_SMALL_BITS +
                    J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE),
               "MAX_WIRE_BITS must equal ext_marker + nsnnwn + ext_size");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only. Use the public J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_*()
 * accessors instead.  */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_UNAVAILABLE                 0U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_POLICE_AND_TRAFFIC_OFFICERS 1U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_TRAFFIC_CONTROL_PERSONS     2U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_RAILROAD_CROSSING_GUARDS    3U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_CIVIL_DEFENSE_NATIONAL_GUARD_MILITARY_POLICE \
  4U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_EMERGENCY_ORGANIZATION_PERSONNEL \
  5U
#define J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_HIGHWAY_SERVICE_VEHICLE_PERSONNEL \
  6U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 15 bits from buffer for PublicSafetyDirectingTrafficSubType.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for
 * PublicSafetyDirectingTrafficSubType. All other macros operate purely on the 15-bit value returned
 * by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded PublicSafetyDirectingTrafficSubType (const uint8_t*).
 * @pre @p buf must point to at least 2 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the PublicSafetyDirectingTrafficSubType encoding.
 * @return 15-bit value as uint64_t with wire bits left-justified:
 *         - Bit 14: Extension marker (0=root, 1=extended)
 *         - Bits 13..0: Payload (format depends on extension state)
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf)                      \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Check extension bit from pre-read 15-bit raw value.
 *
 * The extension bit is at position 14 (MSB of the 15-bit read):
 * - 0 = Root form: 7 flag bits follow immediately after extension bit
 * - 1 = Extended form: 7-bit nsnnwn (value=7) followed by 7 flag bits
 *
 * @param[in] raw15 Value previously returned by
 * J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE().
 * @return Non-zero (true) if extended form, zero (false) if root form.
 * @note Internal use only. Use J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED() for
 * public API.
 */
#define J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(raw15)                \
  (((raw15) >> (J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE - 1U)) != 0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 15-bit raw value.
 *
 * Bit layout depends on extension state:
 *
 * Non-extended (bit 14 = 0):
 *   raw15: [0][F0][F1]...[F6][garbage:7]
 *           14 13  12      7     6..0
 *   Extract: (raw15 >> 7) & 0x7F -> 7 bits
 *
 * Extended (bit 14 = 1):
 *   raw15: [1][nsnnwn:7][F0][F1]...[F6]
 *           14  13.. 7    6   5      0
 *   Extract: raw15 & 0x7F -> 7 bits
 *
 * @param[in] raw15 Value previously returned by
 * J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE().
 * @return Right-aligned flag bits as uint8_t:
 *         - 7 significant bits (0x0000-0x007F) if non-extended
 *         - 7 significant bits (0x0000-0x007F) if extended
 * @note Uses 1ULL for bit shifts to prevent undefined behavior if size >= 32 bits.
 *       The compiler optimizes the truncation to uint8_t.
 * @note Internal use only. Use J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(raw15)                     \
  (J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(raw15)                     \
       ? /* Extended: low 7 bits */                                                                \
       ((uint8_t)((raw15) &                                                                        \
                  ((1ULL << J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE) -    \
                   1ULL)))                                                                         \
       : /* Non-ext: bits 13..7 = 7 bits */                                                        \
       ((uint8_t)(((raw15) >>                                                                      \
                   (J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE - 1U -   \
                    J2735_INTERNAL_ROOT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE)) &          \
                  ((1ULL << J2735_INTERNAL_ROOT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE) -   \
                   1ULL))))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 15-bit raw value.
 *
 * Computes the bit position within raw15 based on extension state:
 *
 * Non-extended: Flag[N] is at raw15 bit (13 - N)
 *   F0 at bit 13, F1 at bit 12, ..., F6 at bit 7
 *   Formula: (raw15 >> (13 - bit_pos)) & 1
 *
 * Extended: Flag[N] is at raw15 bit (6 - N)
 *   F0 at bit 6, F1 at bit 5, ..., F6 at bit 0
 *   Formula: (raw15 >> (6 - bit_pos)) & 1
 *
 * @param[in] raw15   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE().
 * @param[in] bit_pos ASN.1 bit position (0-6). Use
 * J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_* constants.
 * @return 0 or 1 as uint8_t.
 * @warning For non-extended messages, bit_pos >= 7 reads undefined garbage bits.
 *          Caller should verify IS_EXTENDED before accessing extension-only flags.
 * @note Internal use only. Use J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_*() accessors for
 * public API.
 */
#define J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(raw15, bit_pos)            \
  ((uint8_t)(J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(raw15)           \
                 ? /* Extended: F0 at bit 6 */                                                     \
                 (((raw15) >>                                                                      \
                   ((J2735_INTERNAL_EXT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE - 1U) -      \
                    (bit_pos))) &                                                                  \
                  1U)                                                                              \
                 : /* Non-ext: F0 at bit 13 */                                                     \
                 (((raw15) >>                                                                      \
                   ((J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE - 2U) - \
                    (bit_pos))) &                                                                  \
                  1U)))

/* ============================================================================================== */
/*  PUBLIC API: PublicSafetyDirectingTrafficSubType Accessors                                     */
/* ============================================================================================== */
/**
 * @brief Check if PublicSafetyDirectingTrafficSubType is in extended form.
 *
 * Extended form includes 7 flags (bits 0-6).
 * Root form has only 7 flags (bits 0-6).
 *
 * @param[in] buf Pointer to the start of the PublicSafetyDirectingTrafficSubType UPER encoding
 * (const uint8_t*).
 * @pre @p buf must point to valid PublicSafetyDirectingTrafficSubType encoding with +7 byte
 * padding.
 * @return Non-zero (true) if extended (7 flags), zero (false) if root (7 flags).
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED(buf)                            \
  J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                            \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf))

/**
 * @brief Get wire size of PublicSafetyDirectingTrafficSubType in bits.
 *
 * Use this to advance a bit cursor past the PublicSafetyDirectingTrafficSubType field.
 *
 * @param[in] buf Pointer to the start of the PublicSafetyDirectingTrafficSubType UPER encoding
 * (const uint8_t*).
 * @pre @p buf must point to valid PublicSafetyDirectingTrafficSubType encoding with +7 byte
 * padding.
 * @return 15U if extended form, 8U if root form.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_SIZE(buf)                                   \
  (J2735_INTERNAL_IS_EXTENSION_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                           \
       J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf))                      \
       ? J2735_INTERNAL_MAX_WIRE_BITS_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE                     \
       : (J2735_INTERNAL_EXTENSION_MARKER_BITS +                                                   \
          J2735_INTERNAL_ROOT_SIZE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE))

/**
 * @brief Get all PublicSafetyDirectingTrafficSubType as a single uint8_t value.
 *
 * Returns all flag bits packed into a right-aligned integer.
 * The number of significant bits depends on extension state:
 * - Root form: bits 0-6 valid (7 bits), bit 7 is zero
 * - Extended form: bits 0-6 valid (7 bits)
 *
 * @param[in] buf Pointer to the start of the PublicSafetyDirectingTrafficSubType UPER encoding
 * (const uint8_t*).
 * @pre @p buf must point to valid PublicSafetyDirectingTrafficSubType encoding with +7 byte
 * padding.
 * @return Right-aligned flag value (uint8_t). Bit 0 of result = first named bit.
 * @note Use J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_IS_EXTENDED() to determine if bit 7 is
 * meaningful.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET(buf)                                    \
  J2735_INTERNAL_GET_ALL_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf))

/**
 * @brief Get unavailable flag (ASN.1 bit 0).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_UNAVAILABLE(buf)                        \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                       \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_UNAVAILABLE)

/**
 * @brief Get policeAndTrafficOfficers flag (ASN.1 bit 1).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_POLICE_AND_TRAFFIC_OFFICERS(buf)        \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                       \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_POLICE_AND_TRAFFIC_OFFICERS)

/**
 * @brief Get trafficControlPersons flag (ASN.1 bit 2).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_TRAFFIC_CONTROL_PERSONS(buf)            \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                       \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_TRAFFIC_CONTROL_PERSONS)

/**
 * @brief Get railroadCrossingGuards flag (ASN.1 bit 3).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_RAILROAD_CROSSING_GUARDS(buf)           \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                       \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_RAILROAD_CROSSING_GUARDS)

/**
 * @brief Get civilDefenseNationalGuardMilitaryPolice flag (ASN.1 bit 4).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_CIVIL_DEFENSE_NATIONAL_GUARD_MILITARY_POLICE( \
    buf)                                                                                                 \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                       \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                             \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_CIVIL_DEFENSE_NATIONAL_GUARD_MILITARY_POLICE)

/**
 * @brief Get emergencyOrganizationPersonnel flag (ASN.1 bit 5).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_EMERGENCY_ORGANIZATION_PERSONNEL(buf)   \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                       \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_EMERGENCY_ORGANIZATION_PERSONNEL)

/**
 * @brief Get highwayServiceVehiclePersonnel flag (ASN.1 bit 6).
 * @param[in] buf Pointer to PublicSafetyDirectingTrafficSubType UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_GET_HIGHWAY_SERVICE_VEHICLE_PERSONNEL(buf)  \
  J2735_INTERNAL_GET_ONE_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(                                 \
      J2735_INTERNAL_RAW_READ_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE(buf),                       \
      J2735_INTERNAL_BIT_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE_HIGHWAY_SERVICE_VEHICLE_PERSONNEL)

#endif /* J2735_INTERNAL_DE_PUBLICSAFETYDIRECTINGTRAFFICSUBTYPE_H */
