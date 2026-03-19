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
 * @brief Tests for BSMcoreData non-extensible SEQUENCE.
 *
 * The data frame BSMcoreData is a simple case with no extensions or optional fields.
 */

#ifndef J2735_INTERNAL_DF_BSMCOREDATA_TEST_H
#define J2735_INTERNAL_DF_BSMCOREDATA_TEST_H

/* Happy path tests */
void test_bsm_core_data_fixed_data(void);

/* Boundary value tests - signed fields */
void test_bsm_core_data_latitude_negative_min(void);
void test_bsm_core_data_latitude_positive_max(void);
void test_bsm_core_data_steering_angle_negative(void);
void test_bsm_core_data_steering_angle_positive_max(void);

/* Misalignment tests */
void test_bsm_core_data_misaligned_access(void);

void run_testsuite_bsm_core_data(void);

#endif /* J2735_INTERNAL_DF_BSMCOREDATA_TEST_H */
