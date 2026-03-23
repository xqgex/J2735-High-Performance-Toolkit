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
 * @brief Internal Constant Definitions for J2735 Toolkit.
 */
#ifndef J2735_INTERNAL_CONSTANTS_H
#define J2735_INTERNAL_CONSTANTS_H

/* ============================================================================================== */
/*  Core Bit-Stream Arithmetic Constants                                                          */
/* ============================================================================================== */

/**
 * @internal
 * @brief Shift value for converting bit offset to byte offset (divide by 8 = 2^3).
 *
 * Used in J2735_READ_BITS: `buf[bit_offset >> 3]` gives the byte containing the bit.
 */
#define J2735_INTERNAL_BITS_TO_BYTE_SHIFT 3U

/**
 * @internal
 * @brief Mask for extracting sub-byte bit position (modulo 8).
 *
 * Used in J2735_READ_BITS: `bit_offset & 7` gives the bit position within the byte (0-7).
 */
#define J2735_INTERNAL_BIT_IN_BYTE_MASK 7U

/**
 * @internal
 * @brief Width of uint64_t in bits.
 *
 * Used for right-aligning extracted bits in J2735_READ_BITS.
 */
#define J2735_INTERNAL_U64_WIDTH_BITS 64U

/**
 * @internal
 * @brief Bits per byte (8). Used for open-type octet calculations.
 */
#define J2735_INTERNAL_BITS_PER_BYTE 8U

/* ============================================================================================== */
/*  UPER Length Determinant Constants (X.691 §11.9)                                               */
/* ============================================================================================== */

/**
 * @internal
 * @brief Total bits consumed by short-form length determinant (1 prefix + 7 value bits).
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_BITS 8U

/**
 * @internal
 * @brief Prefix bits in short-form length determinant ("0b0").
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_PREFIX_BITS 1U

/**
 * @internal
 * @brief Value bits in short-form length determinant.
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_VALUE_BITS 7U

_Static_assert(J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_BITS ==
                   (J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_PREFIX_BITS +
                    J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_VALUE_BITS),
               "Short-form length determinant bit counts must sum correctly");

/**
 * @internal
 * @brief Total bits consumed by long-form length determinant (2 prefix + 14 value bits).
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_LONG_BITS 16U

/**
 * @internal
 * @brief Prefix bits in long-form length determinant ("0b10").
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_LONG_PREFIX_BITS 2U

/**
 * @internal
 * @brief Value bits in long-form length determinant.
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_LONG_VALUE_BITS 14U

_Static_assert(J2735_INTERNAL_LENGTH_DETERMINANT_LONG_BITS ==
                   (J2735_INTERNAL_LENGTH_DETERMINANT_LONG_PREFIX_BITS +
                    J2735_INTERNAL_LENGTH_DETERMINANT_LONG_VALUE_BITS),
               "Long-form length determinant bit counts must sum correctly");

/**
 * @internal
 * @brief Mask to check if length determinant is short-form (bit 1 of 2-bit read).
 *
 * Length determinant encoding (X.691 §11.9):
 *   - 0xxxxxxx: short-form (bit[0]=0)
 *   - 10xxxxxx: long-form (bit[0]=1, bit[1]=0)
 *   - 11xxxxxx: fragmented (bit[0]=1, bit[1]=1)
 *
 * After reading 2 bits: if (two_bits & 2) == 0, bit[0]=0 -> short-form.
 */
#define J2735_INTERNAL_LENGTH_DETERMINANT_SHORT_FORM_MASK 2U

/* ============================================================================================== */
/*  UPER Normally-Small-Non-Negative-Whole-Number Constants (X.691 §11.6)                         */
/* ============================================================================================== */

/**
 * @internal
 * @brief Prefix bits in nsnnwn (1 bit: 0=small form, 1=large form).
 */
#define J2735_INTERNAL_NSNNWN_PREFIX_BITS 1U

/**
 * @internal
 * @brief Total bits consumed by small-form nsnnwn (1 prefix + 6 value bits).
 */
#define J2735_INTERNAL_NSNNWN_SMALL_BITS 7U

/**
 * @internal
 * @brief Value bits in small-form nsnnwn.
 */
#define J2735_INTERNAL_NSNNWN_SMALL_VALUE_BITS 6U

_Static_assert(J2735_INTERNAL_NSNNWN_SMALL_BITS ==
                   (J2735_INTERNAL_NSNNWN_SMALL_VALUE_BITS + J2735_INTERNAL_NSNNWN_PREFIX_BITS),
               "Small-form nsnnwn bit counts must sum correctly");

/**
 * @internal
 * @brief Maximum octets for large-form nsnnwn that fit in uint32_t.
 */
#define J2735_INTERNAL_MAX_NSNNWN_OCTETS 4U

/* ============================================================================================== */
/*  Extension Handling Constants                                                                  */
/* ============================================================================================== */

/**
 * @internal
 * @brief Width of the extension marker bit in bits.
 *
 * Per X.691 §21.5, extensible types have a single extension marker bit at position 0:
 *   - 0 = root component only (no extensions present)
 *   - 1 = extensions are present
 *
 * This constant is used in wire size calculations: total = EXT_MARKER + payload.
 */
#define J2735_INTERNAL_EXTENSION_MARKER_BITS 1U

/**
 * @brief Maximum number of extension additions supported.
 *
 * X.691 extension bitmaps are read into a uint64_t, which holds 64 bits.
 * The bitmap has (count + 1) bits, so max count is 63.
 */
#define J2735_MAX_EXTENSION_ADDITIONS 63U

/* ============================================================================================== */
/*  J2735 BIT-WIDTH CONSTANTS (Spec: J2735_202409)                                                */
/*  Required for O(1) bit-stream navigation.                                                      */
/*  Constraint: All bit-widths must satisfy 1 <= width <= 56 for use with J2735_READ_BITS.        */
/* ============================================================================================== */
#define J2735_BW_64_B 63U /* 64b: SEQUENCE (2 fields) */
#define J2735_BW_ACCEL_STEER_YAW_RATE_CONFIDENCE                                                   \
  8U /* AccelSteerYawRateConfidence: SEQUENCE (3 fields) */
#define J2735_BW_ACCELERATION              12U  /* Acceleration: INTEGER (-2000..2001) */
#define J2735_BW_ACCELERATION_CONFIDENCE   3U   /* AccelerationConfidence: ENUMERATED (5 values) */
#define J2735_BW_ACCELERATION_SET_4_WAY    48U  /* AccelerationSet4Way: SEQUENCE (4 fields) */
#define J2735_BW_ACTUATED_INTERVAL         1U   /* ActuatedInterval: BOOLEAN */
#define J2735_BW_ADVISORY_SPEED_TYPE       2U   /* AdvisorySpeedType: ENUMERATED (4 values) */
#define J2735_BW_ALLOWED_MANEUVERS         12U  /* AllowedManeuvers: BIT STRING (SIZE(12)) */
#define J2735_BW_AMBIENT_AIR_PRESSURE      8U   /* AmbientAirPressure: INTEGER (0..255) */
#define J2735_BW_AMBIENT_AIR_TEMPERATURE   8U   /* AmbientAirTemperature: INTEGER (0..191) */
#define J2735_BW_ANGLE                     15U  /* Angle: INTEGER (0..28800) */
#define J2735_BW_ANIMAL_PROPELLED_TYPE     2U   /* AnimalPropelledType: ENUMERATED (4 values) */
#define J2735_BW_ANIMAL_TYPE               2U   /* AnimalType: ENUMERATED (4 values) */
#define J2735_BW_ANTI_LOCK_BRAKE_STATUS    2U   /* AntiLockBrakeStatus: ENUMERATED (4 values) */
#define J2735_BW_APPROACH_ID               4U   /* ApproachID: INTEGER (0..15) */
#define J2735_BW_APPROACH_OR_LANE          1U   /* ApproachOrLane: CHOICE (2 alternatives) */
#define J2735_BW_ASPHALT_OR_TAR            2U   /* AsphaltOrTar: SEQUENCE (1 fields) */
#define J2735_BW_ASPHALT_OR_TAR_TYPE       2U   /* AsphaltOrTarType: ENUMERATED (4 values) */
#define J2735_BW_ATTACHMENT                3U   /* Attachment: ENUMERATED (7 values) */
#define J2735_BW_ATTACHMENT_RADIUS         8U   /* AttachmentRadius: INTEGER (0..200) */
#define J2735_BW_AUXILIARY_BRAKE_STATUS    2U   /* AuxiliaryBrakeStatus: ENUMERATED (4 values) */
#define J2735_BW_AXLE_LOCATION             8U   /* AxleLocation: INTEGER (0..255) */
#define J2735_BW_AXLE_WEIGHT               16U  /* AxleWeight: INTEGER (0..64255) */
#define J2735_BW_BSM_CORE_DATA             290U /* BSMcoreData: SEQUENCE (14 fields) */
#define J2735_BW_BASIC_VEHICLE_CLASS       8U   /* BasicVehicleClass: INTEGER (0..255) */
#define J2735_BW_BASIC_VEHICLE_ROLE        5U   /* BasicVehicleRole: ENUMERATED (23 values) */
#define J2735_BW_BRAKE_APPLIED_PRESSURE    4U   /* BrakeAppliedPressure: ENUMERATED (16 values) */
#define J2735_BW_BRAKE_APPLIED_STATUS      5U   /* BrakeAppliedStatus: BIT STRING (SIZE(5)) */
#define J2735_BW_BRAKE_BOOST_APPLIED       2U   /* BrakeBoostApplied: ENUMERATED (3 values) */
#define J2735_BW_BRAKE_SYSTEM_STATUS       15U  /* BrakeSystemStatus: SEQUENCE (6 fields) */
#define J2735_BW_BUMPER_HEIGHT             7U   /* BumperHeight: INTEGER (0..127) */
#define J2735_BW_BUMPER_HEIGHTS            14U  /* BumperHeights: SEQUENCE (2 fields) */
#define J2735_BW_CARGO_WEIGHT              16U  /* CargoWeight: INTEGER (0..64255) */
#define J2735_BW_CINDERS                   1U   /* Cinders: SEQUENCE (1 fields) */
#define J2735_BW_CINDERS_TYPE              1U   /* CindersType: ENUMERATED (1 values) */
#define J2735_BW_COARSE_HEADING            8U   /* CoarseHeading: INTEGER (0..240) */
#define J2735_BW_COEFFICIENT_OF_FRICTION   6U   /* CoefficientOfFriction: INTEGER (0..50) */
#define J2735_BW_CONFIDENCE                8U   /* Confidence: INTEGER (0..200) */
#define J2735_BW_CONSECUTIVE_TRAFFIC_LIGHT 2U   /* ConsecutiveTrafficLight: ENUMERATED (3 values) */
#define J2735_BW_COUNT                     6U   /* Count: INTEGER (0..32) */
#define J2735_BW_D_DATE                    21U  /* DDate: SEQUENCE (3 fields) */
#define J2735_BW_D_DAY                     5U   /* DDay: INTEGER (0..31) */
#define J2735_BW_D_FULL_TIME               32U  /* DFullTime: SEQUENCE (5 fields) */
#define J2735_BW_D_HOUR                    5U   /* DHour: INTEGER (0..31) */
#define J2735_BW_D_MINUTE                  6U   /* DMinute: INTEGER (0..60) */
#define J2735_BW_D_MONTH                   4U   /* DMonth: INTEGER (0..12) */
#define J2735_BW_D_MONTH_DAY               9U   /* DMonthDay: SEQUENCE (2 fields) */
#define J2735_BW_D_OFFSET                  11U  /* DOffset: INTEGER (-840..840) */
#define J2735_BW_DSRC_MSG_ID               15U  /* DSRCmsgID: INTEGER (0..32767) */
#define J2735_BW_D_SECOND                  16U  /* DSecond: INTEGER (0..65535) */
#define J2735_BW_D_YEAR                    12U  /* DYear: INTEGER (0..4095) */
#define J2735_BW_D_YEAR_MONTH              16U  /* DYearMonth: SEQUENCE (2 fields) */
#define J2735_BW_DELTA_ANGLE               9U   /* DeltaAngle: INTEGER (-150..150) */
#define J2735_BW_DELTA_TIME                8U   /* DeltaTime: INTEGER (-122..121) */
#define J2735_BW_DIRECTION_OF_USE          2U   /* DirectionOfUse: ENUMERATED (4 values) */
#define J2735_BW_DISTANCE_UNITS            3U   /* DistanceUnits: ENUMERATED (8 values) */
#define J2735_BW_DRIVE_AXLE_LIFT_AIR_PRESSURE                                                      \
  10U                                         /* DriveAxleLiftAirPressure: INTEGER (0..1000)       \
                                               */
#define J2735_BW_DRIVE_AXLE_LOCATION      8U  /* DriveAxleLocation: INTEGER (0..255) */
#define J2735_BW_DRIVE_AXLE_LUBE_PRESSURE 8U  /* DriveAxleLubePressure: INTEGER (0..250) */
#define J2735_BW_DRIVE_AXLE_TEMPERATURE   8U  /* DriveAxleTemperature: INTEGER (-40..210) */
#define J2735_BW_DRIVEN_LINE_OFFSET_LG    16U /* DrivenLineOffsetLg: INTEGER (-32767..32767) */
#define J2735_BW_DRIVEN_LINE_OFFSET_SM    12U /* DrivenLineOffsetSm: INTEGER (-2047..2047) */
#define J2735_BW_DRIVING_WHEEL_ANGLE      8U  /* DrivingWheelAngle: INTEGER (-128..127) */
#define J2735_BW_DURATION                 12U /* Duration: INTEGER (0..3600) */
#define J2735_BW_ELEVATION                16U /* Elevation: INTEGER (-4096..61439) */
#define J2735_BW_ELEVATION_CONFIDENCE     3U  /* ElevationConfidence: ENUMERATED (8 values) */
#define J2735_BW_EXTENT                   4U  /* Extent: ENUMERATED (16 values) */
#define J2735_BW_EXTERIOR_LIGHTS          9U  /* ExteriorLights: BIT STRING (SIZE(9)) */
#define J2735_BW_FUEL_TYPE                4U  /* FuelType: INTEGER (0..15) */
#define J2735_BW_FURTHER_INFO_ID          16U /* FurtherInfoID: OCTET STRING (SIZE(2)) */
#define J2735_BW_GNSS_STATUS              8U  /* GNSSstatus: BIT STRING (SIZE(8)) */
#define J2735_BW_GRASS                    1U  /* Grass: SEQUENCE (1 fields) */
#define J2735_BW_GRASS_TYPE               1U  /* GrassType: ENUMERATED (1 values) */
#define J2735_BW_GRAVEL                   1U  /* Gravel: SEQUENCE (1 fields) */
#define J2735_BW_GRAVEL_TYPE              1U  /* GravelType: ENUMERATED (2 values) */
#define J2735_BW_GROSS_DISTANCE           10U /* GrossDistance: INTEGER (0..1023) */
#define J2735_BW_GROSS_SPEED              5U  /* GrossSpeed: INTEGER (0..31) */
#define J2735_BW_HEADING                  15U /* Heading: INTEGER (0..28800) */
#define J2735_BW_HEADING_CONFIDENCE       3U  /* HeadingConfidence: ENUMERATED (8 values) */
#define J2735_BW_HEADING_SLICE            16U /* HeadingSlice: BIT STRING (SIZE(16)) */
#define J2735_BW_HUMAN_PROPELLED_TYPE     3U  /* HumanPropelledType: ENUMERATED (6 values) */
#define J2735_BW_ICE                      1U  /* Ice: SEQUENCE (1 fields) */
#define J2735_BW_ICE_TYPE                 1U  /* IceType: ENUMERATED (1 values) */
#define J2735_BW_INTERSECTION_ID          16U /* IntersectionID: INTEGER (0..65535) */
#define J2735_BW_INTERSECTION_STATUS_OBJECT                                                        \
  16U                                         /* IntersectionStatusObject: BIT STRING (SIZE(16)) */
#define J2735_BW_IS_DOLLY                 1U  /* IsDolly: BOOLEAN */
#define J2735_BW_ISO_3833_VEHICLE_TYPE    7U  /* Iso3833VehicleType: INTEGER (0..100) */
#define J2735_BW_LANE_CONNECTION_ID       8U  /* LaneConnectionID: INTEGER (0..255) */
#define J2735_BW_LANE_DATA_ATTRIBUTE      3U  /* LaneDataAttribute: CHOICE (8 alternatives) */
#define J2735_BW_LANE_DIRECTION           2U  /* LaneDirection: BIT STRING (SIZE(2)) */
#define J2735_BW_LANE_ID                  8U  /* LaneID: INTEGER (0..255) */
#define J2735_BW_LANE_SHARING             10U /* LaneSharing: BIT STRING (SIZE(10)) */
#define J2735_BW_LANE_WIDTH               15U /* LaneWidth: INTEGER (0..32767) */
#define J2735_BW_LATITUDE                 31U /* Latitude: INTEGER (-900000000..900000001) */
#define J2735_BW_LAYER_ID                 7U  /* LayerID: INTEGER (0..100) */
#define J2735_BW_LAYER_TYPE               3U  /* LayerType: ENUMERATED (7 values) */
#define J2735_BW_LIGHTBAR_IN_USE          3U  /* LightbarInUse: ENUMERATED (8 values) */
#define J2735_BW_LONGITUDE                32U /* Longitude: INTEGER (-1799999999..1800000001) */
#define J2735_BW_MUTCD_CODE               3U  /* MUTCDCode: ENUMERATED (7 values) */
#define J2735_BW_MEAN_VARIATION           15U /* MeanVariation: INTEGER (0..25000) */
#define J2735_BW_MERGE_DIVERGE_NODE_ANGLE 9U  /* MergeDivergeNodeAngle: INTEGER (-180..180) */
#define J2735_BW_MINUTE_OF_THE_YEAR       20U /* MinuteOfTheYear: INTEGER (0..527040) */
#define J2735_BW_MINUTES_DURATION         15U /* MinutesDuration: INTEGER (0..32000) */
#define J2735_BW_MOTORIZED_PROPELLED_TYPE 3U  /* MotorizedPropelledType: ENUMERATED (6 values) */
#define J2735_BW_MOVEMENT_PHASE_STATE     3U  /* MovementPhaseState: ENUMERATED (8 values) */
#define J2735_BW_MSG_CRC                  16U /* MsgCRC: OCTET STRING (SIZE(2)) */
#define J2735_BW_MSG_COUNT                7U  /* MsgCount: INTEGER (0..127) */
#define J2735_BW_MULTI_VEHICLE_RESPONSE   2U  /* MultiVehicleResponse: ENUMERATED (4 values) */
#define J2735_BW_NODE_ATTRIBUTE_LL        1U  /* NodeAttributeLL: ENUMERATED (2 values) */
#define J2735_BW_NODE_ATTRIBUTE_XY        1U  /* NodeAttributeXY: ENUMERATED (2 values) */
#define J2735_BW_NODE_OFFSET_POINT_LL     4U  /* NodeOffsetPointLL: CHOICE (14 alternatives) */
#define J2735_BW_NODE_OFFSET_POINT_XY     4U  /* NodeOffsetPointXY: CHOICE (10 alternatives) */
#define J2735_BW_NUMBER_OF_PARTICIPANTS_IN_CLUSTER                                                 \
  2U /* NumberOfParticipantsInCluster: ENUMERATED (4 values) */
#define J2735_BW_OBJECT_COUNT                  10U /* ObjectCount: INTEGER (0..1023) */
#define J2735_BW_OBSTACLE_DIRECTION            15U /* ObstacleDirection */
#define J2735_BW_OBSTACLE_DISTANCE             15U /* ObstacleDistance: INTEGER (0..32767) */
#define J2735_BW_PATH_PREDICTION               24U /* PathPrediction: SEQUENCE (2 fields) */
#define J2735_BW_PEDESTRIAN_BICYCLE_DETECT     1U  /* PedestrianBicycleDetect: BOOLEAN */
#define J2735_BW_PEDESTRIAN_CALL               1U  /* PedestrianCall: BOOLEAN */
#define J2735_BW_PERMISSIVE_NON_PROTECTED      1U /* PermissiveNonProtected: ENUMERATED (2 values) */
#define J2735_BW_PERSONAL_ASSISTIVE            6U /* PersonalAssistive: BIT STRING (SIZE(6)) */
#define J2735_BW_PERSONAL_CLUSTER_RADIUS       7U /* PersonalClusterRadius: INTEGER (0..100) */
#define J2735_BW_PERSONAL_CROSSING_IN_PROGRESS 1U /* PersonalCrossingInProgress: BOOLEAN */
#define J2735_BW_PERSONAL_CROSSING_REQUEST     1U /* PersonalCrossingRequest: BOOLEAN */
#define J2735_BW_PERSONAL_DEVICE_USAGE_STATE                                                       \
  9U                                           /* PersonalDeviceUsageState: BIT STRING (SIZE(9))   \
                                                */
#define J2735_BW_PERSONAL_DEVICE_USER_TYPE 3U  /* PersonalDeviceUserType: ENUMERATED (5 values) */
#define J2735_BW_PIVOTING_ALLOWED          1U  /* PivotingAllowed: BOOLEAN */
#define J2735_BW_PORTLAND_CEMENT           2U  /* PortlandCement: SEQUENCE (1 fields) */
#define J2735_BW_PORTLAND_CEMENT_TYPE      2U  /* PortlandCementType: ENUMERATED (3 values) */
#define J2735_BW_POSITION_CONFIDENCE       4U  /* PositionConfidence: ENUMERATED (16 values) */
#define J2735_BW_POSITION_CONFIDENCE_SET   7U  /* PositionConfidenceSet: SEQUENCE (2 fields) */
#define J2735_BW_POSITIONAL_ACCURACY       32U /* PositionalAccuracy: SEQUENCE (3 fields) */
#define J2735_BW_PRIORITIZATION_RESPONSE_STATUS                                                    \
  3U                                       /* PrioritizationResponseStatus: ENUMERATED (8 values) */
#define J2735_BW_PRIORITY               8U /* Priority: OCTET STRING (SIZE(1)) */
#define J2735_BW_PRIORITY_REQUEST_TYPE  2U /* PriorityRequestType: ENUMERATED (4 values) */
#define J2735_BW_PRIVILEGED_EVENT_FLAGS 16U /* PrivilegedEventFlags: BIT STRING (SIZE(16)) */
#define J2735_BW_PRIVILEGED_EVENTS      21U /* PrivilegedEvents: SEQUENCE (2 fields) */
#define J2735_BW_PROBE_SEGMENT_NUMBER   15U /* ProbeSegmentNumber: INTEGER (0..32767) */
#define J2735_BW_PROPELLED_INFORMATION  2U  /* PropelledInformation: CHOICE (3 alternatives) */
#define J2735_BW_PUBLIC_SAFETY_AND_ROAD_WORKER_ACTIVITY                                            \
  6U /* PublicSafetyAndRoadWorkerActivity: BIT STRING (SIZE(6)) */
#define J2735_BW_PUBLIC_SAFETY_DIRECTING_TRAFFIC_SUB_TYPE                                          \
  7U /* PublicSafetyDirectingTrafficSubType: BIT STRING (SIZE(7)) */
#define J2735_BW_PUBLIC_SAFETY_EVENT_RESPONDER_WORKER_TYPE                                         \
  3U /* PublicSafetyEventResponderWorkerType: ENUMERATED (8 values) */
#define J2735_BW_RADIUS_OF_CURVATURE      16U /* RadiusOfCurvature: INTEGER (-32767..32767) */
#define J2735_BW_RAIN_SENSOR              3U  /* RainSensor: ENUMERATED (8 values) */
#define J2735_BW_REGION_ID                8U  /* RegionId: INTEGER (0..255) */
#define J2735_BW_REGULATORY_SPEED_LIMIT   16U /* RegulatorySpeedLimit: SEQUENCE (2 fields) */
#define J2735_BW_REQUEST_ID               8U  /* RequestID: INTEGER (0..255) */
#define J2735_BW_REQUEST_IMPORTANCE_LEVEL 4U  /* RequestImportanceLevel: ENUMERATED (16 values) */
#define J2735_BW_REQUEST_SUB_ROLE         4U  /* RequestSubRole: ENUMERATED (16 values) */
#define J2735_BW_REQUESTED_ITEM           1U  /* RequestedItem: ENUMERATED (2 values) */
#define J2735_BW_RESERVED_BIT             1U  /* ReservedBit: BOOLEAN */
#define J2735_BW_RESPONSE_TYPE            3U  /* ResponseType: ENUMERATED (7 values) */
#define J2735_BW_RESTRICTION_APPLIES_TO   4U  /* RestrictionAppliesTo: ENUMERATED (9 values) */
#define J2735_BW_RESTRICTION_CLASS_ID     8U  /* RestrictionClassID: INTEGER (0..255) */
#define J2735_BW_RESTRICTION_USER_TYPE    1U  /* RestrictionUserType: CHOICE (2 alternatives) */
#define J2735_BW_ROAD_REGULATOR_ID        16U /* RoadRegulatorID: INTEGER (0..65535) */
#define J2735_BW_ROAD_SEGMENT_ID          16U /* RoadSegmentID: INTEGER (0..65535) */
#define J2735_BW_ROAD_SURFACE_CONDITION   1U  /* RoadSurfaceCondition: ENUMERATED (2 values) */
#define J2735_BW_ROADWAY_CROWN_ANGLE      8U  /* RoadwayCrownAngle: INTEGER (-128..127) */
#define J2735_BW_ROCK                     1U  /* Rock: SEQUENCE (1 fields) */
#define J2735_BW_ROCK_TYPE                1U  /* RockType: ENUMERATED (1 values) */
#define J2735_BW_SSP_INDEX                5U  /* SSPindex: INTEGER (0..31) */
#define J2735_BW_SECOND_OF_TIME           6U  /* SecondOfTime: INTEGER (0..61) */
#define J2735_BW_SEGMENT_ATTRIBUTE_LL     4U  /* SegmentAttributeLL: ENUMERATED (10 values) */
#define J2735_BW_SEGMENT_ATTRIBUTE_XY     4U  /* SegmentAttributeXY: ENUMERATED (10 values) */
#define J2735_BW_SEMI_MAJOR_AXIS_ACCURACY 8U  /* SemiMajorAxisAccuracy: INTEGER (0..255) */
#define J2735_BW_SEMI_MAJOR_AXIS_ORIENTATION                                                       \
  16U                                          /* SemiMajorAxisOrientation: INTEGER (0..65535)     \
                                                */
#define J2735_BW_SEMI_MINOR_AXIS_ACCURACY  8U  /* SemiMinorAxisAccuracy: INTEGER (0..255) */
#define J2735_BW_SIGN_PRIORITY             3U  /* SignPriority: INTEGER (0..7) */
#define J2735_BW_SIGNAL_GROUP_ID           8U  /* SignalGroupID: INTEGER (0..255) */
#define J2735_BW_SIGNAL_REQ_SCHEME         8U  /* SignalReqScheme: OCTET STRING (SIZE(1)) */
#define J2735_BW_SIREN_IN_USE              2U  /* SirenInUse: ENUMERATED (4 values) */
#define J2735_BW_SNAPSHOT_DISTANCE         30U /* SnapshotDistance: SEQUENCE (4 fields) */
#define J2735_BW_SNAPSHOT_TIME             22U /* SnapshotTime: SEQUENCE (4 fields) */
#define J2735_BW_SNOW                      1U  /* Snow: SEQUENCE (1 fields) */
#define J2735_BW_SNOW_TYPE                 1U  /* SnowType: ENUMERATED (2 values) */
#define J2735_BW_SPEED                     13U /* Speed: INTEGER (0..8191) */
#define J2735_BW_SPEED_ADVICE              9U  /* SpeedAdvice: INTEGER (0..500) */
#define J2735_BW_SPEED_CONFIDENCE          3U  /* SpeedConfidence: ENUMERATED (8 values) */
#define J2735_BW_SPEED_LIMIT_TYPE          3U  /* SpeedLimitType: ENUMERATED (8 values) */
#define J2735_BW_SPEED_PROFILE_MEASUREMENT 5U  /* SpeedProfileMeasurement */
#define J2735_BW_SPEEDAND_HEADINGAND_THROTTLE_CONFIDENCE                                           \
  8U /* SpeedandHeadingandThrottleConfidence: SEQUENCE (3 fields) */
#define J2735_BW_STABILITY_CONTROL_STATUS    2U  /* StabilityControlStatus: ENUMERATED (4 values) */
#define J2735_BW_STATION_ID                  32U /* StationID: INTEGER (0..4294967295) */
#define J2735_BW_STEERING_AXLE_LUBE_PRESSURE 8U  /* SteeringAxleLubePressure: INTEGER (0..250) */
#define J2735_BW_STEERING_AXLE_TEMPERATURE   8U  /* SteeringAxleTemperature: INTEGER (-40..210) */
#define J2735_BW_STEERING_WHEEL_ANGLE        8U  /* SteeringWheelAngle: INTEGER (-126..127) */
#define J2735_BW_STEERING_WHEEL_ANGLE_CONFIDENCE                                                   \
  2U /* SteeringWheelAngleConfidence: ENUMERATED (4 values) */
#define J2735_BW_STEERING_WHEEL_ANGLE_RATE_OF_CHANGE                                               \
  8U /* SteeringWheelAngleRateOfChange: INTEGER (-127..127) */
#define J2735_BW_SUN_SENSOR               10U /* SunSensor: INTEGER (0..1000) */
#define J2735_BW_TEMPORARY_ID             32U /* TemporaryID: OCTET STRING (SIZE(4)) */
#define J2735_BW_TERM_DISTANCE            15U /* TermDistance: INTEGER (1..30000) */
#define J2735_BW_TERM_TIME                11U /* TermTime: INTEGER (1..1800) */
#define J2735_BW_THROTTLE_CONFIDENCE      2U  /* ThrottleConfidence: ENUMERATED (4 values) */
#define J2735_BW_THROTTLE_POSITION        8U  /* ThrottlePosition: INTEGER (0..200) */
#define J2735_BW_TIME_CONFIDENCE          5U  /* TimeConfidence: ENUMERATED (17 values) */
#define J2735_BW_TIME_INTERVAL_CONFIDENCE 4U  /* TimeIntervalConfidence: INTEGER (0..15) */
#define J2735_BW_TIME_MARK                16U /* TimeMark: INTEGER (0..36111) */
#define J2735_BW_TIME_OFFSET              16U /* TimeOffset: INTEGER (1..65535) */
#define J2735_BW_TIRE_LEAKAGE_RATE        16U /* TireLeakageRate: INTEGER (0..64255) */
#define J2735_BW_TIRE_LOCATION            8U  /* TireLocation: INTEGER (0..255) */
#define J2735_BW_TIRE_PRESSURE            8U  /* TirePressure: INTEGER (0..250) */
#define J2735_BW_TIRE_PRESSURE_THRESHOLD_DETECTION                                                 \
  3U /* TirePressureThresholdDetection: ENUMERATED (8 values) */
#define J2735_BW_TIRE_TEMP               16U /* TireTemp: INTEGER (-8736..55519) */
#define J2735_BW_TRACTION_CONTROL_STATUS 2U  /* TractionControlStatus: ENUMERATED (4 values) */
#define J2735_BW_TRAFFIC_LIGHT_CONTROLLER_STATUS                                                   \
  8U /* TrafficLightControllerStatus: BIT STRING (SIZE(8)) */
#define J2735_BW_TRAFFIC_LIGHT_DIRECTION_CODE 9U /* TrafficLightDirectionCode: INTEGER (0..359) */
#define J2735_BW_TRAFFIC_LIGHT_ID             1U /* TrafficLightID: CHOICE (2 alternatives) */
#define J2735_BW_TRAFFIC_LIGHT_INTERVAL_TYPE                                                       \
  1U /* TrafficLightIntervalType: ENUMERATED (2 values) */
#define J2735_BW_TRAFFIC_LIGHT_OPERATION_STATUS                                                    \
  8U /* TrafficLightOperationStatus: BIT STRING (SIZE(8)) */
#define J2735_BW_TRAFFIC_LIGHT_TYPE        3U  /* TrafficLightType: ENUMERATED (6 values) */
#define J2735_BW_TRAFFIC_LIGHTING_STATUS   2U  /* TrafficLightingStatus: ENUMERATED (3 values) */
#define J2735_BW_TRAILER_MASS              8U  /* TrailerMass: INTEGER (0..255) */
#define J2735_BW_TRAILER_WEIGHT            16U /* TrailerWeight: INTEGER (0..64255) */
#define J2735_BW_TRANSIT_STATUS            6U  /* TransitStatus: BIT STRING (SIZE(6)) */
#define J2735_BW_TRANSIT_VEHICLE_OCCUPANCY 3U  /* TransitVehicleOccupancy: ENUMERATED (8 values) */
#define J2735_BW_TRANSIT_VEHICLE_STATUS    8U  /* TransitVehicleStatus: BIT STRING (SIZE(8)) */
#define J2735_BW_TRANSMISSION_AND_SPEED    16U /* TransmissionAndSpeed: SEQUENCE (2 fields) */
#define J2735_BW_TRANSMISSION_STATE        3U  /* TransmissionState: ENUMERATED (8 values) */
#define J2735_BW_TRAVELER_INFO_TYPE        2U  /* TravelerInfoType: ENUMERATED (4 values) */
#define J2735_BW_UNIQUE_MSGID              72U /* UniqueMSGID: OCTET STRING (SIZE(9)) */
#define J2735_BW_USER_SIZE_AND_BEHAVIOUR   5U  /* UserSizeAndBehaviour: BIT STRING (SIZE(5)) */
#define J2735_BW_VARIATION_STD_DEV         12U /* VariationStdDev: INTEGER (0..2500) */
#define J2735_BW_VEHICLE_HEIGHT            7U  /* VehicleHeight: INTEGER (0..127) */
#define J2735_BW_VEHICLE_ID                1U  /* VehicleID: CHOICE (2 alternatives) */
#define J2735_BW_VEHICLE_LENGTH            12U /* VehicleLength: INTEGER (0..4095) */
#define J2735_BW_VEHICLE_MASS              8U  /* VehicleMass: INTEGER (0..255) */
#define J2735_BW_VEHICLE_SIZE              22U /* VehicleSize: SEQUENCE (2 fields) */
#define J2735_BW_VEHICLE_STATUS_DEVICE_TYPE_TAG                                                    \
  5U                                       /* VehicleStatusDeviceTypeTag: ENUMERATED (32 values) */
#define J2735_BW_VEHICLE_TYPE          5U  /* VehicleType: ENUMERATED (17 values) */
#define J2735_BW_VEHICLE_WIDTH         10U /* VehicleWidth: INTEGER (0..1023) */
#define J2735_BW_VELOCITY              13U /* Velocity: INTEGER (0..8191) */
#define J2735_BW_VERTICAL_ACCELERATION 8U  /* VerticalAcceleration: INTEGER (-127..127) */
#define J2735_BW_VERTICAL_ACCELERATION_THRESHOLD                                                   \
  5U                                       /* VerticalAccelerationThreshold: BIT STRING (SIZE(5)) */
#define J2735_BW_VERTICAL_OFFSET       4U  /* VerticalOffset: CHOICE (9 alternatives) */
#define J2735_BW_WAIT_ON_STOPLINE      1U  /* WaitOnStopline: BOOLEAN */
#define J2735_BW_WHEEL_END_ELECT_FAULT 2U  /* WheelEndElectFault: ENUMERATED (4 values) */
#define J2735_BW_WHEEL_SENSOR_STATUS   2U  /* WheelSensorStatus: ENUMERATED (4 values) */
#define J2735_BW_WIPER_RATE            7U  /* WiperRate: INTEGER (0..127) */
#define J2735_BW_WIPER_STATUS          3U  /* WiperStatus: ENUMERATED (7 values) */
#define J2735_BW_YAW_RATE              16U /* YawRate: INTEGER (-32767..32767) */
#define J2735_BW_YAW_RATE_CONFIDENCE   3U  /* YawRateConfidence: ENUMERATED (5 values) */
#define J2735_BW_ZONE_LENGTH           14U /* ZoneLength: INTEGER (0..10000) */
#define J2735_BW_ZOOM                  4U  /* Zoom: INTEGER (0..15) */

/* ============================================================================================== */
/*  Container Size Constants (Bytes)                                                              */
/* ============================================================================================== */
#define J2735_SIZE_BSM_CORE_DATA 37U /* BSMcoreData: 290 bits = 37 bytes (ceil(290/8)) */
#define J2735_SIZE_SPAT_HEADER   4U

#endif /* J2735_INTERNAL_CONSTANTS_H */
