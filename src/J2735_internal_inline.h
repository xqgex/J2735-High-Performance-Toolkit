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
 * @brief TODO
 */
#ifndef J2735_INTERNAL_INLINE_H
#define J2735_INTERNAL_INLINE_H

#include "J2735_internal_common.h"
#include "J2735_internal_constants.h"

/* ============================================================================================== */
/*  Inline Functions                                                                              */
/* ============================================================================================== */

/**
 * @brief Read a UPER length determinant starting at a given bit offset.
 *
 * UPER length determinants are encoded as:
 *   - If bit[0] = 0: next 7 bits contain length (0-127)
 *   - If bit[0] = 1 and bit[1] = 0: next 14 bits contain length (0-16383)
 *   - Larger values use fragmented encoding (not implemented here)
 *
 * This function returns both the decoded length and the number of bits consumed.
 *
 * @param buf       Pointer to the buffer.
 * @param bit_off   Bit offset where the length determinant starts.
 * @param out_len   Output: the decoded length value.
 * @return Number of bits consumed by the length determinant (8 or 16), or 0 on error.
 */
static inline uint8_t j2735_internal_inline_read_length_determinant(uint8_t const *const buf,
                                                                    uint32_t const bit_off,
                                                                    uint16_t *const out_len) {
  uint8_t result = 0U;
  /**
   * Optimization: Read 2 bits upfront to save one J2735_READ_BITS call in long-form path.
   * X.691 §11.9 encoding:
   *   - 0xxxxxxx: short form (7-bit length, 0-127)
   *   - 10xxxxxx xxxxxxxx: long form (14-bit length, 0-16383)
   *   - 11xxxxxx: fragmented (not supported)
   *
   * We check bit[0] first. If 0 -> short form. If 1 -> check bit[1] for long vs fragmented.
   * two_bits layout: bits are MSB-first from stream, so bit[0] is (two_bits >> 1) & 1,
   * but since we only care about the combination, we use direct value comparison.
   */
  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
  uint8_t const two_bits = (uint8_t)J2735_READ_BITS(buf, bit_off, 2U);

  if (0U == (two_bits & J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_FORM_MASK)) {
    /* bit[0] = 0: Short form (0x or 00-01 in two_bits) */
    /* Short form: 1 prefix + 7 value = 8 bits total, max value 127 */
    /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
    *out_len = (uint16_t)J2735_READ_BITS(
        buf, bit_off + J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_PREFIX_BITS,
        J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_VALUE_BITS);
    result = J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_BITS;
  } else if (0U == (two_bits & 1U)) {
    /* bits[0:1] = 10: Long form (two_bits = 2) */
    /* Long form: 2 prefix + 14 value = 16 bits total, max value 16383 */
    uint32_t const read_offset = bit_off + J2735_INTERNAL_LENGTH_DETERMINANT_LONG_PREFIX_BITS;
    /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
    *out_len = (uint16_t)J2735_READ_BITS(buf, read_offset,
                                         J2735_INTERNAL_LENGTH_DETERMINANT_LONG_VALUE_BITS);
    result = J2735_INTERNAL_LENGTH_DETERMINANT_LONG_BITS;
  } else {
    /* bits[0:1] = 11: Fragmented encoding (two_bits = 3), not supported */
    *out_len = 0U;
    result = 0U;
  }
  return result;
}

/**
 * @brief Read a normally-small-non-negative-whole-number (nsnnwn) at a given bit offset.
 *
 * Used for extension bitmap lengths and extension addition counts in UPER.
 *
 * X.691 §11.6 encoding:
 * - If n <= 63: 1 bit (0) + 6 bits value = 7 bits total
 * - If n >= 64: 1 bit (1) + semi-constrained whole number (§11.7.4):
 *   - Length determinant (8 or 16 bits) = number of octets
 *   - Value bytes (len × 8 bits) = the actual value
 *
 * @param buf       Pointer to the buffer.
 * @param bit_off   Bit offset where the nsnnwn starts.
 * @param out_val   Output: the decoded value.
 * @return Number of bits consumed, or 0 on error.
 */
static inline uint8_t j2735_internal_inline_read_nsnnwn(uint8_t const *const buf,
                                                        uint32_t const bit_off,
                                                        uint32_t *const out_val) {
  uint8_t result = 0U;
  /* Single bit read: 0 or 1, safely fits uint8_t */
  /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
  uint8_t const bit_1st = (uint8_t)J2735_READ_BITS(buf, bit_off, J2735_INTERNAL_NSNNWN_PREFIX_BITS);

  if (0U == bit_1st) {
    /* Small form: 1 prefix + 6 value = 7 bits total, value 0-63 */
    /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
    *out_val = (uint32_t)J2735_READ_BITS(buf, bit_off + J2735_INTERNAL_NSNNWN_PREFIX_BITS,
                                         J2735_INTERNAL_NSNNWN_SMALL_VALUE_BITS);
    result = J2735_INTERNAL_NSNNWN_SMALL_BITS;
  } else {
    /* Large form (§11.7.4): 1 bit + length determinant + value octets */
    /* For extension bitmaps, this is rare - typically < 64 extensions */
    /* num_octets: uint16_t for API, but validated below to be 1-4 */
    uint16_t num_octets = 0U;
    uint8_t const len_det_bits = j2735_internal_inline_read_length_determinant(
        buf, bit_off + J2735_INTERNAL_NSNNWN_PREFIX_BITS, &num_octets);
    if ((0U == len_det_bits) || (J2735_INTERNAL_MAX_NSNNWN_OCTETS < num_octets)) {
      /* Error: fragmented encoding or value too large for uint32_t */
      *out_val = 0U;
      result = 0U;
    } else {
      /* Read the actual value from num_octets bytes */
      /* num_octets is validated to be 1-4, so value_bits is 8-32 (fits uint8_t) */
      uint8_t const value_bits = (uint8_t)(num_octets * J2735_INTERNAL_BITS_PER_BYTE);
      uint32_t const value_bit_off = bit_off + 1U + (uint32_t)len_det_bits;
      /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
      *out_val = (uint32_t)J2735_READ_BITS(buf, value_bit_off, value_bits);
      /* All operands are uint8_t: 1 + len_det_bits(8-16) + value_bits(8-32) = max 49 */
      result = (uint8_t)(J2735_INTERNAL_NSNNWN_PREFIX_BITS + len_det_bits + value_bits);
    }
  }
  return result;
}

/**
 * @brief Skip extension additions in an extensible SEQUENCE.
 *
 * When the extension bit is set, the encoding after the root component contains:
 *   1. Extension bitmap length (nsnnwn) - number of extension additions present
 *   2. Extension bitmap - which extensions are present (n+1 bits for n extensions)
 *   3. Each present extension as: length + content (open type)
 *
 * This function calculates the total bit size of all extension data.
 *
 * @param buf            Pointer to the buffer.
 * @param ext_start_off  Bit offset where extension data starts (after root component).
 * @param out_ext_bits   Output: total bits consumed by extensions.
 * @return 0 on success, non-zero on parsing error.
 */
static inline int j2735_internal_inline_skip_extensions(uint8_t const *const buf,
                                                        uint32_t const ext_start_off,
                                                        uint32_t *const out_ext_bits) {
  int result = 0;
  uint32_t cursor = ext_start_off;
  /* ext_count_raw: uint32_t required by j2735_internal_inline_read_nsnnwn API. */
  /* After validation, narrowed to uint8_t ext_count (max 63). */
  uint32_t ext_count_raw = 0U;
  bool parse_error = false;

  /* Read extension bitmap length (nsnnwn) - gives count of possible extensions */
  uint8_t const nsnnwn_bits = j2735_internal_inline_read_nsnnwn(buf, cursor, &ext_count_raw);
  if (0U == nsnnwn_bits) {
    parse_error = true;
  }
  /* Validate ext_count fits in bitmap (uint64_t can hold 64 bits, so max ext_count is 63) */
  if ((!parse_error) && (J2735_MAX_EXTENSION_ADDITIONS < ext_count_raw)) {
    parse_error = true;
  }

  if (!parse_error) {
    cursor += nsnnwn_bits;

    /* ext_count: validated above to be 0-63, safely fits uint8_t */
    uint8_t const ext_count = (uint8_t)ext_count_raw;
    /* bitmap_bits: ext_count + 1, so max 64, safely fits uint8_t */
    uint8_t const bitmap_bits = (uint8_t)(ext_count + 1U);
    /* bitmap: uint64_t to hold up to 64 bits */
    /* cppcheck-suppress misra-c2012-11.3 ; Zero-copy architecture requires packed-struct cast */
    uint64_t const bitmap = J2735_READ_BITS(buf, cursor, bitmap_bits);
    cursor += bitmap_bits;

    /* For each set bit in bitmap, skip the open type (length + content) */
    /* index: loops 0 to ext_count (max 63), safely fits uint8_t */
    /* NOLINTNEXTLINE(altera-id-dependent-backward-branch) FPGA-specific check, not applicable */
    for (uint8_t index = 0U; (index <= ext_count) && (!parse_error); ++index) {
      /* Check bit from MSB (first extension is highest bit)
       * Cast to uint8_t safe: ext_count and index both <= 63, difference is 0-63 */
      uint8_t const shift = (uint8_t)(ext_count - index);
      uint64_t const mask = 1ULL << shift;
      if (0U != (bitmap & mask)) {
        /* Extension is present - read its length and skip content */
        /* ext_len_bytes: max 16383 (length determinant long form), fits uint16_t */
        uint16_t ext_len_bytes = 0U;
        uint8_t const len_bits =
            j2735_internal_inline_read_length_determinant(buf, cursor, &ext_len_bytes);
        if (0U == len_bits) {
          parse_error = true;
        } else {
          cursor += len_bits;
          /* Content is ext_len_bytes * 8 bits (open type is octet-aligned) */
          cursor += (uint32_t)ext_len_bytes * J2735_INTERNAL_BITS_PER_BYTE;
        }
      }
    }
  }

  if (parse_error) {
    *out_ext_bits = 0U;
    result = -1;
  } else {
    *out_ext_bits = cursor - ext_start_off;
    result = 0;
  }
  return result;
}

#endif /* J2735_INTERNAL_INLINE_H */
