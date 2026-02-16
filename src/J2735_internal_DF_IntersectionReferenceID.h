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
 * @brief J2735 IntersectionReferenceID Definition and Access Macros.
 *
 * @par IntersectionReferenceID Wire Format (UPER):
 * @code
 * IntersectionReferenceID ::= SEQUENCE {
 *     region        RoadRegulatorID OPTIONAL,  -- 16 bits (unsigned, 0..65535)
 *     id            IntersectionID             -- 16 bits (unsigned, 0..65535)
 * }
 * @endcode
 *
 * @par Wire Format (region ABSENT, 17 bits):
 * @code
 * ┌────────────┬──────────────┐
 * │ Bit 0      │ Bits 1-16    │
 * ├────────────┼──────────────┤
 * │ Opt=0      │ id (16)      │
 * └────────────┴──────────────┘
 * @endcode
 *
 * @par Wire Format (region PRESENT, 33 bits):
 * @code
 * ┌────────────┬──────────────┬──────────────┐
 * │ Bit 0      │ Bits 1-16    │ Bits 17-32   │
 * ├────────────┼──────────────┼──────────────┤
 * │ Opt=1      │ region (16)  │ id (16)      │
 * └────────────┴──────────────┴──────────────┘
 * @endcode
 */
#ifndef J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_H
#define J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* ============================================================================================== */
/*  INTERNAL: Structure Metadata                                                                  */
/* ============================================================================================== */
/**
 * @internal
 * @brief Number of prefix bits before first field (extension bit + optional preamble).
 *
 * Calculation: 0 ext + 1 opt = 1 bit (non-extensible, 1 OPTIONAL)
 */
#define J2735_INTERNAL_PREFIX_BITS_INTERSECTION_REFERENCE_ID                                       \
  (0U + J2735_INTERNAL_PREAMBLE_BITS(1U)) /* 0 ext + 1 opt = 1 bit (non-extensible, 1 OPTIONAL) */

/* ============================================================================================== */
/*  INTERNAL: Optional Field Indices                                                              */
/*  (Bitmap position for each OPTIONAL field, 0-indexed from MSB)                                 */
/* ============================================================================================== */
#define J2735_INTERNAL_OPT_INTERSECTION_REFERENCE_ID_REGION 0U /* optional bitmap bit 0 */

/* ============================================================================================== */
/*  INTERNAL: Dynamic Widths                                                                      */
/*  (Returns 0 if field absent, else J2735_BW_<FieldType> or computed width)                      */
/* ============================================================================================== */
/**
 * @internal
 * @brief Dynamic width of OPTIONAL field 'region'.
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return J2735_BW_ROAD_REGULATOR_ID if present, 0 otherwise.
 */
#define J2735_INTERNAL_WIDTH_INTERSECTION_REFERENCE_ID_REGION(buf)                                 \
  (J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf) ? J2735_BW_ROAD_REGULATOR_ID : 0U)

/* ============================================================================================== */
/*  INTERNAL: Field Offsets                                                                       */
/*  (Cumulative bit offset: prev_offset + prev_width)                                             */
/* ============================================================================================== */
/**
 * @internal
 * @brief Bit offset of field 'region' within IntersectionReferenceID.
 */
#define J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_REGION(buf)                                   \
  J2735_INTERNAL_PREFIX_BITS_INTERSECTION_REFERENCE_ID /*   1 */

/**
 * @internal
 * @brief Bit offset of field 'id' within IntersectionReferenceID.
 */
#define J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_ID(buf)                                       \
  (J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_REGION(buf) +                                      \
   J2735_INTERNAL_WIDTH_INTERSECTION_REFERENCE_ID_REGION(buf))

/* ============================================================================================== */
/*  PUBLIC API: Has-Checkers (OPTIONAL Fields)                                                    */
/* ============================================================================================== */
/**
 * @brief Check if OPTIONAL field 'region' (RoadRegulatorID) is present.
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return 1 if present, 0 otherwise.
 */
#define J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf)                                            \
  J2735_INTERNAL_HAS_FIELD((buf), 0U, J2735_INTERNAL_OPT_INTERSECTION_REFERENCE_ID_REGION)

/* ============================================================================================== */
/*  PUBLIC API: Field Getters                                                                     */
/* ============================================================================================== */
/**
 * @brief Get 'region' (RoadRegulatorID, unsigned 16 bits).
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return RoadRegulatorID value (uint16_t, range 0..65535).
 * @pre J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf) must be true.
 */
#define J2735_INTERSECTION_REFERENCE_ID_GET_REGION(buf)                                            \
  ((uint16_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_REGION(buf),      \
                             J2735_BW_ROAD_REGULATOR_ID))

/**
 * @brief Get 'id' (IntersectionID, unsigned 16 bits).
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return IntersectionID value (uint16_t, range 0..65535).
 */
#define J2735_INTERSECTION_REFERENCE_ID_GET_ID(buf)                                                \
  ((uint16_t)J2735_READ_BITS((buf), J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_ID(buf),          \
                             J2735_BW_INTERSECTION_ID))

#endif /* J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_H */
