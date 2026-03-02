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
 * @brief Tests for LaneSharing non-extensible BIT STRING.
 *
 * LaneSharing is SIZE(10): a fixed BIT STRING with 10 bits.
 */

#ifndef J2735_DE_INTERNAL_LANESHARING_TEST_H
#define J2735_DE_INTERNAL_LANESHARING_TEST_H

/* Core tests */
void test_lane_sharing_all_zeros(void);
void test_lane_sharing_all_ones_bits_0_to_4(void);
void test_lane_sharing_all_ones_bits_5_to_9(void);
void test_lane_sharing_alternating_1010101010(void);
void test_lane_sharing_alternating_0101010101(void);
void test_lane_sharing_single_bit_overlapping(void);
void test_lane_sharing_single_bit_reserved(void);

/* Metadata tests */
void test_lane_sharing_size(void);
void test_lane_sharing_is_extended(void);

/* Misalignment test */
void test_lane_sharing_misaligned_access(void);

void run_testsuite_lane_sharing(void);

#endif /* J2735_DE_INTERNAL_LANESHARING_TEST_H */
