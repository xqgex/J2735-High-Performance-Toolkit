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
 * @brief Tests for BrakeAppliedStatus non-extensible BIT STRING.
 *
 * BrakeAppliedStatus is SIZE(5): a fixed BIT STRING with 5 bits.
 */

#ifndef J2735_INTERNAL_DE_BRAKEAPPLIEDSTATUS_TEST_H
#define J2735_INTERNAL_DE_BRAKEAPPLIEDSTATUS_TEST_H

/* Core tests */
void test_brake_applied_status_all_zeros(void);
void test_brake_applied_status_all_ones(void);
void test_brake_applied_status_alternating_10101(void);
void test_brake_applied_status_alternating_01010(void);
void test_brake_applied_status_single_bit_unavailable(void);
void test_brake_applied_status_single_bit_right_rear(void);

/* Metadata tests */
void test_brake_applied_status_size(void);
void test_brake_applied_status_is_extended(void);

/* Misalignment test */
void test_brake_applied_status_misaligned_access(void);

void run_testsuite_brake_applied_status(void);

#endif /* J2735_INTERNAL_DE_BRAKEAPPLIEDSTATUS_TEST_H */
