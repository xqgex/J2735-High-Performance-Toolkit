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
 * @brief Tests for ExteriorLights extensible BIT STRING.
 *
 * ExteriorLights is SIZE(9,...): an extensible BIT STRING with
 * 9-bit root and 9-bit extension (same size).
 */

#ifndef J2735_INTERNAL_DE_EXTERIORLIGHTS_TEST_H
#define J2735_INTERNAL_DE_EXTERIORLIGHTS_TEST_H

/* Basic form tests */
void test_exterior_lights_non_extended(void);
void test_exterior_lights_extended(void);

/* Individual flag accessor tests (non-extended) */
void test_exterior_lights_non_extended_flags(void);

/* SIZE macro tests */
void test_exterior_lights_size_non_extended(void);
void test_exterior_lights_size_extended(void);

/* Edge case tests */
void test_exterior_lights_all_zeros_non_extended(void);
void test_exterior_lights_non_extended_all_flags_on(void);
void test_exterior_lights_extended_all_zeros(void);
void test_exterior_lights_non_extended_alternating_101010101(void);
void test_exterior_lights_non_extended_alternating_010101010(void);

/* Single-bit isolation tests */
void test_exterior_lights_single_bit_0_low_beam(void);
void test_exterior_lights_single_bit_8_parking_lights(void);

/* Misalignment test */
void test_exterior_lights_misaligned_access(void);

void run_testsuite_exterior_lights(void);

#endif /* J2735_INTERNAL_DE_EXTERIORLIGHTS_TEST_H */
