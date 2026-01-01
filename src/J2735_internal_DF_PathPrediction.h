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
 * @brief J2735 PathPrediction Definition and Access Macros.
 *
 * PathPrediction ::= SEQUENCE {
 *     radiusOfCurve  RadiusOfCurvature,  -- 16 bits (signed, -32767..32767)
 *     confidence     Confidence,         -- 8 bits (unsigned, 0..200)
 *     ...                                -- Extensible (extension bit at position 0)
 * }
 *
 * Wire Format (no extensions):
 * | Bit 0 | Bits 1-16           | Bits 17-24    |
 * |-------|---------------------|---------------|
 * | Ext=0 | radiusOfCurve (16)  | confidence(8) |
 *
 * Wire Format (with extensions):
 * | Bit 0 | Bits 1-16           | Bits 17-24    | Extension Data...
 * |-------|---------------------|---------------|------------------
 * | Ext=1 | radiusOfCurve (16)  | confidence(8) | (variable)
 *
 * @todo Update the Doxygen to indicate [in] and [out] parameters
 */
#ifndef J2735_INTERNAL_DF_PATHPREDICTION_H
#define J2735_INTERNAL_DF_PATHPREDICTION_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"
#include "J2735_internal_inline.h"

/* Internal - Structure metadata */
#define J2735_PREFIX_BITS_PATH_PREDICTION                                                          \
  (1U + J2735_INTERNAL_PREAMBLE_BITS(0U)) /* 1 ext + 0 opt = 1 bit (extensible, all required) */

/* Internal - Root component size (for calculating where extensions start) */
#define J2735_ROOT_SIZE_BITS_PATH_PREDICTION                                                       \
  (J2735_PREFIX_BITS_PATH_PREDICTION + J2735_BW_RADIUS_OF_CURVATURE +                              \
   J2735_BW_CONFIDENCE) /* 25 bits */

/* Internal - Optional field indices (bitmap index, not bit offset) */

/* Internal - Widths */

/* Internal - Offsets */
#define J2735_OFF_PATH_PREDICTION_RADIUS_OF_CURVE(buf) J2735_PREFIX_BITS_PATH_PREDICTION /*   1 */
#define J2735_OFF_PATH_PREDICTION_CONFIDENCE(buf)                                                  \
  (J2735_OFF_PATH_PREDICTION_RADIUS_OF_CURVE(buf) + J2735_BW_RADIUS_OF_CURVATURE) /*  17 */

/* Has-checkers */
/**
 * @brief Check if PathPrediction has extension additions present.
 * @param buf Pointer to the PathPrediction encoding.
 * @return 1 if extensions are present, 0 otherwise.
 */
#define J2735_PATH_PREDICTION_HAS_EXTENSION(buf) J2735_INTERNAL_HAS_EXTENSION(buf)

/* Getters */
/**
 * @brief Get 'radiusOfCurve' (RadiusOfCurvature, signed 16 bits).
 * @param buf Pointer to the PathPrediction encoding.
 * @return RadiusOfCurvature value (int16_t, range -32767..32767).
 */
#define J2735_PATH_PREDICTION_GET_RADIUS_OF_CURVE(buf)                                             \
  J2735_INTERNAL_SIGN_EXTEND(J2735_READ_BITS((buf),                                                \
                                             J2735_OFF_PATH_PREDICTION_RADIUS_OF_CURVE(buf),       \
                                             J2735_BW_RADIUS_OF_CURVATURE),                        \
                             J2735_BW_RADIUS_OF_CURVATURE, int16_t)
/**
 * @brief Get 'confidence' (Confidence, unsigned 8 bits).
 * @param buf Pointer to the PathPrediction encoding.
 * @return Confidence value (uint8_t, range 0..200).
 */
#define J2735_PATH_PREDICTION_GET_CONFIDENCE(buf)                                                  \
  ((uint8_t)J2735_READ_BITS((buf), J2735_OFF_PATH_PREDICTION_CONFIDENCE(buf), J2735_BW_CONFIDENCE))

/* Inline Functions */
/**
 * @brief Calculate total size in bits of a PathPrediction encoding.
 *
 * For non-extended messages, returns the fixed root size (25 bits).
 * For extended messages, parses and skips extension data.
 *
 * @param buf           Pointer to the PathPrediction encoding.
 * @param out_size_bits Output: total size in bits.
 * @return 0 on success, non-zero on parsing error.
 */
static inline int j2735_inline_path_prediction_size(uint8_t const *const buf,
                                                    uint32_t *const out_size_bits) {
  int result = 0;

  /* TODO: Investigate all those suppressions */
  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
  /* cppcheck-suppress misra-c2012-17.3 ; cppcheck false positive: v is struct member, not function
   */
  /* cppcheck-suppress misra-config ; cppcheck cannot resolve struct member v through macro
   * expansion */
  if (J2735_PATH_PREDICTION_HAS_EXTENSION(buf) == 0U) {
    *out_size_bits = J2735_ROOT_SIZE_BITS_PATH_PREDICTION;
    result = 0;
  } else {
    /* Extensions present - parse them to find total size */
    uint32_t ext_bits = 0U;
    int const parse_result =
        j2735_internal_inline_skip_extensions(buf, J2735_ROOT_SIZE_BITS_PATH_PREDICTION, &ext_bits);
    if (0 != parse_result) {
      *out_size_bits = 0U;
      result = parse_result;
    } else {
      *out_size_bits = J2735_ROOT_SIZE_BITS_PATH_PREDICTION + ext_bits;
      result = 0;
    }
  }
  return result;
}

#endif /* J2735_INTERNAL_DF_PATHPREDICTION_H */
