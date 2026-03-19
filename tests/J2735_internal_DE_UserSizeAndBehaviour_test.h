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
 * @brief Tests for UserSizeAndBehaviour extensible BIT STRING.
 *
 * UserSizeAndBehaviour is SIZE(5,...): an extensible BIT STRING with
 * 5-bit root and 5-bit extension (same size).
 */

#ifndef J2735_INTERNAL_DE_USERSIZEANDBEHAVIOUR_TEST_H
#define J2735_INTERNAL_DE_USERSIZEANDBEHAVIOUR_TEST_H

/* Basic form tests */
void test_user_size_and_behaviour_non_extended(void);
void test_user_size_and_behaviour_extended(void);

/* Individual flag accessor tests (non-extended) */
void test_user_size_and_behaviour_non_extended_flags(void);

/* SIZE macro tests */
void test_user_size_and_behaviour_size_non_extended(void);
void test_user_size_and_behaviour_size_extended(void);

/* Edge case tests */
void test_user_size_and_behaviour_all_zeros_non_extended(void);
void test_user_size_and_behaviour_non_extended_all_flags_on(void);
void test_user_size_and_behaviour_extended_all_zeros(void);
void test_user_size_and_behaviour_non_extended_alternating_10101(void);
void test_user_size_and_behaviour_non_extended_alternating_01010(void);

/* Single-bit isolation tests */
void test_user_size_and_behaviour_single_bit_0_unavailable(void);
void test_user_size_and_behaviour_single_bit_4_slow_moving(void);

/* Misalignment test */
void test_user_size_and_behaviour_misaligned_access(void);

void run_testsuite_user_size_and_behaviour(void);

#endif /* J2735_INTERNAL_DE_USERSIZEANDBEHAVIOUR_TEST_H */
