# Copyright 2026 Yogev Neumann
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Yogev Neumann
"""Shared test fixtures and utilities for J2735 Python tests.

This module provides:
    - SPEC_FILE_PATH: Path to the J2735 specification file
    - Mock spec builders used across multiple test modules
"""

from pathlib import Path
from unittest import TestCase

from tools.j2735_c_generator_jinja import (
    create_jinja_env,
    get_template,
)
from tools.j2735_spec_constraints import (
    BitStringConstraint,
    IntegerConstraint,
    SequenceField,
    SequenceOfType,
    SequenceType,
)
from tools.j2735_spec_parser import (
    ASN1TypeClass,
    ASN1TypeDefinition,
    J2735Specification,
    parse_spec_file,
)

# =============================================================================
# Constants
# =============================================================================


# Path to the J2735 specification file (relative to repository root)
_REPOSITORY_ROOT: Path = Path(__file__).parent.parent.parent
SPEC_FILE_PATH: Path = _REPOSITORY_ROOT / "J2735_202409_pdf_content.txt"


# BIT STRING Data Element types that have no UPER extension marker on the wire.
# These types take a different template code path than extensible types: no
# extension bit, no nsnnwn length field, and SIZE == ROOT_SIZE (not 1+ROOT_SIZE).
# Each tuple is (ASN.1 type name, C macro prefix).
NON_EXTENSIBLE_BITSTRING_TYPES: list[tuple[str, str]] = [
    ("LaneDirection", "LANE_DIRECTION"),
    ("GNSSstatus", "GNSS_STATUS"),
    ("AllowedManeuvers", "ALLOWED_MANEUVERS"),
    ("BrakeAppliedStatus", "BRAKE_APPLIED_STATUS"),
    ("TransitStatus", "TRANSIT_STATUS"),
    ("LaneSharing", "LANE_SHARING"),
    ("VerticalAccelerationThreshold", "VERTICAL_ACCELERATION_THRESHOLD"),
]


# =============================================================================
# Mock Spec Builders
# =============================================================================
#
# These functions create lightweight, self-contained J2735Specification objects
# for unit testing.
#
# =============================================================================


def make_nested_mock_spec() -> J2735Specification:
    """Create a mock spec with a SEQUENCE containing nested INTEGER fields.

    This models the real J2735 PositionalAccuracy type, which looks like:

        PositionalAccuracy ::= SEQUENCE {
            semiMajor    SemiMajorAxisAccuracy,    -- 8 bits (0..255)
            semiMinor    SemiMinorAxisAccuracy,    -- 8 bits (0..255)
            orientation  SemiMajorAxisOrientation  -- 16 bits (0..65535)
        }

    Wire format (32 bits total, no preamble since no OPTIONAL/extensible):
        +----------+----------+------------------+
        | semiMajor| semiMinor|   orientation    |
        |  8 bits  |  8 bits  |     16 bits      |
        +----------+----------+------------------+

    Returns:
        A J2735Specification containing PositionalAccuracy and its child types.
    """
    # Step 1: Define constraints for each primitive INTEGER field.
    # These determine the bit-width: 0..255 needs 8 bits, 0..65535 needs 16 bits.
    semi_major_constraint = IntegerConstraint(min_value=0, max_value=255)
    semi_minor_constraint = IntegerConstraint(min_value=0, max_value=255)
    orientation_constraint = IntegerConstraint(min_value=0, max_value=65535)

    # Step 2: Create ASN1TypeDefinition objects for each child type.
    # These represent the Data Elements referenced by the parent SEQUENCE.
    semi_major = ASN1TypeDefinition(
        name="SemiMajorAxisAccuracy",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..255)",
        constraint=semi_major_constraint,
        spec_section="7.184",
        description="The DE_SemiMajorAxisAccuracy data element is used to express the radius "
        "(length) of the semi-major axis of an ellipsoid representing the accuracy which can be "
        "expected from a GNSS system in 5 cm steps, typically at a one sigma level of confidence.",
    )
    semi_minor = ASN1TypeDefinition(
        name="SemiMinorAxisAccuracy",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..255)",
        constraint=semi_minor_constraint,
        spec_section="7.186",
        description="The DE_SemiMinorAxisAccuracy data element is used to express the radius of "
        "the semi-minor axis of an ellipsoid representing the accuracy which can be expected from "
        "a GNSS system in 5 cm steps, typically at a one sigma level of confidence.",
    )
    orientation = ASN1TypeDefinition(
        name="SemiMajorAxisOrientation",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..65535)",
        constraint=orientation_constraint,
        spec_section="7.185",
        description="The DE_SemiMajorAxisOrientation data element is used to orientate the angle "
        "of the semi-major axis of an ellipsoid representing the accuracy which can be expected "
        "from a GNSS system with respect to the coordinate system.",
    )

    # Step 3: Create the parent SEQUENCE that groups these fields together.
    # Note: is_extensible=False means no "..." marker, so no extension preamble bit.
    positional_accuracy = ASN1TypeDefinition(
        name="PositionalAccuracy",
        type_class=ASN1TypeClass.SEQUENCE,
        raw_definition="SEQUENCE { ... }",
        constraint=SequenceType(
            fields=(
                SequenceField(
                    name="semiMajor",
                    type_name="SemiMajorAxisAccuracy",
                    type=semi_major_constraint,
                    is_optional=False,
                    section_comment="NMEA-183 values expressed in strict ASN form",
                    inline_comment="",
                ),
                SequenceField(
                    name="semiMinor",
                    type_name="SemiMinorAxisAccuracy",
                    type=semi_minor_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
                SequenceField(
                    name="orientation",
                    type_name="SemiMajorAxisOrientation",
                    type=orientation_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
            ),
            is_extensible=False,
        ),
        spec_section="6.97",
        description="The DF_PositionalAccuracy data frame consists of various parameters of "
        "quality used to model the accuracy of the positional determination with respect to "
        "each given axis.",
    )

    # Step 4: Build the type registry (name -> definition lookup table).
    # Code generators use this to resolve type references.
    registry = {
        "SemiMajorAxisAccuracy": semi_major,
        "SemiMinorAxisAccuracy": semi_minor,
        "SemiMajorAxisOrientation": orientation,
        "PositionalAccuracy": positional_accuracy,
    }

    return J2735Specification(
        version="202409",
        messages=(),
        data_frames=(),
        data_elements=(),
        type_registry=registry,
    )


def make_optional_mock_spec() -> J2735Specification:
    """Create a mock spec with a SEQUENCE containing an OPTIONAL field.

    This models the real J2735 IntersectionReferenceID type:

        IntersectionReferenceID ::= SEQUENCE {
            region  RoadRegulatorID OPTIONAL,  -- 16 bits (0..65535)
            id      IntersectionID             -- 16 bits (0..65535)
        }

    Wire format (17 or 33 bits, depending on presence of 'region'):
        When region is ABSENT (17 bits):
            +---+------------------+
            | 0 |        id        |
            | 1b|      16 bits     |
            +---+------------------+

        When region is PRESENT (33 bits):
            +---+------------------+------------------+
            | 1 |      region      |        id        |
            | 1b|      16 bits     |      16 bits     |
            +---+------------------+------------------+

    The leading bit is the "optional presence bitmap" - one bit per OPTIONAL
    field indicating whether that field is present (1) or absent (0).

    Returns:
        A J2735Specification containing IntersectionReferenceID and its child types.
    """
    # Step 1: Define constraints for each primitive INTEGER field.
    # These determine the bit-width: 0..65535 needs 16 bits.
    region_constraint = IntegerConstraint(min_value=0, max_value=65535)
    intersection_id_constraint = IntegerConstraint(min_value=0, max_value=65535)

    # Step 2: Create ASN1TypeDefinition objects for each child type.
    # These represent the Data Elements referenced by the parent SEQUENCE.
    road_regulator_id = ASN1TypeDefinition(
        name="RoadRegulatorID",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..65535)",
        constraint=region_constraint,
        spec_section="7.173",
        description="The RoadRegulatorID is a 16-bit globally unique identifier assigned to an "
        "entity responsible for assigning Intersection IDs in the region over which it has such "
        "authority. The value zero shall be used for testing and should only be used in the "
        "absence of a suitable assignment. A single entity which assigns intersection IDs may be "
        "assigned several RoadRegulatorIDs. These assignments are presumed to be permanent.",
    )
    intersection_id = ASN1TypeDefinition(
        name="IntersectionID",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..65535)",
        constraint=intersection_id_constraint,
        spec_section="7.64",
        description="The IntersectionID is used within a region to uniquely define an intersection "
        "within that country or region in a 16-bit field. Assignment rules are established by the "
        "regional authority associated with the RoadRegulatorID under which this IntersectionID is "
        "assigned. Within the region the policies used to ensure an assigned value's uniqueness "
        "before that value is reused (if ever) is the responsibility of that region. Any such "
        "reuse would be expected to occur over a long epoch (many years).",
    )

    # Step 3: Create the parent SEQUENCE that groups these fields together.
    # Note: is_extensible=False means no "..." marker, so no extension preamble bit.
    # Note: Key difference from make_nested_mock_spec: is_optional=True on 'region'.
    intersection_reference_id = ASN1TypeDefinition(
        name="IntersectionReferenceID",
        type_class=ASN1TypeClass.SEQUENCE,
        raw_definition="SEQUENCE { region RoadRegulatorID OPTIONAL, id IntersectionID }",
        constraint=SequenceType(
            fields=(
                SequenceField(
                    name="region",
                    type_name="RoadRegulatorID",
                    type=region_constraint,
                    is_optional=True,  # <-- This triggers presence bitmap generation
                    section_comment="",
                    inline_comment="A globally unique regional assignment value typical assigned "
                    "to a regional DOT authority the value zero shall be used for testing needs",
                ),
                SequenceField(
                    name="id",
                    type_name="IntersectionID",
                    type=intersection_id_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="A unique mapping to the intersection in question within the "
                    "above region of use",
                ),
            ),
            is_extensible=False,
        ),
        spec_section="6.44",
        description="The IntersectionReferenceID data frame conveys the combination of an optional "
        "RoadRegulatorID and of an IntersectionID that is unique within that region. When the "
        "RoadRegulatorID is present the IntersectionReferenceID is guaranteed to be globally "
        "unique.",
    )

    # Step 4: Build the type registry (name -> definition lookup table).
    # Code generators use this to resolve type references.
    registry = {
        "RoadRegulatorID": road_regulator_id,
        "IntersectionID": intersection_id,
        "IntersectionReferenceID": intersection_reference_id,
    }

    return J2735Specification(
        version="202409",
        messages=(),
        data_frames=(),
        data_elements=(),
        type_registry=registry,
    )


def make_extensible_mock_spec() -> J2735Specification:
    """Create a mock spec with an extensible SEQUENCE (has "..." marker).

    This models the real J2735 PathPrediction type:

        PathPrediction ::= SEQUENCE {
            radiusOfCurve  RadiusOfCurvature,  -- 16 bits (-32767..32767)
            confidence     Confidence,         -- 8 bits (0..200)
            ...                                -- Extension marker
        }

    Wire format (25 bits minimum):
        +---+------------------+------------+
        | 0 |  radiusOfCurve   | confidence |
        | 1b|      16 bits     |   8 bits   |
        +---+------------------+------------+
         ^
         |__ Extension bit: 0 = no extensions, 1 = extensions present

    The "..." in ASN.1 means "future versions may add fields here." In UPER:
        - A 1-bit extension flag is prepended to the entire SEQUENCE
        - If the flag is 0, only the root component fields are present
        - If the flag is 1, additional encoding follows (not supported yet)

    Returns:
        A J2735Specification containing PathPrediction and its child types.
    """
    # Step 1: Define constraints for each primitive INTEGER field.
    # RadiusOfCurvature is a signed 16-bit integer (needs offset encoding).
    # Confidence is an unsigned 8-bit integer.
    radius_constraint = IntegerConstraint(min_value=-32767, max_value=32767)
    confidence_constraint = IntegerConstraint(min_value=0, max_value=200)

    # Step 2: Create ASN1TypeDefinition objects for each child type.
    # These represent the Data Elements referenced by the parent SEQUENCE.
    radius_of_curvature = ASN1TypeDefinition(
        name="RadiusOfCurvature",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (-32767..32767)",
        constraint=radius_constraint,
        spec_section="7.160",
        description="The entry DE_RadiusOfCurvature is a data element representing an estimate of "
        "the current trajectory of the sender. The value is represented as a first order of "
        "curvature approximation, as a circle with a radius R and an origin located at (0,R), "
        "where the x-axis is bore sight from the transmitting vehicle's perspective and normal to "
        "the vehicle's vertical axis. The vehicle's (x,y,z) coordinate frame follows the SAE "
        "convention. Radius R will be positive for curvatures to the right when observed from the "
        "transmitting vehicle's perspective. Radii shall be capped at a maximum value supported by "
        "the path prediction radius data type. Overflow of this data type shall be interpreted by "
        'the receiving vehicle as "a straight path" prediction. The radius can be derived from a '
        "number of sources including, but not limited to, map databases, rate sensors, vision "
        "systems, and global positioning. The precise algorithm to be used is outside the scope of "
        "this document.",
    )
    confidence = ASN1TypeDefinition(
        name="Confidence",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..200)",
        constraint=confidence_constraint,
        spec_section="7.27",
        description="The entry DE_Confidence is a data element representing the general confidence "
        "of another associated value.",
    )

    # Step 3: Create the parent SEQUENCE that groups these fields together.
    # Note: is_extensible=True this causes a 1-bit extension flag to be added at the start.
    path_prediction = ASN1TypeDefinition(
        name="PathPrediction",
        type_class=ASN1TypeClass.SEQUENCE,
        raw_definition="SEQUENCE { radiusOfCurve RadiusOfCurvature, confidence Confidence, ... }",
        constraint=SequenceType(
            fields=(
                SequenceField(
                    name="radiusOfCurve",
                    type_name="RadiusOfCurvature",
                    type=radius_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="LSB units of 10cm straight path to use value of 32767",
                ),
                SequenceField(
                    name="confidence",
                    type_name="Confidence",
                    type=confidence_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="LSB units of 0.5 percent",
                ),
            ),
            is_extensible=True,  # <-- This triggers extension bit generation
        ),
        spec_section="6.93",
        description="The DF_PathPrediction data frame allows vehicles and other type of users to "
        "share their predicted path trajectory by estimating a future path of travel. This future "
        "trajectory estimation provides an indication of future positions of the transmitting "
        "vehicle and can significantly enhance in-lane and out-of-lane threat classification. "
        "Trajectories in the PathPrediction data element are represented by the RadiusOfCurvature "
        "element. The algorithmic approach and allowed error limits are defined in a relevant "
        "standard using the data frame. To help distinguish between steady state and non-steady "
        "state conditions, a confidence factor is included in the data element to provide an "
        "indication of signal accuracy due to rapid change in driver input. When driver input is "
        "in steady state (straight roadways or curves with a constant radius of curvature), a high "
        "confidence value is reported. During non-steady state conditions (curve transitions, lane "
        "changes, etc.), signal confidence is reduced.",
    )

    # Step 4: Build the type registry (name -> definition lookup table).
    # Code generators use this to resolve type references.
    registry = {
        "RadiusOfCurvature": radius_of_curvature,
        "Confidence": confidence,
        "PathPrediction": path_prediction,
    }

    return J2735Specification(
        version="202409",
        messages=(),
        data_frames=(),
        data_elements=(),
        type_registry=registry,
    )


# =============================================================================
# Synthetic Type Builders
# =============================================================================
#
# Lightweight helpers for creating SequenceField and SequenceType objects
# in unit tests without importing the full mock spec builders.
#
# =============================================================================


def make_integer_field(
    name: str,
    type_name: str,
    min_value: int,
    max_value: int,
    *,
    is_optional: bool = False,
) -> SequenceField:
    """Create a SequenceField with an IntegerConstraint.

    Args:
        name: Field name (e.g., "msgCnt").
        type_name: ASN.1 type name (e.g., "MsgCount").
        min_value: Integer constraint minimum.
        max_value: Integer constraint maximum.
        is_optional: Whether the field is OPTIONAL.

    Returns:
        A SequenceField with an IntegerConstraint.

    Examples:
        >>> field = make_integer_field("msgCnt", "MsgCount", 0, 127)
        >>> field.name
        'msgCnt'
        >>> field.type_name
        'MsgCount'
        >>> field.type.uper_bit_width
        7
        >>> field.is_optional
        False
        >>> opt = make_integer_field("region", "RegionID", 0, 65535, is_optional=True)
        >>> opt.is_optional
        True
        >>> opt.type.uper_bit_width
        16
    """
    return SequenceField(
        name=name,
        type_name=type_name,
        type=IntegerConstraint(min_value=min_value, max_value=max_value),
        is_optional=is_optional,
        section_comment="",
        inline_comment="",
    )


def make_bitstring_field(
    name: str,
    type_name: str,
    root_size: int,
    *,
    is_optional: bool = False,
) -> SequenceField:
    """Create a SequenceField with a BitStringConstraint.

    Args:
        name: Field name.
        type_name: ASN.1 type name.
        root_size: BIT STRING size.
        is_optional: Whether the field is OPTIONAL.

    Returns:
        A SequenceField with a BitStringConstraint.

    Examples:
        >>> field = make_bitstring_field("lights", "ExteriorLights", 9)
        >>> field.name
        'lights'
        >>> field.type.uper_bit_width
        9
        >>> len(field.type.named_bits)
        9
        >>> field.is_optional
        False
        >>> opt = make_bitstring_field("flags", "Flags", 4, is_optional=True)
        >>> opt.is_optional
        True
    """
    named_bits = {f"bit{i}": i for i in range(root_size)}
    return SequenceField(
        name=name,
        type_name=type_name,
        type=BitStringConstraint(
            root_size=root_size,
            is_extensible=False,
            extension_size=None,
            named_bits=named_bits,
        ),
        is_optional=is_optional,
        section_comment="",
        inline_comment="",
    )


def make_variable_width_field(
    name: str,
    type_name: str,
    *,
    is_optional: bool = False,
) -> SequenceField:
    """Create a SequenceField with a SequenceOfType (variable-width).

    Args:
        name: Field name.
        type_name: ASN.1 type name.
        is_optional: Whether the field is OPTIONAL.

    Returns:
        A SequenceField with ``uper_bit_width is None``.

    Examples:
        >>> field = make_variable_width_field("items", "NodeList")
        >>> field.name
        'items'
        >>> field.type.uper_bit_width is None
        True
        >>> field.is_optional
        False
        >>> opt = make_variable_width_field("x", "T", is_optional=True)
        >>> opt.is_optional
        True
    """
    return SequenceField(
        name=name,
        type_name=type_name,
        type=SequenceOfType(
            element_type=IntegerConstraint(
                min_value=0,
                max_value=255,
            ),
            min_size=1,
            max_size=10,
        ),
        is_optional=is_optional,
        section_comment="",
        inline_comment="",
    )


def make_sequence(
    fields: tuple[SequenceField, ...],
    *,
    is_extensible: bool = False,
) -> SequenceType:
    """Create a SequenceType.

    Args:
        fields: Ordered tuple of SequenceField objects.
        is_extensible: Whether the SEQUENCE has "...".

    Returns:
        A SequenceType.

    Examples:
        >>> seq = make_sequence(fields=(
        ...     make_integer_field("a", "TypeA", 0, 255),
        ...     make_integer_field("b", "TypeB", 0, 15),
        ... ))
        >>> len(seq.fields)
        2
        >>> seq.is_extensible
        False
        >>> seq.uper_bit_width
        12
        >>> ext = make_sequence(
        ...     fields=(make_integer_field("x", "TypeX", 0, 127),),
        ...     is_extensible=True,
        ... )
        >>> ext.is_extensible
        True
    """
    return SequenceType(fields=fields, is_extensible=is_extensible)


# =============================================================================
# Shared typedef Validators
# =============================================================================


def _validate_bitstring_type(typedef: ASN1TypeDefinition | None) -> None:
    """Validate that type_name exists and is a BIT STRING in the spec.

    Args:
        typedef: The ASN.1 type definition to validate.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.
    """
    if typedef is None:
        raise ValueError("Type not found in specification")
    if typedef.type_class != ASN1TypeClass.BIT_STRING:
        raise ValueError(f"Type '{typedef.name}' is not a BIT STRING")
    if not isinstance(typedef.constraint, BitStringConstraint):
        raise ValueError(f"Type '{typedef.name}' has unexpected constraint type")


def generate_bitstring_code(template_path: str, spec: J2735Specification, type_name: str) -> str:
    """Generate BIT STRING C code for a given template.

    Args:
        template_path: Path to the Jinja2 template for BIT STRING code generation.
        spec: The parsed J2735 specification.
        type_name: Name of the BIT STRING type (e.g., "VehicleEventFlags").

    Returns:
        C code from the rendered template.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.
        jinja2.TemplateNotFound: If template_path does not exist.
    """
    typedef = spec.lookup_type(type_name)
    _validate_bitstring_type(typedef)
    env = create_jinja_env()
    return get_template(env, template_path).render(typedef=typedef)


def get_sequence_typedef(type_name: str, spec: J2735Specification) -> ASN1TypeDefinition:
    """Lookup and validate a SEQUENCE type from the specification.

    Args:
        type_name: Name of the type to look up.
        spec: The parsed J2735 specification.

    Returns:
        The type definition (guaranteed to have SequenceType constraint).

    Raises:
        ValueError: If type not found, not a SEQUENCE, or missing constraint.
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if typedef.type_class != ASN1TypeClass.SEQUENCE:
        raise ValueError(f"Type '{type_name}' is not a SEQUENCE")
    if not isinstance(typedef.constraint, SequenceType):
        raise ValueError(f"Type '{type_name}' has no SequenceType constraint")
    return typedef


# =============================================================================
# Shared Test Base Classes
# =============================================================================


class SpecLoadingTestBase(TestCase):
    """Base class that loads the J2735 specification once per test class.

    Use this for any test that needs access to the parsed J2735 spec.
    Subclasses can access the spec via `self.spec`.
    """

    spec: J2735Specification

    @classmethod
    def setUpClass(cls) -> None:
        """Load spec once for all tests."""
        cls.spec = parse_spec_file(SPEC_FILE_PATH)
