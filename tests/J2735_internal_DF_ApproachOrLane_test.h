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
 * @brief Tests for ApproachOrLane CHOICE type.
 *
 * ApproachOrLane is a non-extensible CHOICE with 2 alternatives.
 * This validates CHOICE index reading and alternative value extraction.
 */

#ifndef J2735_INTERNAL_DF_APPROACHORLANE_TEST_H
#define J2735_INTERNAL_DF_APPROACHORLANE_TEST_H

/* Happy path tests */
void test_approach_or_lane_approach_typical(void);
void test_approach_or_lane_lane_typical(void);

/* Boundary value tests - approach */
void test_approach_or_lane_approach_boundary_min(void);
void test_approach_or_lane_approach_boundary_max(void);

/* Boundary value tests - lane */
void test_approach_or_lane_lane_boundary_min(void);
void test_approach_or_lane_lane_boundary_max(void);

/* Misalignment tests */
void test_approach_or_lane_misaligned_access(void);

void run_testsuite_approach_or_lane(void);

#endif /* J2735_INTERNAL_DF_APPROACHORLANE_TEST_H */
