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
 * @brief Sanity tests for IntersectionReferenceID.
 *
 * The data frame IntersectionReferenceID is a simple case with an optional fields.
 */

#ifndef J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_TEST_H
#define J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_TEST_H

/* Happy path tests */
void test_intersection_reference_id_optional_field_absent(void);
void test_intersection_reference_id_optional_field_present(void);

/* Boundary value tests */
void test_intersection_reference_id_boundary_min(void);
void test_intersection_reference_id_boundary_max(void);
void test_intersection_reference_id_absent_region_max_id(void);

/* Misalignment tests */
void test_intersection_reference_id_misaligned_access(void);

void run_testsuite_intersection_reference_id(void);

#endif /* J2735_INTERNAL_DF_INTERSECTIONREFERENCEID_TEST_H */
