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
 * @brief J2735 VehicleEventFlags Definition and Access Macros.
 *
 * @verbatim
 * VehicleEventFlags ::= BIT STRING {
 *     eventHazardLights (0),
 *     eventStopLineViolation (1),
 *     eventABSactivated (2),
 *     eventTractionControlLoss (3),
 *     eventStabilityControlactivated (4),
 *     eventHazardousMaterials (5),
 *     eventReserved1 (6),
 *     eventHardBraking (7),
 *     eventLightsChanged (8),
 *     eventWipersChanged (9),
 *     eventFlatTire (10),
 *     eventDisabledVehicle (11),
 *     eventAirBagDeployment (12),
 *     eventJackKnife (13)
 * } (SIZE (13, ..., 14))
 * @endverbatim
 *
 * Extensible BIT STRING with root size 13 and known extension size 14.
 *
 * @par Wire Format (non-extended, 14 bits total):
 * @code
 * ┌───────┬──────────────────────────────────────────────────────┐
 * │ Bit 0 │ Bits 1-13                                            │
 * ├───────┼──────────────────────────────────────────────────────┤
 * │ Ext=0 │ flags[0..12] (13 bits)                               │
 * └───────┴──────────────────────────────────────────────────────┘
 * @endcode
 *
 * @par Wire Format (extended, 22 bits total):
 * @code
 * ┌───────┬────────────────────┬─────────────────────────────────┐
 * │ Bit 0 │ Bits 1-7           │ Bits 8-21                       │
 * ├───────┼────────────────────┼─────────────────────────────────┤
 * │ Ext=1 │ nsnnwn=14 (7 bits) │ flags[0..13] (14 bits)          │
 * └───────┴────────────────────┴─────────────────────────────────┘
 * @endcode
 *
 * @par Optimization: Single-Read Strategy
 * @verbatim
 * Max wire size = 22 bits ≤ 56-bit READ_BITS limit.
 * We read all 22 bits in ONE call, then use bit arithmetic to extract:
 *   - Extension bit at position 21 (MSB of 22-bit value)
 *   - Flags at positions 0-13 (extended) or shifted for non-extended
 *
 * 22-bit read layout (left-justified from bit 0):
 *   Non-extended: [Ext=0][F0..F12][8 garbage bits]
 *                  bit21  20..8     7..0
 *   Extended:     [Ext=1][nsnnwn:7][F0..F13]
 *                  bit21  20..14    13..0
 * @endverbatim
 */
#ifndef J2735_INTERNAL_DE_VEHICLEEVENTFLAGS_H
#define J2735_INTERNAL_DE_VEHICLEEVENTFLAGS_H

#include "J2735_internal_common.h"

/* ============================================================================================== */
/*  Constants                                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Root size of VehicleEventFlags in bits.
 */
#define J2735_INTERNAL_ROOT_SIZE_VEHICLE_EVENT_FLAGS 13U

/**
 * @internal
 * @brief Extended size of VehicleEventFlags in bits.
 */
#define J2735_INTERNAL_EXT_SIZE_VEHICLE_EVENT_FLAGS 14U

/**
 * @internal
 * @brief Maximum wire encoding size of VehicleEventFlags in bits.
 *
 * Used for single-read optimization: read all possible bits at once.
 * Calculated as: 1 (ext bit) + 7 (nsnnwn) + 14U (max payload).
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS 22U

_Static_assert(J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS ==
                   (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_NSNNWN_SMALL_BITS +
                    J2735_INTERNAL_EXT_SIZE_VEHICLE_EVENT_FLAGS),
               "MAX_WIRE_BITS must equal ext_marker + nsnnwn + ext_size");

/* ============================================================================================== */
/*  INTERNAL: Bit Position Constants                                                              */
/*                                                                                                */
/*  ASN.1 BIT STRING numbering convention: bit 0 = MSB (leftmost in wire order).                  */
/*  These constants map semantic flag names to their ASN.1 bit positions.                         */
/*                                                                                                */
/*  @note Internal use only. Use the public J2735_VEHICLE_EVENT_FLAGS_GET_*() accessors instead.  */
/* ============================================================================================== */
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HAZARD_LIGHTS              0U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_STOP_LINE_VIOLATION        1U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_ABS_ACTIVATED              2U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_TRACTION_CONTROL_LOSS      3U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_STABILITY_CONTROLACTIVATED 4U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HAZARDOUS_MATERIALS        5U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_RESERVED_1                 6U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HARD_BRAKING               7U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_LIGHTS_CHANGED             8U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_WIPERS_CHANGED             9U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_FLAT_TIRE                  10U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_DISABLED_VEHICLE           11U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_AIR_BAG_DEPLOYMENT         12U
#define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_JACK_KNIFE                 13U

/* ============================================================================================== */
/*  INTERNAL: Raw Buffer Read                                                                     */
/* ============================================================================================== */
/**
 * @internal
 * @brief Read 22 bits from buffer for VehicleEventFlags.
 *
 * This is the ONLY macro that performs I/O (calls J2735_READ_BITS) for VehicleEventFlags.
 * All other macros operate purely on the 22-bit value returned by this macro.
 *
 * @param[in] buf Pointer to UPER-encoded VehicleEventFlags (const uint8_t*).
 * @pre @p buf must point to at least 3 bytes of readable memory.
 * @pre Buffer should include +7 bytes padding beyond data for J2735_READ_BITS safety.
 * @pre @p buf must be aligned to the start of the VehicleEventFlags encoding.
 * @return 22-bit value as uint64_t with wire bits left-justified:
 *         - Bit 21: Extension marker (0=root, 1=extended)
 *         - Bits 20..0: Payload (format depends on extension state)
 * @note Internal use only. Not part of the public API.
 */
#define J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf)                                           \
  J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS)

/* ============================================================================================== */
/*  INTERNAL: Extension Bit Check                                                                 */
/* ============================================================================================== */
/**
 * @internal
 * @brief Check extension bit from pre-read 22-bit raw value.
 *
 * The extension bit is at position 21 (MSB of the 22-bit read):
 * - 0 = Root form: 13 flag bits follow immediately after extension bit
 * - 1 = Extended form: 7-bit nsnnwn (value=14) followed by 14 flag bits
 *
 * @param[in] raw22 Value previously returned by J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS().
 * @return Non-zero (true) if extended form, zero (false) if root form.
 * @note Internal use only. Use J2735_VEHICLE_EVENT_FLAGS_IS_EXTENDED() for public API.
 */
#define J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS(raw22)                                     \
  (((raw22) >> (J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS - 1U)) != 0U)

/* ============================================================================================== */
/*  INTERNAL: Extract Flags                                                                       */
/* ============================================================================================== */
/**
 * @internal
 * @brief Extract all flag bits from pre-read 22-bit raw value.
 *
 * Bit layout depends on extension state:
 *
 * Non-extended (bit 21 = 0):
 *   raw22: [0][F0][F1]...[F12][garbage:8]
 *           21 20  19      8     7..0
 *   Extract: (raw22 >> 8) & 0x1FFF -> 13 bits
 *
 * Extended (bit 21 = 1):
 *   raw22: [1][nsnnwn:7][F0][F1]...[F13]
 *           21  20..14   13  12      0
 *   Extract: raw22 & 0x3FFF -> 14 bits
 *
 * @param[in] raw22 Value previously returned by J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS().
 * @return Right-aligned flag bits as uint16_t:
 *         - 13 significant bits (0x0000-0x1FFF) if non-extended
 *         - 14 significant bits (0x0000-0x3FFF) if extended
 * @note Uses 1ULL for bit shifts to prevent undefined behavior if size >= 32 bits.
 *       The compiler optimizes the truncation to uint16_t.
 * @note Internal use only. Use J2735_VEHICLE_EVENT_FLAGS_GET() for public API.
 */
#define J2735_INTERNAL_GET_ALL_VEHICLE_EVENT_FLAGS(raw22)                                          \
  (J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS(raw22) ? /* Extended: low 14 bits */            \
       ((uint16_t)((raw22) & ((1ULL << J2735_INTERNAL_EXT_SIZE_VEHICLE_EVENT_FLAGS) - 1ULL)))      \
                                                          : /* Non-ext: bits 20..8 = 13 bits */    \
       ((uint16_t)(((raw22) >> (J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS - 1U -            \
                                J2735_INTERNAL_ROOT_SIZE_VEHICLE_EVENT_FLAGS)) &                   \
                   ((1ULL << J2735_INTERNAL_ROOT_SIZE_VEHICLE_EVENT_FLAGS) - 1ULL))))

/**
 * @internal
 * @brief Extract a single flag bit from pre-read 22-bit raw value.
 *
 * Computes the bit position within raw22 based on extension state:
 *
 * Non-extended: Flag[N] is at raw22 bit (20 - N)
 *   F0 at bit 20, F1 at bit 19, ..., F12 at bit 8
 *   Formula: (raw22 >> (20 - bit_pos)) & 1
 *
 * Extended: Flag[N] is at raw22 bit (13 - N)
 *   F0 at bit 13, F1 at bit 12, ..., F13 at bit 0
 *   Formula: (raw22 >> (13 - bit_pos)) & 1
 *
 * @param[in] raw22   Value previously returned by
 *                    J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS().
 * @param[in] bit_pos ASN.1 bit position (0-13). Use J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_*
 *                    constants.
 * @return 0 or 1 as uint8_t.
 * @warning For non-extended messages, bit_pos >= 13 reads undefined garbage bits.
 *          Caller should verify IS_EXTENDED before accessing extension-only flags.
 * @note Internal use only. Use J2735_VEHICLE_EVENT_FLAGS_GET_*() accessors for public API.
 */
#define J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(raw22, bit_pos)                                 \
  ((uint8_t)(J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS(raw22) ? /* Extended: F0 at bit 13 */ \
                 (((raw22) >> ((J2735_INTERNAL_EXT_SIZE_VEHICLE_EVENT_FLAGS - 1U) - (bit_pos))) &  \
                  1U)                                                                              \
                                                                    : /* Non-ext: F0 at bit 20 */  \
                 (((raw22) >>                                                                      \
                   ((J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS - 2U) - (bit_pos))) &        \
                  1U)))

/* ============================================================================================== */
/*  PUBLIC API: VehicleEventFlags Accessors                                                       */
/* ============================================================================================== */
/**
 * @brief Check if VehicleEventFlags is in extended form.
 *
 * Extended form includes 14 flags (bits 0-13).
 * Root form has only 13 flags (bits 0-12).
 *
 * @param[in] buf Pointer to the start of the VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid VehicleEventFlags encoding with +7 byte padding.
 * @return Non-zero (true) if extended (14 flags), zero (false) if root (13 flags).
 */
#define J2735_VEHICLE_EVENT_FLAGS_IS_EXTENDED(buf)                                                 \
  J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS(J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf))

/**
 * @brief Get wire size of VehicleEventFlags in bits.
 *
 * Use this to advance a bit cursor past the VehicleEventFlags field.
 *
 * @param[in] buf Pointer to the start of the VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid VehicleEventFlags encoding with +7 byte padding.
 * @return 22U if extended form, 14U if root form.
 */
#define J2735_VEHICLE_EVENT_FLAGS_SIZE(buf)                                                        \
  (J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS(                                                \
       J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf))                                           \
       ? J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS                                          \
       : (J2735_INTERNAL_EXTENSION_MARKER_BITS + J2735_INTERNAL_ROOT_SIZE_VEHICLE_EVENT_FLAGS))

/**
 * @brief Get all VehicleEventFlags as a single uint16_t value.
 *
 * Returns all flag bits packed into a right-aligned integer.
 * The number of significant bits depends on extension state:
 * - Root form: bits 0-12 valid (13 bits), bit 13 is zero
 * - Extended form: bits 0-13 valid (14 bits)
 *
 * @param[in] buf Pointer to the start of the VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid VehicleEventFlags encoding with +7 byte padding.
 * @return Right-aligned flag value (uint16_t). Bit 0 of result = first named bit.
 * @note Use J2735_VEHICLE_EVENT_FLAGS_IS_EXTENDED() to determine if bit 13 is meaningful.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET(buf)                                                         \
  J2735_INTERNAL_GET_ALL_VEHICLE_EVENT_FLAGS(J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf))

/**
 * @brief Get eventHazardLights flag (ASN.1 bit 0).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS(buf)                                     \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HAZARD_LIGHTS)

/**
 * @brief Get eventStopLineViolation flag (ASN.1 bit 1).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STOP_LINE_VIOLATION(buf)                               \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_STOP_LINE_VIOLATION)

/**
 * @brief Get eventABSactivated flag (ASN.1 bit 2).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_ABS_ACTIVATED(buf)                                     \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_ABS_ACTIVATED)

/**
 * @brief Get eventTractionControlLoss flag (ASN.1 bit 3).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_TRACTION_CONTROL_LOSS(buf)                             \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_TRACTION_CONTROL_LOSS)

/**
 * @brief Get eventStabilityControlactivated flag (ASN.1 bit 4).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_STABILITY_CONTROLACTIVATED(buf)                        \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_STABILITY_CONTROLACTIVATED)

/**
 * @brief Get eventHazardousMaterials flag (ASN.1 bit 5).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARDOUS_MATERIALS(buf)                               \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HAZARDOUS_MATERIALS)

/**
 * @brief Get eventReserved1 flag (ASN.1 bit 6).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_RESERVED_1(buf)                                        \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_RESERVED_1)

/**
 * @brief Get eventHardBraking flag (ASN.1 bit 7).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HARD_BRAKING(buf)                                      \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HARD_BRAKING)

/**
 * @brief Get eventLightsChanged flag (ASN.1 bit 8).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_LIGHTS_CHANGED(buf)                                    \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_LIGHTS_CHANGED)

/**
 * @brief Get eventWipersChanged flag (ASN.1 bit 9).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_WIPERS_CHANGED(buf)                                    \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_WIPERS_CHANGED)

/**
 * @brief Get eventFlatTire flag (ASN.1 bit 10).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_FLAT_TIRE(buf)                                         \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_FLAT_TIRE)

/**
 * @brief Get eventDisabledVehicle flag (ASN.1 bit 11).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_DISABLED_VEHICLE(buf)                                  \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_DISABLED_VEHICLE)

/**
 * @brief Get eventAirBagDeployment flag (ASN.1 bit 12).
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @return 0 or 1.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_AIR_BAG_DEPLOYMENT(buf)                                \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_AIR_BAG_DEPLOYMENT)

/**
 * @brief Get eventJackKnife flag (ASN.1 bit 13, extended form only).
 *
 * This flag is only present in extended-form VehicleEventFlags messages.
 *
 * @param[in] buf Pointer to VehicleEventFlags UPER encoding (const uint8_t*).
 * @pre @p buf must point to valid encoding with +7 byte padding.
 * @pre Caller should verify J2735_VEHICLE_EVENT_FLAGS_IS_EXTENDED(buf) returns true.
 * @return 0 or 1.
 * @warning Returns garbage (0 or 1) for non-extended messages. Always check IS_EXTENDED first.
 */
#define J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE(buf)                                        \
  J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS(                                                      \
      J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf),                                            \
      J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_JACK_KNIFE)

#endif /* J2735_INTERNAL_DE_VEHICLEEVENTFLAGS_H */
