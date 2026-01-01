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
 * @brief Tests for VehicleEventFlags extensible BIT STRING.
 *
 * VehicleEventFlags is SIZE(13,...,14): an extensible BIT STRING with
 * 13-bit root and 14-bit extension.
 */

#ifndef J2735_DE_INTERNAL_VEHICLEEVENTFLAGS_TEST_H
#define J2735_DE_INTERNAL_VEHICLEEVENTFLAGS_TEST_H

/* Basic form tests */
void test_vehicle_event_flags_non_extended(void);
void test_vehicle_event_flags_extended(void);

/* Individual flag accessor tests (non-extended) */
void test_vehicle_event_flags_individual_extended_flags_0_to_4(void);
void test_vehicle_event_flags_individual_extended_flags_5_to_9(void);
void test_vehicle_event_flags_individual_extended_flags_10_to_13(void);
void test_vehicle_event_flags_individual_non_extended_flags_0_to_4(void);
void test_vehicle_event_flags_individual_non_extended_flags_5_to_9(void);
void test_vehicle_event_flags_individual_non_extended_flags_10_to_13(void);

/* Individual flag accessor tests (extended) */
void test_vehicle_event_flags_individual_extended_all_on(void);

/* SIZE macro tests */
void test_vehicle_event_flags_size_non_extended(void);
void test_vehicle_event_flags_size_extended(void);

/* Edge case tests */
void test_vehicle_event_flags_all_zeros_non_extended(void);
void test_vehicle_event_flags_extended_single_jackknife(void);
void test_vehicle_event_flags_non_extended_all_root_flags_on_metadata(void);
void test_vehicle_event_flags_non_extended_all_root_flags_on_bits(void);
void test_vehicle_event_flags_extended_all_zeros(void);
void test_vehicle_event_flags_non_extended_alternating_0x1555_metadata(void);
void test_vehicle_event_flags_non_extended_alternating_0x1555_0_to_4(void);
void test_vehicle_event_flags_non_extended_alternating_0x1555_5_to_9(void);
void test_vehicle_event_flags_non_extended_alternating_0x1555_10_to_12(void);
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_metadata(void);
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_0_to_4(void);
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_5_to_9(void);
void test_vehicle_event_flags_non_extended_alternating_0x0AAA_10_to_12(void);

/* Single-bit isolation tests */
void test_vehicle_event_flags_single_bit_0_hazard_lights(void);
void test_vehicle_event_flags_single_bit_12_airbag(void);
void test_vehicle_event_flags_single_bit_7_hard_braking(void);
void test_vehicle_event_flags_extended_single_hazard_lights(void);

void run_testsuite_vehicle_event_flags(void);

#endif /* J2735_DE_INTERNAL_VEHICLEEVENTFLAGS_TEST_H */
