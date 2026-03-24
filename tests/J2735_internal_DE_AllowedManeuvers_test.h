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
 * @brief Tests for AllowedManeuvers non-extensible BIT STRING.
 *
 * AllowedManeuvers is SIZE(12): a fixed BIT STRING with 12 bits.
 */

#ifndef J2735_INTERNAL_DE_ALLOWEDMANEUVERS_TEST_H
#define J2735_INTERNAL_DE_ALLOWEDMANEUVERS_TEST_H

/* Core tests */
void test_allowed_maneuvers_all_zeros(void);
void test_allowed_maneuvers_all_ones_bits_0_to_5(void);
void test_allowed_maneuvers_all_ones_bits_6_to_11(void);
void test_allowed_maneuvers_alternating_101010101010(void);
void test_allowed_maneuvers_alternating_010101010101(void);
void test_allowed_maneuvers_single_bit_straight_allowed(void);
void test_allowed_maneuvers_single_bit_reserved_1(void);

/* Metadata tests */
void test_allowed_maneuvers_size(void);
void test_allowed_maneuvers_has_extension(void);

/* Misalignment test */
void test_allowed_maneuvers_misaligned_access(void);

void run_testsuite_allowed_maneuvers(void);

#endif /* J2735_INTERNAL_DE_ALLOWEDMANEUVERS_TEST_H */
