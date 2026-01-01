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
 * IntersectionReferenceID ::= SEQUENCE {
 *     region  RoadRegulatorID OPTIONAL,  -- 16 bits (J2735_BW_ROAD_REGULATOR_ID)
 *     id      IntersectionID             -- 16 bits (J2735_BW_INTERSECTION_ID)
 * }
 *
 * Wire Format (region absent, 17 bits):
 * | Bit 0   | Bits 1-16 |
 * |---------|-----------|
 * | Opt=0   | id(16)    |
 *
 * Wire Format (region present, 33 bits):
 * | Bit 0   | Bits 1-16  | Bits 17-32 |
 * |---------|------------|------------|
 * | Opt=1   | region(16) | id(16)     |
 *
 * @todo Update the Doxygen to indicate [in] and [out] parameters
 */
#ifndef J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_H
#define J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* Internal - Structure metadata */
#define J2735_PREFIX_BITS_INTERSECTION_REFERENCE_ID                                                \
  (0U + J2735_INTERNAL_PREAMBLE_BITS(1U)) /* 0 ext + 1 opt = 1 bit (non-extensible, 1 OPTIONAL) */

/* Internal - Root component size (for calculating where extensions start) */

/* Internal - Optional field indices (bitmap index, not bit offset) */
#define J2735_OPT_INTERSECTION_REFERENCE_ID_REGION 0U /* optional bitmap bit 0 */

/* Internal - Widths */
#define J2735_WIDTH_INTERSECTION_REFERENCE_ID_REGION(buf)                                          \
  (J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf) ? J2735_BW_ROAD_REGULATOR_ID : 0U)

/* Internal - Offsets */
#define J2735_OFF_INTERSECTION_REFERENCE_ID_REGION(buf)                                            \
  J2735_PREFIX_BITS_INTERSECTION_REFERENCE_ID /*   1 */
#define J2735_OFF_INTERSECTION_REFERENCE_ID_ID(buf)                                                \
  (J2735_OFF_INTERSECTION_REFERENCE_ID_REGION(buf) +                                               \
   J2735_WIDTH_INTERSECTION_REFERENCE_ID_REGION(buf))

/* Has-checkers */
/**
 * @brief Check if OPTIONAL field 'region' (RoadRegulatorID) is present.
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return 1 if present, 0 otherwise.
 */
#define J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf)                                            \
  J2735_INTERNAL_HAS_FIELD((buf), 0U, J2735_OPT_INTERSECTION_REFERENCE_ID_REGION)

/* Getters */
/**
 * @brief Get 'region' (RoadRegulatorID, unsigned 16 bits).
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return RoadRegulatorID value (uint16_t, range 0..65535).
 * @pre J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf) must be true.
 */
#define J2735_INTERSECTION_REFERENCE_ID_GET_REGION(buf)                                            \
  ((uint16_t)J2735_READ_BITS((buf), J2735_OFF_INTERSECTION_REFERENCE_ID_REGION(buf),               \
                             J2735_BW_ROAD_REGULATOR_ID))
/**
 * @brief Get 'id' (IntersectionID, unsigned 16 bits).
 * @param buf Pointer to the IntersectionReferenceID encoding.
 * @return IntersectionID value (uint16_t, range 0..65535).
 */
#define J2735_INTERSECTION_REFERENCE_ID_GET_ID(buf)                                                \
  ((uint16_t)J2735_READ_BITS((buf), J2735_OFF_INTERSECTION_REFERENCE_ID_ID(buf),                   \
                             J2735_BW_INTERSECTION_ID))

/* Inline Functions */

#endif /* J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_H */
