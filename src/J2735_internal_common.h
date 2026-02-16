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
 * @brief Internal Common Macros and Definitions for J2735 Toolkit.
 */
#ifndef J2735_INTERNAL_COMMON_H
#define J2735_INTERNAL_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#include "J2735_internal_constants.h"

#if defined(_MSC_VER)
#  include <stdlib.h>
#endif

/* ============================================================================================== */
/*  Platform and Compiler Abstraction                                                             */
/* ============================================================================================== */

#if defined(_MSC_VER)
#  define J2735_PACK_START __pragma(pack(push, 1))
#  define J2735_PACK_END   __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#  define J2735_PACK_START _Pragma("pack(push, 1)")
#  define J2735_PACK_END   _Pragma("pack(pop)")
#else
#  error "Unsupported compiler: Please define packing directives."
#endif

/* ============================================================================================== */
/*  Compiler Intrinsics & Safe Types                                                              */
/* ============================================================================================== */

/**
 * Define a Packed 64-bit type.
 * Accessing via this pointer tells the compiler "Do not assume alignment".
 */
#if defined(__GNUC__) || defined(__clang__)
struct __attribute__((packed)) j2735_aligned_u64 {
  uint64_t v;
};
#elif defined(_MSC_VER)
#  pragma pack(push, 1)
struct j2735_aligned_u64 {
  uint64_t v;
};
#  pragma pack(pop)
#else
#  error "Compiler not supported for packed casting optimization."
#endif
typedef struct j2735_aligned_u64 j2735_aligned_u64_t;

/**
 * @internal
 * @def J2735_INTERNAL_BSWAP64(x)
 * @brief Byte-Swap for Big-Endian UPER to Host Order conversion.
 *
 * UPER encoding is Big-Endian. On Little-Endian hosts, we must swap bytes.
 * On Big-Endian hosts, no swap is needed.
 */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define J2735_INTERNAL_BSWAP64(x) (x) /* UPER is Big-Endian; no swap needed on BE host */
#elif defined(__GNUC__) || defined(__clang__)
#  define J2735_INTERNAL_BSWAP64(x) __builtin_bswap64(x)
#elif defined(_MSC_VER)
#  define J2735_INTERNAL_BSWAP64(x) _byteswap_uint64(x)
#else
#  error "Unsupported compiler/endianness configuration."
#endif

/* ============================================================================================== */
/*  Core Bit-Stream Arithmetic (0-Copy, Direct Cast)                                              */
/* ============================================================================================== */

/**
 * @internal
 * @brief Raw Unaligned 64-bit Read via Packed Cast.
 * This compiles to a single instruction (MOV on x86, unaligned LDR on ARM).
 * It is strictly 0-copy.
 *
 * MISRA Deviation: misra-c2012-11.3 (pointer cast between object types)
 * Rationale: Zero-copy architecture requires casting uint8_t* to packed struct pointer to enable
 * safe unaligned access without memcpy. This is the core design pattern and cannot be avoided while
 * maintaining zero-copy performance.
 *
 * @param addr Base pointer (uint8_t*)
 * @warning Caller must ensure at least 8 bytes are readable from addr.
 *          Buffers should include +7 bytes of padding beyond the last accessed field.
 */
#define J2735_INTERNAL_LOAD_U64(addr) (((const j2735_aligned_u64_t *)(addr))->v)

/**
 * @brief Extracts N bits from a buffer at a specific bit offset.
 *
 * Logic:
 * 1. J2735_INTERNAL_LOAD_U64: Reads 8 bytes from (bo >> 3) effectively handling unalignment.
 * 2. J2735_INTERNAL_BSWAP64: Flips bytes to Host Order (Big-Endian to Little-Endian conversion).
 * 3. Shifts: Isolates the specific bits requested.
 *
 * @param b  Base pointer (uint8_t*)
 * @param bo Bit Offset (integer)
 * @param n  Number of bits to extract (1 <= n <= 56).
 * @pre Caller must ensure (bo + n) does not exceed buffer bounds (including +7 byte padding).
 * @warning For n > 56, bits may be truncated when bo is not byte-aligned.
 */
#define J2735_READ_BITS(b, bo, n)                                                                  \
  /* Byte offset: bo >> 3 (divide by 8) */                                                         \
  /* Bit-in-byte: bo & 7 (modulo 8) */                                                             \
  /* Right-align: shift right by (64 - n) to place n bits at LSB */                                \
  ((J2735_INTERNAL_BSWAP64(                                                                        \
        J2735_INTERNAL_LOAD_U64(&(b)[(bo) >> J2735_INTERNAL_BITS_TO_BYTE_SHIFT]))                  \
    << ((bo) & J2735_INTERNAL_BIT_IN_BYTE_MASK)) >>                                                \
   (J2735_INTERNAL_U64_WIDTH_BITS - (n)))

/**
 * @internal
 * @brief Sign-extend an N-bit unsigned value to a signed type.
 *
 * UPER encoding stores signed integers as offset values (always positive in the bitstream).
 * This macro performs two's complement sign extension using unsigned arithmetic to avoid:
 * - Left-shifting signed values (undefined behavior in C11/C17)
 * - Right-shifting signed negative values (implementation-defined)
 *
 * Implementation:
 * 1. Check sign bit: `(val) & (1ULL << ((n) - 1U))` tests bit (n-1), the MSB/sign bit
 * 2. If sign bit is set (negative): OR with `~0ULL << n` to fill upper bits with 1s
 * 3. If sign bit is clear (positive): return value unchanged
 *
 * @param val  The unsigned value extracted by J2735_READ_BITS.
 * @param n    Number of bits in the original field (1 <= n <= 64).
 * @param type The signed target type (e.g., int32_t).
 * @return The sign-extended value as the specified signed type.
 * @pre n must not exceed sizeof(type) * 8.
 * @note For a 31-bit Latitude, if bit 30 is set, upper bits are filled with 1s.
 */
#define J2735_INTERNAL_SIGN_EXTEND(val, n, type)                                                   \
  ((type)(((val) & (1ULL << ((n) - 1U))) ? ((val) | (~0ULL << (n))) : (val)))

/* ============================================================================================== */
/*  Prefix & Optional Bitmap helpers                                                              */
/* ============================================================================================== */
/*                                                                                                */
/*  UPER SEQUENCE Encoding Layout:                                                                */
/*  ┌─────────────────┬──────────────────┬─────────────────────────┐                              */
/*  │ Extension Bit   │ Optional Bitmap  │ Root Component Fields   │                              */
/*  │ (1 bit if ...)  │ (1 bit per OPT)  │ (actual data)           │                              */
/*  └─────────────────┴──────────────────┴─────────────────────────┘                              */
/*       ↑                   ↑                                                                    */
/*  Only present if     "Preamble" in                                                             */
/*  type is extensible   X.691 §21.4                                                              */
/*                                                                                                */
/*  Terminology:                                                                                  */
/*    PREFIX_BITS = extension_bit + optional_bitmap = total bits before first field               */
/*    PREAMBLE    = optional_bitmap only (X.691 terminology)                                      */
/*    OPT_OFFSET  = bit offset where optional bitmap starts (0 or 1)                              */
/*                                                                                                */
/* ============================================================================================== */

/**
 * @internal
 * @brief Number of optional bitmap bits for a SEQUENCE with N OPTIONAL fields.
 *
 * In UPER, a SEQUENCE with OPTIONAL fields has a presence bitmap ("preamble").
 * Each OPTIONAL field requires 1 bit. This is an identity macro for clarity.
 *
 * @note This counts only the optional bitmap, NOT the extension bit.
 *       For total prefix bits, use J2735_INTERNAL_PREFIX_BITS_<TYPE>.
 *
 * @param n Number of OPTIONAL fields in the SEQUENCE.
 * @return Number of optional bitmap bits (equals n).
 *
 * Example: IntersectionReferenceID has 1 OPTIONAL field -> 1 bitmap bit.
 */
#define J2735_INTERNAL_PREAMBLE_BITS(n) (n)

/**
 * @internal
 * @brief Check if an OPTIONAL field is present in the SEQUENCE optional bitmap.
 *
 * The optional bitmap ("preamble") is stored after the extension bit (if any).
 * Bit 0 (MSB of bitmap) = first OPTIONAL field, bit 1 = second, etc.
 *
 * @param buf        Pointer to the start of the SEQUENCE encoding (uint8_t*).
 * @param opt_offset Bit offset where the optional bitmap starts:
 *                   - 0 for non-extensible SEQUENCEs
 *                   - 1 for extensible SEQUENCEs (after extension bit)
 * @param field_idx  0-based index of the OPTIONAL field in the bitmap.
 * @return 1 if field is present, 0 otherwise.
 *
 * Example for IntersectionReferenceID (non-extensible, 1 OPTIONAL):
 *   J2735_HAS_FIELD(buf, 0, 0) -> checks if 'region' field is present
 */
#define J2735_INTERNAL_HAS_FIELD(buf, opt_offset, field_idx)                                       \
  ((J2735_READ_BITS((buf), (opt_offset) + (field_idx), 1U)) != 0U)

/**
 * @internal
 * @brief Read the extension bit (first bit) of an extensible SEQUENCE.
 *
 * Per X.691 §21.5, the extension bit is always at bit offset 0 (first bit of the encoding).
 * It is a single bit: 0 = no extensions present, 1 = extensions present.
 *
 * @param buf Pointer to the start of the SEQUENCE encoding.
 * @return 1 if extensions are present, 0 otherwise.
 */
#define J2735_INTERNAL_HAS_EXTENSION(buf)                                                          \
  ((J2735_READ_BITS((buf), 0U, J2735_INTERNAL_EXTENSION_MARKER_BITS)) != 0U)

#endif /* J2735_INTERNAL_COMMON_H */
