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
 * @brief J2735 ApproachOrLane Definition and Access Macros.
 *
 * @par ApproachOrLane Wire Format (UPER):
 * @verbatim
 * ApproachOrLane ::= CHOICE {
 *     approach  ApproachID,  -- 4 bits (J2735_BW_APPROACH_ID)
 *     lane  LaneID  -- 8 bits (J2735_BW_LANE_ID)
 * }
 * @endverbatim
 *
 * This is a non-extensible CHOICE with 2 alternatives.
 * Per ITU-T X.691 §23, the choice index uses ceil(log2(2)) = 1 bit.
 *
 * @par Wire Format (approach selected, 5 bits total):
 * @code
 * ┌───────────┬───────────────────────────┐
 * │ Bit 0     │ Bits 1-4                  │
 * ├───────────┼───────────────────────────┤
 * │ Index = 0 │ ApproachID value (4 bits) │
 * └───────────┴───────────────────────────┘
 * @endcode
 *
 * @par Wire Format (lane selected, 9 bits total):
 * @code
 * ┌───────────┬───────────────────────┐
 * │ Bit 0     │ Bits 1-8              │
 * ├───────────┼───────────────────────┤
 * │ Index = 1 │ LaneID value (8 bits) │
 * └───────────┴───────────────────────┘
 * @endcode
 *
 * Performance Rationale (Single I/O Pattern):
 * Reading 9 bits unconditionally (even when smaller alternatives use less) is faster than
 * reading the index first, then conditionally reading the value bits:
 *   - Single I/O: ~9-10 instructions, 1 memory load, 0 branches
 *   - Two-Step:   ~13-14 instructions, 2 memory loads, 1 branch (misprediction risk)
 * The "garbage" bits (when smaller alternative selected) cost nothing - already in register.
 * This pattern applies when max_variant ≤ 56 bits (J2735_READ_BITS limit).
 *
 * Usage Pattern (single I/O, O(1)):
 * @code
 * uint16_t const raw9 = J2735_APPROACH_OR_LANE_RAW_READ(buf); [9 bits in uint16_t]
 *
 * switch (J2735_APPROACH_OR_LANE_WHICH(raw9)) {
 * case J2735_CHOICE_APPROACH_OR_LANE_APPROACH:
 *     use(J2735_APPROACH_OR_LANE_GET_APPROACH(raw9)); [uint8_t: 4 bits]
 *     break;
 * case J2735_CHOICE_APPROACH_OR_LANE_LANE:
 *     use(J2735_APPROACH_OR_LANE_GET_LANE(raw9)); [uint8_t: 8 bits]
 *     break;
 * }
 *
 * buf += J2735_BIT_TO_BYTE(J2735_APPROACH_OR_LANE_SIZE(raw9)); [uint8_t: 5 or 9]
 * @endcode
 */
#ifndef J2735_INTERNAL_DF_APPROACHORLANE_H
#define J2735_INTERNAL_DF_APPROACHORLANE_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* ============================================================================================== */
/*  INTERNAL: CHOICE Index Bits                                                                   */
/* ============================================================================================== */
/**
 * @internal
 * @brief Number of bits for ApproachOrLane choice index.
 *
 * ApproachOrLane has 2 alternatives, so index = ceil(log2(2)) = 1 bit.
 * This is a non-extensible CHOICE.
 */
#define J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE 1U

_Static_assert(J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE == J2735_BW_APPROACH_OR_LANE,
               "CHOICE index bits must match J2735_BW_APPROACH_OR_LANE");

/* ============================================================================================== */
/*  INTERNAL: Structure Metadata                                                                  */
/* ============================================================================================== */
/**
 * @internal
 * @brief Maximum wire size in bits for ApproachOrLane encoding.
 *
 * This is the larger of the alternatives: 1 (index) + 8 (lane) = 9 bits.
 *
 * We always read MAX bits (9) unconditionally rather than reading the index first
 * then conditionally reading the value bits. Assembly-level analysis:
 *
 *   Single I/O (read max):  ~9-10 instructions, 1 memory load, 0 branches
 *   Two-Step (conditional): ~13-14 instructions, 2 memory loads, 1 branch
 *
 * The "garbage" bits when a smaller alternative is selected cost nothing - they're already
 * loaded into the register and simply get shifted/masked away.
 */
#define J2735_INTERNAL_MAX_WIRE_BITS_APPROACH_OR_LANE                                              \
  (J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE + J2735_BW_LANE_ID)

/* ============================================================================================== */
/*  PUBLIC API: Alternative Indices                                                               */
/* ============================================================================================== */
/**
 * @brief Alternative index for 'approach' in ApproachOrLane CHOICE.
 */
#define J2735_CHOICE_APPROACH_OR_LANE_APPROACH 0U

/**
 * @brief Alternative index for 'lane' in ApproachOrLane CHOICE.
 */
#define J2735_CHOICE_APPROACH_OR_LANE_LANE 1U

/* ============================================================================================== */
/*  PUBLIC API: Raw Read (Single I/O)                                                             */
/* ============================================================================================== */
/**
 * @brief Read raw 9 bits for ApproachOrLane encoding (single I/O operation).
 *
 * Reads 9 bits (maximum wire size) into a uint16_t. All other ApproachOrLane
 * macros operate on this pre-read 9-bit value for O(1) performance.
 *
 * Raw value bit layout (9 bits in uint16_t, right-justified):
 * @code
 *   uint16_t bit: 8..0
 *                  [I][Value bits...]
 *
 *   Where:
 *     I = CHOICE index
 *     Value bits = Alternative-specific data
 * @endcode
 *
 * @param[in] buf Pointer to the ApproachOrLane encoding (const uint8_t*).
 * @return uint16_t containing the raw 9-bit encoding.
 * @warning J2735_READ_BITS loads 8 bytes for efficient bit extraction. Caller must ensure
 *          buffer has at least 8 bytes of readable memory (2 data + 6 padding, or more).
 *
 * @note Store result in `uint16_t raw9` variable, then pass to other macros.
 */
#define J2735_APPROACH_OR_LANE_RAW_READ(buf)                                                       \
  ((uint16_t)J2735_READ_BITS((buf), 0U, J2735_INTERNAL_MAX_WIRE_BITS_APPROACH_OR_LANE))

/* ============================================================================================== */
/*  PUBLIC API: Which-Checker (Pure Computation on Raw Value)                                     */
/* ============================================================================================== */
/**
 * @brief Get the CHOICE alternative index (1 bit) from pre-read raw value.
 *
 * Extracts the index bits from the MSB position of the raw value.
 *
 * @param[in] raw9 9-bit value previously returned by J2735_APPROACH_OR_LANE_RAW_READ().
 * @return uint8_t: Alternative index (0-1).
 *         Use J2735_CHOICE_APPROACH_OR_LANE_* constants for comparison in switch statements.
 */
#define J2735_APPROACH_OR_LANE_WHICH(raw9)                                                         \
  ((uint8_t)((((uint32_t)(raw9)) >> (J2735_INTERNAL_MAX_WIRE_BITS_APPROACH_OR_LANE - 1U)) &        \
             ((1U << J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE) - 1U)))

/* ============================================================================================== */
/*  PUBLIC API: Getters (Pure Computation on Raw Value)                                           */
/* ============================================================================================== */
/**
 * @brief Get 'approach' value (ApproachID, 4 bits) from pre-read raw value.
 *
 * Extracts the 4-bit value after shifting right by 4 bits.
 *
 * @param[in] raw9 9-bit value previously returned by J2735_APPROACH_OR_LANE_RAW_READ().
 * @return uint8_t: ApproachID value (4 bits).
 * @pre J2735_APPROACH_OR_LANE_WHICH(raw9) == J2735_CHOICE_APPROACH_OR_LANE_APPROACH.
 */
#define J2735_APPROACH_OR_LANE_GET_APPROACH(raw9)                                                  \
  ((uint8_t)((((uint32_t)(raw9)) >>                                                                \
              (J2735_INTERNAL_MAX_WIRE_BITS_APPROACH_OR_LANE - 1U - J2735_BW_APPROACH_ID)) &       \
             ((1UL << J2735_BW_APPROACH_ID) - 1UL)))

/**
 * @brief Get 'lane' value (LaneID, 8 bits) from pre-read raw value.
 *
 * Extracts the 8-bit value from the lowest bits (no shift needed).
 *
 * Generic pattern: ((raw >> (MAX - INDEX_BITS - BW)) & MASK)
 * Because: shift = 9 - 1 - 8 = 0, we omit the shift (MISRA 12.2 compliant).
 *
 * @param[in] raw9 9-bit value previously returned by J2735_APPROACH_OR_LANE_RAW_READ().
 * @return uint8_t: LaneID value (8 bits).
 * @pre J2735_APPROACH_OR_LANE_WHICH(raw9) == J2735_CHOICE_APPROACH_OR_LANE_LANE.
 */
#define J2735_APPROACH_OR_LANE_GET_LANE(raw9)                                                      \
  ((uint8_t)((raw9) & ((1UL << J2735_BW_LANE_ID) - 1UL)))

/* ============================================================================================== */
/*  PUBLIC API: Size Calculation (Pure Computation on Raw Value)                                  */
/* ============================================================================================== */
/**
 * @brief Calculate the total wire size in bits from pre-read raw value.
 *
 * The size depends on which alternative is selected:
 *   - approach: 1 (index) + 4 (value) = 5 bits
 *   - lane: 1 (index) + 8 (value) = 9 bits
 *
 * @param[in] raw9 9-bit value previously returned by J2735_APPROACH_OR_LANE_RAW_READ().
 * @return uint8_t: Total encoding size in bits (5 or 9).
 */
#define J2735_APPROACH_OR_LANE_SIZE(raw9)                                                          \
  ((uint8_t)(J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE +                                   \
             ((J2735_APPROACH_OR_LANE_WHICH(raw9) == J2735_CHOICE_APPROACH_OR_LANE_APPROACH)       \
                  ? (J2735_BW_APPROACH_ID)                                                         \
                  : (J2735_BW_LANE_ID))))

#endif /* J2735_INTERNAL_DF_APPROACHORLANE_H */
