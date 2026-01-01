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
 * @brief Unit tests for UPER encoding primitives (Phase 4).
 *
 * Tests for the shared UPER parsing functions:
 * - j2735_internal_inline_read_length_determinant()
 * - j2735_internal_inline_read_nsnnwn()
 * - j2735_internal_inline_skip_extensions()
 */

#ifndef J2735_UPER_TEST_H
#define J2735_UPER_TEST_H

/* j2735_internal_inline_read_length_determinant() tests */
void test_inline_read_length_determinant_short_form_min(void);
void test_inline_read_length_determinant_short_form_max(void);
void test_inline_read_length_determinant_short_form_typical(void);
void test_inline_read_length_determinant_long_form_min(void);
void test_inline_read_length_determinant_long_form_128(void);
void test_inline_read_length_determinant_long_form_max(void);
void test_inline_read_length_determinant_fragmented_error(void);
void test_inline_read_length_determinant_nonzero_bit_offset(void);

/* j2735_internal_inline_read_nsnnwn() tests */
void test_inline_read_nsnnwn_small_form_min(void);
void test_inline_read_nsnnwn_small_form_max(void);
void test_inline_read_nsnnwn_small_form_typical(void);
void test_inline_read_nsnnwn_large_form_64(void);
void test_inline_read_nsnnwn_large_form_100(void);
void test_inline_read_nsnnwn_large_form_255(void);
void test_inline_read_nsnnwn_large_form_256(void);
void test_inline_read_nsnnwn_large_form_65535(void);
void test_inline_read_nsnnwn_large_form_4_bytes(void);
void test_inline_read_nsnnwn_large_form_5_bytes_error(void);
void test_inline_read_nsnnwn_fragmented_error(void);
void test_inline_read_nsnnwn_nonzero_bit_offset(void);

/* j2735_internal_inline_skip_extensions() tests */
void test_inline_skip_extensions_one_slot_none_present(void);
void test_inline_skip_extensions_one_slot_present(void);
void test_inline_skip_extensions_two_slots_both_present(void);
void test_inline_skip_extensions_two_slots_first_only(void);
void test_inline_skip_extensions_two_slots_second_only(void);
void test_inline_skip_extensions_empty_content(void);
void test_inline_skip_extensions_nsnnwn_error(void);
void test_inline_skip_extensions_length_error(void);
void test_inline_skip_extensions_nonzero_offset(void);
void test_inline_skip_extensions_too_many_extensions(void);

/**
 * @brief Run all UPER primitive tests.
 */
void run_testsuite_uper(void);

#endif /* J2735_UPER_TEST_H */
