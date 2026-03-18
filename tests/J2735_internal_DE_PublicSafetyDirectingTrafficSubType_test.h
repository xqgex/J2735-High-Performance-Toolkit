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
 * @brief Tests for PublicSafetyDirectingTrafficSubType extensible BIT STRING.
 *
 * PublicSafetyDirectingTrafficSubType is SIZE(7,...): an extensible BIT STRING
 * with 7-bit root and 7-bit extension (same size).
 */

#ifndef J2735_INTERNAL_DE_PUBLICSAFETYDIRECTINGTRAFFICSUBTYPE_TEST_H
#define J2735_INTERNAL_DE_PUBLICSAFETYDIRECTINGTRAFFICSUBTYPE_TEST_H

/* Basic form tests */
void test_public_safety_directing_traffic_sub_type_non_extended(void);
void test_public_safety_directing_traffic_sub_type_extended(void);

/* Individual flag accessor tests (non-extended) */
void test_public_safety_directing_traffic_sub_type_non_extended_flags(void);

/* SIZE macro tests */
void test_public_safety_directing_traffic_sub_type_size_non_extended(void);
void test_public_safety_directing_traffic_sub_type_size_extended(void);

/* Edge case tests */
void test_public_safety_directing_traffic_sub_type_all_zeros_non_extended(void);
void test_public_safety_directing_traffic_sub_type_non_extended_all_flags_on(void);
void test_public_safety_directing_traffic_sub_type_extended_all_zeros(void);
void test_public_safety_directing_traffic_sub_type_non_extended_alternating_1010101(void);
void test_public_safety_directing_traffic_sub_type_non_extended_alternating_0101010(void);

/* Single-bit isolation tests */
void test_public_safety_directing_traffic_sub_type_single_bit_0_unavailable(void);
void test_public_safety_directing_traffic_sub_type_single_bit_6_highway_service(void);

/* Misalignment test */
void test_public_safety_directing_traffic_sub_type_misaligned_access(void);

void run_testsuite_public_safety_directing_traffic_sub_type(void);

#endif /* J2735_INTERNAL_DE_PUBLICSAFETYDIRECTINGTRAFFICSUBTYPE_TEST_H */
