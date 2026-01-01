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
 * @brief Unit tests for the J2735 High-Performance Toolkit.
 */

#include <inttypes.h>
#include <stdint.h>

#include "unity.h"
#include "unity_internals.h"

#include "J2735_UPER_test.h"
#include "J2735_internal_DE_VehicleEventFlags_test.h"
#include "J2735_internal_DF_BSMcoreData_test.h"
#include "J2735_internal_DF_IntersectionReferenceID_test.h"
#include "J2735_internal_DF_PathPrediction_test.h"

/* cppcheck-suppress unusedFunction ; Unity framework requirement */
void setUp(void) {}

/* cppcheck-suppress unusedFunction ; Unity framework requirement */
void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  run_testsuite_bsm_core_data();
  run_testsuite_intersection_reference_id();
  run_testsuite_path_prediction();
  run_testsuite_uper();
  run_testsuite_vehicle_event_flags();
  return UNITY_END();
}
