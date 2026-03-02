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
#include "J2735_internal_DE_AllowedManeuvers_test.h"
#include "J2735_internal_DE_BrakeAppliedStatus_test.h"
#include "J2735_internal_DE_ExteriorLights_test.h"
#include "J2735_internal_DE_GNSSstatus_test.h"
#include "J2735_internal_DE_LaneDirection_test.h"
#include "J2735_internal_DE_LaneSharing_test.h"
#include "J2735_internal_DE_PersonalAssistive_test.h"
#include "J2735_internal_DE_PersonalDeviceUsageState_test.h"
#include "J2735_internal_DE_PublicSafetyAndRoadWorkerActivity_test.h"
#include "J2735_internal_DE_PublicSafetyDirectingTrafficSubType_test.h"
#include "J2735_internal_DE_TrafficLightOperationStatus_test.h"
#include "J2735_internal_DE_TransitStatus_test.h"
#include "J2735_internal_DE_UserSizeAndBehaviour_test.h"
#include "J2735_internal_DE_VehicleEventFlags_test.h"
#include "J2735_internal_DE_VerticalAccelerationThreshold_test.h"
#include "J2735_internal_DF_ApproachOrLane_test.h"
#include "J2735_internal_DF_BSMcoreData_test.h"
#include "J2735_internal_DF_IntersectionReferenceID_test.h"
#include "J2735_internal_DF_PathPrediction_test.h"

/* cppcheck-suppress unusedFunction ; Unity framework requirement */
void setUp(void) {}

/* cppcheck-suppress unusedFunction ; Unity framework requirement */
void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  run_testsuite_allowed_maneuvers();
  run_testsuite_approach_or_lane();
  run_testsuite_brake_applied_status();
  run_testsuite_bsm_core_data();
  run_testsuite_exterior_lights();
  run_testsuite_gnss_status();
  run_testsuite_intersection_reference_id();
  run_testsuite_lane_direction();
  run_testsuite_lane_sharing();
  run_testsuite_path_prediction();
  run_testsuite_personal_assistive();
  run_testsuite_personal_device_usage_state();
  run_testsuite_public_safety_and_road_worker_activity();
  run_testsuite_public_safety_directing_traffic_sub_type();
  run_testsuite_traffic_light_operation_status();
  run_testsuite_transit_status();
  run_testsuite_uper();
  run_testsuite_user_size_and_behaviour();
  run_testsuite_vehicle_event_flags();
  run_testsuite_vertical_acceleration_threshold();

  return UNITY_END();
}
