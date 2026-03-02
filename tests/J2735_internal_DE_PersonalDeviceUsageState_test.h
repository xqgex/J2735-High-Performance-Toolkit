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
 * @brief Tests for PersonalDeviceUsageState extensible BIT STRING.
 *
 * PersonalDeviceUsageState is SIZE(9,...): an extensible BIT STRING with
 * 9-bit root and 9-bit extension (same size).
 */

#ifndef J2735_DE_INTERNAL_PERSONALDEVICEUSAGESTATE_TEST_H
#define J2735_DE_INTERNAL_PERSONALDEVICEUSAGESTATE_TEST_H

/* Basic form tests */
void test_personal_device_usage_state_non_extended(void);
void test_personal_device_usage_state_extended(void);

/* Individual flag accessor tests (non-extended) */
void test_personal_device_usage_state_non_extended_flags(void);

/* SIZE macro tests */
void test_personal_device_usage_state_size_non_extended(void);
void test_personal_device_usage_state_size_extended(void);

/* Edge case tests */
void test_personal_device_usage_state_all_zeros_non_extended(void);
void test_personal_device_usage_state_non_extended_all_flags_on(void);
void test_personal_device_usage_state_extended_all_zeros(void);
void test_personal_device_usage_state_non_extended_alternating_101010101(void);
void test_personal_device_usage_state_non_extended_alternating_010101010(void);

/* Single-bit isolation tests */
void test_personal_device_usage_state_single_bit_0_unavailable(void);
void test_personal_device_usage_state_single_bit_8_viewing(void);

/* Misalignment test */
void test_personal_device_usage_state_misaligned_access(void);

void run_testsuite_personal_device_usage_state(void);

#endif /* J2735_DE_INTERNAL_PERSONALDEVICEUSAGESTATE_TEST_H */
